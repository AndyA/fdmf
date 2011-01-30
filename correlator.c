/* correlator.c 
 *
 * Read a list of phashes and for each input hash display its closest
 * neighbours
 */

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_LEN 768
#define HASH_BYTES (HASH_LEN / 8)
#define HASH_CHARS (HASH_LEN / 4)
#define SUMMARY_LEN 8
#define SUMMARY_SPAN (HASH_BYTES/SUMMARY_LEN)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct phash {
  struct phash *next;
  unsigned char bits[HASH_BYTES];
  unsigned int summary[SUMMARY_LEN];
};

struct correlation {
  const struct phash *pair[2];
  unsigned int distance;
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
count_hash_bits( unsigned char *hash, size_t bytes ) {
  unsigned int i, c = 0;
  for ( i = 0; i < bytes; i++ ) {
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

static int
read_hash( FILE * fl, unsigned char hash[HASH_BYTES] ) {
  memset( hash, 0, HASH_BYTES );
  int hp = 0;
  for ( ;; ) {
    int v, c = fgetc( fl );
    if ( c == EOF ) {
      return 0;
    }
    if ( c == '\n' ) {
      if ( hp != HASH_CHARS ) {
        die( "Hash too short" );
      }
      return 1;
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
  int i = 0;
  struct phash *ph = safe_malloc( sizeof( struct phash ) );
  ph->next = next;
  memcpy( ph->bits, hash, HASH_BYTES );
  for ( i = 0; i < SUMMARY_LEN; i++ ) {
    ph->summary[i] =
        count_hash_bits( ph->bits + SUMMARY_SPAN * i, SUMMARY_SPAN );
  }
  return ph;
}

static struct phash *
read_file( FILE * fl ) {
  struct phash *data = NULL;
  unsigned char hash[HASH_BYTES];
  for ( ;; ) {
    if ( !read_hash( fl, hash ) )
      break;
    data = new_phash( hash, data );
  }
  return data;
}

static void
free_phash( struct phash *ph ) {
  struct phash *next;
  while ( ph ) {
    next = ph->next;
    free( ph );
    ph = next;
  }
}

static void
hexdump( const unsigned char *data, size_t len ) {
  unsigned int i;
  for ( i = 0; i < len; i++ ) {
    printf( "%02x", data[i] );
  }
}

static void
dump_phash( const struct phash *ph ) {
  int i;
  while ( ph ) {
    hexdump( ph->bits, HASH_BYTES );
    for ( i = 0; i < SUMMARY_LEN; i++ ) {
      printf( " %3d", ph->summary[i] );
    }
    printf( "\n" );
    ph = ph->next;
  }
}

static struct correlation *
new_correlation( size_t nent ) {
  struct correlation *c;
  unsigned i;

  c = safe_malloc( sizeof( struct correlation ) * nent );
  for ( i = 0; i < nent; i++ ) {
    c[i].pair[0] = c[i].pair[1] = NULL;
    c[i].distance = UINT_MAX;
  }
  return c;
}

static void
show_correlation( const struct correlation *c, size_t nused ) {
  unsigned i;
  for ( i = 0; i < nused; i++ ) {
    printf( "%5u ", c[i].distance );
    hexdump( c[i].pair[0]->bits, HASH_BYTES );
    printf( " " );
    hexdump( c[i].pair[1]->bits, HASH_BYTES );
    printf( "\n" );
  }
}

static void
sanity_check( const struct correlation *c, size_t nused ) {
  unsigned i;
  for ( i = 1; i < nused; i++ ) {
    if ( c[i - 1].distance > c[i].distance ) {
      show_correlation( c, nused );
      die( "distance out of order at %u\n", i );
    }
  }
}

static void
insert_correlation( struct correlation *c, size_t nent, size_t * nused,
                    const struct phash *this, const struct phash *that,
                    unsigned distance ) {
  unsigned lo, hi, mid = 0;
  size_t nnew;
  for ( lo = 0, hi = *nused; lo < hi; ) {
    mid = ( lo + hi ) / 2;
    if ( c[mid].distance < distance ) {
      lo = ++mid;
    }
    else if ( c[mid].distance >= distance ) {
      hi = mid;
    }
  }

  nnew = MIN( nent, *nused + 1 );
  memmove( &c[mid + 1], &c[mid], ( nnew - 1 - mid ) * sizeof( c[0] ) );
  c[mid].pair[0] = this;
  c[mid].pair[1] = that;
  c[mid].distance = distance;
  *nused = nnew;
  sanity_check( c, *nused );
}

static void
compute_bitcount( unsigned char *bitcount ) {
  unsigned i;
  for ( i = 0; i < 256; i++ ) {
    bitcount[i] = count_bits( i );
  }
}

static unsigned int
hash_distance( const struct phash *pi, const struct phash *pj,
               const unsigned char *bitcount ) {
  unsigned i, distance = 0;
  for ( i = 0; i < HASH_BYTES; i++ ) {
    distance += bitcount[pi->bits[i] ^ pj->bits[i]];
  }
  return distance;
}

static unsigned int
best_distance( const struct phash *pi, const struct phash *pj ) {
  unsigned i, distance = 0;
  for ( i = 0; i < SUMMARY_LEN; i++ ) {
    distance += abs( ( int ) pi->summary[i] - ( int ) pj->summary[i] );
  }
  return distance;
}

static struct correlation *
correlate( const struct phash *data, size_t nent, size_t * nused ) {
  struct correlation *c = new_correlation( nent );
  const struct phash *pi, *pj;
  unsigned distance;
  unsigned char bitcount[256];

  *nused = 0;

  compute_bitcount( bitcount );

  /* O(N^2) :) */
  for ( pi = data; pi; pi = pi->next ) {
    for ( pj = pi->next; pj; pj = pj->next ) {
      /* don't know if this summary stuff is worth the bother */
      fprintf( stderr, "." );
      if ( *nused == nent
           && best_distance( pi, pj ) >= c[*nused - 1].distance ) {
        printf( "*" );
        continue;
      }
      distance = hash_distance( pi, pj, bitcount );
      if ( *nused < nent || distance < c[*nused - 1].distance ) {
        insert_correlation( c, nent, nused, pi, pj, distance );
      }
    }
  }
  fprintf( stderr, "\n" );
  return c;
}

int
main( int argc, char *argv[] ) {
  struct phash *data;
  struct correlation *c;
  size_t nent = 1000, nused;

  if ( argc < 2 ) {
    data = read_file( stdin );
  }
  else {
    FILE *fl = fopen( argv[1], "r" );
    if ( !fl ) {
      die( "Can't read %s", argv[1] );
    }
    data = read_file( fl );
    fclose( fl );
  }

/*  dump_phash( data );*/
  c = correlate( data, nent, &nused );
  show_correlation( c, nused );
  free_phash( data );

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
