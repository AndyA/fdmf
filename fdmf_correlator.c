/* fdmf_correlator.c 
 *
 * Read a list of phashes and for each input hash display its closest
 * neighbours
 */

#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROG "fdmf_correlator"
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

static int verbose = 0;

static void
mention( const char *msg, ... ) {
  va_list ap;
  if ( verbose ) {
    va_start( ap, msg );
    vfprintf( stderr, msg, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
  }
}

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
count_hash_bits( const unsigned char *hash, size_t bytes ) {
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
    if ( hp > HASH_CHARS ) {
      die( "Hash too long" );
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
new_phash( const unsigned char *hash, struct phash *next ) {
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
read_file( FILE * fl, size_t * count ) {
  struct phash *data = NULL;
  unsigned char hash[HASH_BYTES];
  for ( ;; ) {
    if ( !read_hash( fl, hash ) )
      break;
    data = new_phash( hash, data );
    if ( count ) {
      ( *count )++;
    }
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

#ifdef DEBUG
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
#endif

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

#ifdef DEBUG
  sanity_check( c, *nused );
#endif
}

static void
compute_bitcount( unsigned char *bitcount ) {
  unsigned i;
  for ( i = 0; i < 65536; i++ ) {
    bitcount[i] = count_bits( i );
  }
}

static unsigned int
hash_distance( const struct phash *pi, const struct phash *pj,
               const unsigned char *bitcount ) {
  unsigned i, distance = 0;
  unsigned short *si = ( unsigned short * ) pi->bits;
  unsigned short *sj = ( unsigned short * ) pj->bits;
  for ( i = 0; i < HASH_BYTES / 2; i++ ) {
    distance += bitcount[si[i] ^ sj[i]];
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

static unsigned long
calc_work( const struct phash *data ) {
  unsigned long total = 0, pass = 0;
  const struct phash *pi;

  /* The number we're after is count * (count - 1) / 2 - but since we
   * don't have the count we might as well walk the list. This comment
   * exists purely to demonstrate that I know how to calculate the
   * number of iterations properly :)
   */

  for ( pi = data; pi; pi = pi->next ) {
    total += pass++;
  }
  return total;
}

static void
progress( unsigned long done, unsigned long total, size_t used,
          size_t size, unsigned int *lastpc, size_t * lastused ) {
  unsigned int pc = 400 * done / total;
  static char *spinner = "-\\|/";
  if ( pc != *lastpc || used / 100 != *lastused / 100
       || ( used == size && *lastused != size ) ) {
    fprintf( stderr, "\r[%3u%%] %c correlations: %10lu / %10lu", pc / 4,
             spinner[pc % 4], ( unsigned long ) used,
             ( unsigned long ) size );
    fflush( stderr );
    *lastpc = pc;
    *lastused = used;
  }
}

static struct correlation *
correlate( const struct phash *data, size_t nent, size_t * nused ) {
  struct correlation *c = new_correlation( nent );
  const struct phash *pi, *pj;
  unsigned distance;
  unsigned char bitcount[65536];
  unsigned long total = calc_work( data );
  unsigned long done = 0;
  unsigned int lastpc = -1;
  size_t lastused = 0;

  *nused = 0;

  compute_bitcount( bitcount );

  /* O(N^2) :) */
  for ( pi = data; pi; pi = pi->next ) {
    if ( verbose ) {
      /* TODO is this called often enough? */
      progress( done, total, *nused, nent, &lastpc, &lastused );
    }
    for ( pj = pi->next; pj; pj = pj->next ) {
      done++;
      if ( *nused == nent
           && best_distance( pi, pj ) >= c[*nused - 1].distance ) {
        continue;
      }
      distance = hash_distance( pi, pj, bitcount );
      if ( *nused < nent || distance < c[*nused - 1].distance ) {
        insert_correlation( c, nent, nused, pi, pj, distance );
      }
    }
  }
  if ( verbose ) {
    progress( done++, total, *nused, nent, &lastpc, &lastused );
    fprintf( stderr, "\n" );
  }
  return c;
}

static void
usage( void ) {
  fprintf( stderr, "Usage: " PROG " [options] < dump\n\n"
           "Options:\n"
           "  -K, --keep    <N> Number of matches to keep (default 1000)\n"
           "  -v, --verbose     Verbose output\n"
           "  -h, --help        See this text\n" );
  exit( 1 );
}

int
main( int argc, char *argv[] ) {
  struct phash *data;
  struct correlation *c;
  size_t nent = 1000, nused, count = 0;
  int ch;

  static struct option opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"keep", required_argument, NULL, 'K'},
    {NULL, 0, NULL, 0}
  };

  while ( ch = getopt_long( argc, argv, "hvK:", opts, NULL ), ch != -1 ) {
    switch ( ch ) {
    case 'v':
      verbose++;
      break;
    case 'K':
      {
        char *ep;
        nent = strtod( optarg, &ep );
        if ( *ep ) {
          die( "Bad number" );
        }
      }
      break;
    case 'h':
    default:
      usage(  );
    }
  }

  argc -= optind;
  argv += optind;

  if ( argc > 1 ) {
    usage(  );
    return 0;                   /* not reached, silence warning */
  }
  else if ( argc > 0 ) {
    FILE *fl = fopen( argv[0], "r" );
    if ( !fl ) {
      die( "Can't read %s", argv[0] );
    }
    mention( "Reading %s", argv[0] );
    data = read_file( fl, &count );
    fclose( fl );
  }
  else {
    data = read_file( stdin, &count );
  }

  mention( "Looking for %lu correlations in %lu files",
           ( unsigned long ) nent, ( unsigned long ) count );

#ifdef DEBUG
  dump_phash( data );
#endif

  c = correlate( data, nent, &nused );
  show_correlation( c, nused );
  free_phash( data );

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
