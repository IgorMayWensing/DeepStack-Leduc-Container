/*
Copyright (C) 2011 by the Computer Poker Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <getopt.h>
#include "game.h"
#include "rng.h"
#include "net.h"
#include <json-c/json.h>

char* infostate_translator(const Game *game, const State *state) {
    // Accessing members of the 'State' structure within 'MatchState'
    printf("\nNumber of players in the game: %d\n", game->numPlayers);
    printf("Number of rounds in the game: %d\n\n", game->numRounds);
    
    printf("Current round: %d\n", state->round);
    printf("Hand ID: %u\n", state->handId);
    printf("maxSpent: %u\n", state->maxSpent);
    printf("minNoLimitRaiseTo: %u\n", state->minNoLimitRaiseTo);

    char* mycard = "";
    if (rankOfCard(state->holeCards[0][0]) == 10) {
      mycard = "Q";
    } else if (rankOfCard(state->holeCards[0][0]) == 11) {
      mycard = "K";
    } else if (rankOfCard(state->holeCards[0][0]) == 12) {
      mycard = "A";
    } else {
      printf("Card doesnt exist\n");
    }

    char* boardcard = "";
    if (rankOfCard(state->boardCards[0]) == 10) {
      boardcard = "Q";
    } else if (rankOfCard(state->boardCards[0]) == 11) {
      boardcard = "K";
    } else if (rankOfCard(state->boardCards[0]) == 12) {
      boardcard = "A";
    } else {
      printf("No card in board yet\n");
    }

    //do this somehow
    // char* actionhistory = "";
    // for () {
    //   state->action[ state->round ][ i ];
    // }    


    // Initial allocation for actionhistory. We assume 1000 bytes for the sake of this example.
    // In a real-world situation, consider a more robust string handling mechanism.
    char* actionhistory = malloc(1000);
    if (!actionhistory) {
        // Handle memory allocation error, e.g., exit or return
        exit(1);
    }
    strcpy(actionhistory, "");  // Initialize the string

    

    for (int i = 0; i < state->numActions[state->round]; i++) {
      Action currentAction = state->action[state->round][i];
      printf("Action type: %d\n", currentAction.type);
      // Convert currentAction to a string representation and append it to actionhistory.
      // Here, for simplicity, I'll assume Action has a member "type" that's an integer.
      char actionStr[50];  // Temporary string for action representation
      sprintf(actionStr, "%d", currentAction.type);  // Convert action type to string
      if (currentAction.size > 0) {
        printf("Action size: %d\n", currentAction.size);
        char sizeStr[50];
        sprintf(sizeStr, "/%d,", currentAction.size);
        strcat(actionStr, sizeStr);
      } else {
        strcat(actionStr, ",");
      }
      strcat(actionhistory, actionStr);  // Append to actionhistory
    }

    // Optionally, remove trailing comma if desired
    int len = strlen(actionhistory);
    if (len > 0 && actionhistory[len - 1] == ',') {
        actionhistory[len - 1] = '\0';
    }

    // Allocate memory for the infoset string. The size is the sum of the lengths of the other strings + 1 for the null terminator.
    char* infoset = malloc(strlen(mycard) + strlen(boardcard) + strlen(actionhistory) + 1);

    // Check if the memory was successfully allocated
    if (infoset != NULL) {
        strcpy(infoset, mycard); // Copy mycard into infoset
        strcat(infoset, boardcard); // Concatenate boardcard onto the end of infoset
        strcat(infoset, actionhistory); // Concatenate actionhistory onto the end of infoset
    }

    printf("\nmycard: %s\n", mycard);
    printf("boardcard: %s\n", boardcard);
    printf("actionhistory: %s\n", actionhistory);
    printf("InfoSet: %s\n", infoset);

    // Print the state as viewed by player 0
    char stateStr[MAX_LINE_LEN];
    int stateStrLen = printState(game, state, MAX_LINE_LEN, stateStr);
    if (stateStrLen < 0) {
        printf("Error printing state\n");
    } else {
        printf("State as viewed by player 0: %s\n", stateStr);
    }

    // Print the match state as viewed by player 0
    MatchState matchState;
    memset(&matchState, 0, sizeof(matchState));
    memcpy(&matchState.state, state, sizeof(State));
    matchState.viewingPlayer = 0;
    char matchStateStr[MAX_LINE_LEN];
    int matchStateStrLen = printMatchState(game, &matchState, MAX_LINE_LEN, matchStateStr);
    if (matchStateStrLen < 0) {
        printf("Error printing match state\n");
    } else {
        printf("Match state as viewed by player 0: %s\n", matchStateStr);
    }

    free(actionhistory);
    return infoset; // Now infoset should be "AKpp"
}

json_object* loadJSON(const char* filename) {
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    size_t readBytes = fread(string, 1, fsize, f);
    if(readBytes != fsize) {
        printf("ERROR: could not read json\n");
    }

    fclose(f);
    string[fsize] = 0;

    json_object *jobj = json_tokener_parse(string);
    free(string);
    return jobj;
}

// i have no ideia if this is right
void getActionsAndProbs(json_object *strategy, const char *infostate, int **actions, double **probs, int *num_actions) {
    json_object *entry;
    if (json_object_object_get_ex(strategy, infostate, &entry)) {
        json_object *actions_obj = json_object_array_get_idx(entry, 0);
        json_object *probs_obj = json_object_array_get_idx(entry, 1);
        
        *num_actions = json_object_array_length(actions_obj);
        *actions = malloc(*num_actions * sizeof(int));
        *probs = malloc(*num_actions * sizeof(double));

        for (int i = 0; i < *num_actions; i++) {
            (*actions)[i] = json_object_get_int(json_object_array_get_idx(actions_obj, i));
            (*probs)[i] = json_object_get_double(json_object_array_get_idx(probs_obj, i));
        }
    } else {
        *num_actions = 0;
        *actions = NULL;
        *probs = NULL;
    }
    // Inside getActionsAndProbs function, after extracting actions and probabilities
    printf("Number of possible actions: %d\n", *num_actions);
    for (int i = 0; i < *num_actions; i++) {
        printf("Action %d: %d, Probability: %f\n", i, (*actions)[i], (*probs)[i]);
    }
}

int main( int argc, char **argv )
{
  int sock, len, r;
  //int32_t min, max;
  uint16_t port;
  Game *game;
  MatchState state;
  Action action;
  FILE *file, *toServer, *fromServer;
  struct timeval tv;
  rng_state_t rng;
  char line[ MAX_LINE_LEN ];

  /* we make some assumptions about the actions - check them here */
  assert( NUM_ACTION_TYPES == 3 );

  if( argc < 4 ) {

    fprintf( stderr, "usage: player game server port\n" );
    exit( EXIT_FAILURE );
  }

  json_object *strategy = loadJSON("../blueprints/hjO-mccfr-6cards-11maxbet-EPcfr0-mRW0_0-iter10000.json");
  if (strategy == NULL) {
    fprintf(stderr, "ERROR: could not load JSON file\n");
    exit(EXIT_FAILURE);
  }

  /* Initialize the player's random number state using time */
  gettimeofday( &tv, NULL );
  init_genrand( &rng, tv.tv_usec );

  /* get the game */
  file = fopen( argv[ 1 ], "r" );
  if( file == NULL ) {

    fprintf( stderr, "ERROR: could not open game %s\n", argv[ 1 ] );
    exit( EXIT_FAILURE );
  }
  game = readGame( file );
  if( game == NULL ) {

    fprintf( stderr, "ERROR: could not read game %s\n", argv[ 1 ] );
    exit( EXIT_FAILURE );
  }
  fclose( file );

  /* connect to the dealer */
  if( sscanf( argv[ 3 ], "%"SCNu16, &port ) < 1 ) {

    fprintf( stderr, "ERROR: invalid port %s\n", argv[ 3 ] );
    exit( EXIT_FAILURE );
  }
  sock = connectTo( argv[ 2 ], port );
  if( sock < 0 ) {

    exit( EXIT_FAILURE );
  }
  toServer = fdopen( sock, "w" );
  fromServer = fdopen( sock, "r" );
  if( toServer == NULL || fromServer == NULL ) {

    fprintf( stderr, "ERROR: could not get socket streams\n" );
    exit( EXIT_FAILURE );
  }

  /* send version string to dealer */
  if( fprintf( toServer, "VERSION:%"PRIu32".%"PRIu32".%"PRIu32"\n",
	       VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION ) != 14 ) {

    fprintf( stderr, "ERROR: could not get send version to server\n" );
    exit( EXIT_FAILURE );
  }
  fflush( toServer );

  /* play the game! */
  while( fgets( line, MAX_LINE_LEN, fromServer ) ) {

    /* ignore comments */
    if( line[ 0 ] == '#' || line[ 0 ] == ';' ) {
      continue;
    }

    len = readMatchState( line, game, &state );
    if( len < 0 ) {

      fprintf( stderr, "ERROR: could not read state %s", line );
      exit( EXIT_FAILURE );
    }

    if( stateFinished( &state.state ) ) {
      /* ignore the game over message */

      continue;
    }

    if( currentPlayer( game, &state.state ) != state.viewingPlayer ) {
      /* we're not acting */

      continue;
    }

    /* add a colon (guaranteed to fit because we read a new-line in fgets) */
    line[ len ] = ':';
    ++len;

    char *infoset = infostate_translator(game, &state.state);
    printf("Returned InfoSet: %s\n", infoset);

    int *possible_actions;
    double *action_probs;
    int num_possible_actions;

    //getActionsAndProbs(strategy, infostate, &possible_actions, &action_probs, &num_possible_actions);

    if (num_possible_actions > 0) {
        double p = genrand_real2(&rng);
        int chosen_action_index = 0;
        for (int i = 0; i < num_possible_actions; i++) {
            if (p <= action_probs[i]) {
                chosen_action_index = i;
                break;
            }
            p -= action_probs[i];
        }

        action.type = (enum ActionType)possible_actions[chosen_action_index];
        // If your action type requires a size (like a raise amount), you'll need to set action.size here

        free(possible_actions);
        free(action_probs);
    } else {
        // Handle the case where the infostate is not found in the strategy
        action.type = a_fold;  // Default to fold
        action.size = 0;
    }

    // Inside main function, after choosing an action
    printf("Chosen action type: %d\n", action.type);
    if (action.type == a_raise) {
        printf("Chosen action size: %d\n", action.size);
    }

    /* do the action! */
    assert( isValidAction( game, &state.state, 0, &action ) );
    r = printAction( game, &action, MAX_LINE_LEN - len - 2,
		     &line[ len ] );
    if( r < 0 ) {

      fprintf( stderr, "ERROR: line too long after printing action\n" );
      exit( EXIT_FAILURE );
    }
    len += r;
    line[ len ] = '\r';
    ++len;
    line[ len ] = '\n';
    ++len;

    if( fwrite( line, 1, len, toServer ) != len ) {

      fprintf( stderr, "ERROR: could not get send response to server\n" );
      exit( EXIT_FAILURE );
    }
    free(infoset); // Free the memory after using infoset
    fflush( toServer );
  }

  json_object_put(strategy);
  return EXIT_SUCCESS;
}
