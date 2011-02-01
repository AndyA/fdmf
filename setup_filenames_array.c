#include "fdmf_vector_pairs.h"

int
setup_filenames_array( char ***filenames_ptr, int num_vecs ) {
  *filenames_ptr = malloc( sizeof( char * ) * num_vecs );
  if ( *filenames_ptr == NULL ) {
    perror( "malloc for filenames[]:" );
    exit( 1 );
  }
  return ( 0 );
}
