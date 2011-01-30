/* correlator.c 
 *
 * Read a list of phashes and for each input hash display its closest
 * neighbours
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_LEN 768
#define HASH_BYTES (HASH_LEN / 8)
#define HASH_CHARS (HASH_LEN / 4)

struct phash {
  struct phash *next;
  unsigned char *bits;
  unsigned int bit_count;
};

static void
die( const char *msg, ... ) {
  va_list ap;
  va_start( ap, msg );
  fprintf( stderr, "Fatal: " );
  vfprintf( stderr, msg, ap );
  fprintf( stderr, "\n" );
  va_end( ap );
  exit( 1 );
}

static unsigned int
count_bits( unsigned int v ) {
  unsigned int c;
  for ( c = 0; v; c++ ) {
    v &= v - 1;
  }
  return c;
}

static unsigned int
count_hash_bits( unsigned char *hash ) {
  unsigned int i, c = 0;
  for ( i = 0; i < HASH_BYTES; i++ ) {
    c += count_bits( hash[i] );
  }
  return c;
}

static void *
safe_malloc( size_t size ) {
  void *m = malloc( size );
  if ( NULL == m ) {
    die( "Out of memory for %lu bytes", ( unsigned long ) size );
  }
  return m;
}

static unsigned char *
read_hash( FILE * fl ) {
  unsigned char hash[HASH_BYTES];
  memset( hash, 0, HASH_BYTES );
  int hp = 0;
  for ( ;; ) {
    int v, c = fgetc( fl );
    if ( c == EOF ) {
      return NULL;
    }
    if ( c == '\n' ) {
      if ( hp != HASH_CHARS ) {
        die( "Hash too short" );
      }
      unsigned char *h = safe_malloc( HASH_BYTES );
      memcpy( h, hash, HASH_BYTES );
      return h;
    }
    if ( c < 0 || c > 255 || !isxdigit( c ) ) {
      die( "Illegal character '%c' in data", c );
    }
    v = isdigit( c ) ? c - '0' : tolower( c ) - 'a' + 10;
    hash[hp >> 1] |= ( v << ( ( ( hp & 1 ) ^ 1 ) << 2 ) );
    hp++;
  }
}

static struct phash *
new_phash( unsigned char *hash, struct phash *next ) {
  struct phash *ph = safe_malloc( sizeof( struct phash ) );
  ph->next = next;
  ph->bits = hash;
  ph->bit_count = count_hash_bits( hash );
  return ph;
}

static struct phash *
read_file( FILE * fl ) {
  struct phash *data = NULL;
  for ( ;; ) {
    unsigned char *h = read_hash( fl );
    if ( !h )
      break;
    data = new_phash( h, data );
  }
  return data;
}

static void
free_phash( struct phash *ph ) {
  struct phash *next;
  while ( ph ) {
    next = ph->next;
    free( ph->bits );
    free( ph );
    ph = next;
  }
}

static void
hexdump( unsigned char *data, size_t len ) {
  unsigned int i;
  for ( i = 0; i < len; i++ ) {
    printf( "%02x", data[i] );
  }
}

static void
dump_phash( struct phash *ph ) {
  while ( ph ) {
    hexdump( ph->bits, HASH_BYTES );
    printf( " %3d\n", ph->bit_count );
    ph = ph->next;
  }
}

int
main( void ) {
  struct phash *data = read_file( stdin );

  dump_phash( data );
  free_phash( data );

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
