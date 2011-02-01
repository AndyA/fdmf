# include "common.h"

void
write_to_fd( int fd, char *buf, int size ) {
  /* this is naive */
  if ( size != write( fd, buf, size ) ) {
    perror( "write failed" );
    exit( 1 );
  }
}
