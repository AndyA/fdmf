#include "fdmf_vector_pairs.h"
/* This code is a mess.  It would be nice to rewrite this.  */

int
process_args( int argc, char **argv, int *threshold, char *out_delim ) {
  int i, nargs;
  nargs = argc - 1;             /* because otherwise I will get confused. */
  switch ( nargs ) {
  case 0:
    /* use default thresholds and delimit output with spaces */
    *out_delim = ' ';
    break;
  case 1:
    if ( 0 == strncmp( argv[1], "-0", 2 ) ) {
      *out_delim = '\0';
    }
    else {                      /* get thresholds from command line args */
      for ( i = 0; i < NUM_TESTS; i++ ) {
        threshold[i] = atoi( argv[i + 1] );
      }
      *out_delim = ' ';
    }
    break;
  case NUM_TESTS:              /* default thresholds, space delimiter */
    for ( i = 0; i < NUM_TESTS; i++ ) {
      threshold[i] = atoi( argv[i + 1] );
    }
    *out_delim = ' ';

    break;
  case NUM_TESTS + 1:
    if ( 0 == strncmp( argv[1], "-0", 2 ) ) {
      *out_delim = '\0';
      for ( i = 0; i < NUM_TESTS; i++ ) {
        threshold[i] = atoi( argv[i + 2] );
      }
    }
    else {
      print_usage( argv );
      exit( 1 );
    }
    break;
  default:
    print_usage( argv );
    exit( 1 );
  }
  return ( 0 );
}
