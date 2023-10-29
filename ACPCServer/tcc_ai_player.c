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

  // Function to convert card rank to its character representation
  char rankToChar(uint8_t rank) {
      switch (rank) {
          case 0: return 'Q';
          case 1: return 'K';
          case 2: return 'A';
          default: return '?'; // Error case, should not happen
      }
  }

char* infostate_translator(MatchState *state, Game *game) {
  static char infostate[100]; // Static to ensure memory remains allocated after function returns

  // Get and print my current card
  uint8_t my_card = state->cards[state->viewingPlayer][0];
  char my_card_char = rankToChar(rankOfCard(my_card));
  printf("My card: %c\n", my_card_char);

  // Get and print board cards, if they exist
  if (state->round != 0) {
    printf("Board cards: ");
    for (int i = 0; i < state->round; i++) {
      uint8_t board_card = state->cards[2][i];
      char board_card_char = rankToChar(rankOfCard(board_card));
      printf("%c ", board_card_char);
    }
    printf("\n");
  }

  // Get and print action history
  printf("Action history: ");
  for (int i = 0; i < state->numActions; i++) {
    Action action = state->action[i];
    if (action.type == a_raise) {
      printf("R%d ", action.size);
    } else if (action.type == a_call) {
      printf("C ");
    } else if (action.type == a_fold) {
      printf("F ");
    }
  }
  printf("\n");

  // Get and print pot size
  printf("Pot size: %d\n", state->pot);

  // Get and print my stack size
  printf("My stack size: %d\n", state->stack[state->viewingPlayer]);

  printf("\n");
  return infostate;
}

json_object* loadJSON(const char* filename) {
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);
    string[fsize] = 0;

    json_object *jobj = json_tokener_parse(string);
    free(string);
    return jobj;
}

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

    char infostate[100];

    strcpy(infostate, infostate_translator(&state, game));

    // After infostate is formed
    printf("Infostate: %s\n", infostate);

    int *possible_actions;
    double *action_probs;
    int num_possible_actions;

    getActionsAndProbs(strategy, infostate, &possible_actions, &action_probs, &num_possible_actions);

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
    fflush( toServer );
  }

  json_object_put(strategy);
  return EXIT_SUCCESS;
}
