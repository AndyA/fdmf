#include "common.h"

/* This function returns the number if valid bytes in buf. */
/* If EOF is reached it will return a number less than size. */
/* But the buffer will not have junk in it.  It will either */
/* have data in it or it will be padded with zeros */

int
read_from_fd( int fd, char *buf, int size ) {
  int i, b;
  for ( i = 0; i < size; i += b ) {

    b = read( fd, buf + i, size - i );

    if ( b == 0 ) {             /* EOF */
      pad( size, buf, i );
      return ( i );
    }

    if ( b < 0 ) {              /* error */
      perror( "error read_from_fd" );
      exit( 1 );
    }
  }
  return ( i );
}
