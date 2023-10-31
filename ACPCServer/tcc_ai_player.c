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



    // Print the match state as viewed by player 0----------------------
    char matchStateStr[MAX_LINE_LEN];
    int matchStateStrLen = printMatchState(game, &state, MAX_LINE_LEN, matchStateStr);
    if (matchStateStrLen < 0) {
        printf("Error printing match state\n");
    } else {
        printf("Match state as viewed by player 0: %s\n", matchStateStr);
    }



    // Call Python script with matchStateStr as input---------------------
    char command[MAX_LINE_LEN];
    snprintf(command, MAX_LINE_LEN, "python3 matchstate_to_action.py | awk '{$1=$1;print}'\"%s\"", matchStateStr);
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
      fprintf(stderr, "ERROR: could not execute Python script\n");
      exit(EXIT_FAILURE);
    }

    // Read output from Python script
    char output[MAX_LINE_LEN] = {0};
    if (fgets(output, MAX_LINE_LEN, pipe) == NULL) {
      fprintf(stderr, "ERROR: could not read output from Python script\n");
      exit(EXIT_FAILURE);
    }

    // Close pipe and print output
    pclose(pipe);
    printf("Python script output: %s\n", output);

    //define the .type and .size of the action here
    if (output[0] == 'r') { 
      action.type = a_raise;
      // Parse the number following the "r"
      if (sscanf(output, "r%d", &action.size) != 1) {
          fprintf(stderr, "ERROR: could not parse raise amount from output\n");
          exit(EXIT_FAILURE);
      }
    } else if (output[0] == 'c') { 
      action.type = a_call;
      action.size = 0; // should be zero here
    } else if (output[0] == 'f') { 
      action.type = a_fold;
      action.size = 0; // should be zero here
    } else {
      fprintf(stderr, "ERROR: invalid output from Python script\n");
      exit(EXIT_FAILURE);
    }

    /* do the action! ---------------------------------------------------- */
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
  return EXIT_SUCCESS;
}
