
#include "fdmf_vector_pairs.h"

int
prescan_db( GDBM_FILE mdb ) {
  /* walk through the whole database */
  /* count keys and check for database sanity */
  int keycount = 0;
  datum mykey, myvalue;
  mykey = gdbm_firstkey( mdb );
  if ( mykey.dptr == NULL ) {
    fprintf( stderr, "empty database\n" );
    return ( -1 );
  }

  do {
    if ( mykey.dsize < MINKEYSIZE ) {
      fprintf( stderr, "empty key\n" );
      return ( -1 );
    }

    if ( mykey.dsize > MAXKEYSIZE ) {
      fprintf( stderr, "huge key\n" );
      return ( -1 );
    }

    myvalue = gdbm_fetch( mdb, mykey );

    if ( myvalue.dptr == NULL ) {
      fprintf( stderr, "key with no value\n" );
      return ( -1 );
    }
    if ( myvalue.dsize != MULTI_VEC_LEN + FILE_HASH_LEN + BASE_HASH_LEN ) {
      fprintf( stderr, "nonstandard value length: %d \n", myvalue.dsize );
      return ( -1 );
    }
    keycount++;
    mykey = gdbm_nextkey( mdb, mykey );
  } while ( mykey.dptr != NULL );

  return ( keycount );
}
