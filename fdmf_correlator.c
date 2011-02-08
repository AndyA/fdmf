/* fdmf_correlator.c 
 *
 * Read a list of phashes and for each input hash display its closest
 * neighbours
 */

#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anchor_compar.h"

#define PROG "fdmf_correlator"
#define HASH_LEN 768
#define HASH_BYTES (HASH_LEN / 8)
#define HASH_CHARS (HASH_LEN / 4)
#define SUMMARY_LEN 8
#define SUMMARY_SPAN (HASH_BYTES/SUMMARY_LEN)
#define HIST_SIZE 100
#define ANCHORS 8
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ANCHOR_MINDIST(pi, ph, anchor) \
  fabs( (pi)->anchor_dist[anchor] - (pj)->anchor_dist[anchor] )

#define GET_DIM(ph, bit) \
  (( (ph)->bits[(bit) >> 3] & ( 1 << ( (bit) & 7 ) ) ) ? 1 : 0)

struct phash {
  struct phash *next;
  unsigned char bits[HASH_BYTES];
  unsigned int summary[SUMMARY_LEN];
  double anchor_dist[ANCHORS];
};

struct correlation {
  const struct phash *pair[2];
  unsigned int distance;
};

struct index {
  size_t size;
  struct phash **data;
  int ( *compar ) ( const void *, const void * );
};

struct prox_iter {
  const struct index *idx;
  double dist;
  unsigned anchor;
  long pos_lo, pos_hi;
};

static int verbose = 0;

#define BY_ANCHOR_DISTANCE(d)                                        \
  static int                                                         \
  by_anchor_distance_ ## d ( const void *a, const void *b ) {        \
    const struct phash *pa = *( ( const struct phash ** ) a );       \
    const struct phash *pb = *( ( const struct phash ** ) b );       \
    return pa->anchor_dist[d] < pb->anchor_dist[d] ? -1              \
        : pa->anchor_dist[d] > pb->anchor_dist[d] ? 1 : 0;           \
  }

BY_ANCHOR_DISTANCE( 0 )
    BY_ANCHOR_DISTANCE( 1 )
    BY_ANCHOR_DISTANCE( 2 )
    BY_ANCHOR_DISTANCE( 3 )
    BY_ANCHOR_DISTANCE( 4 )
    BY_ANCHOR_DISTANCE( 5 )
    BY_ANCHOR_DISTANCE( 6 )
    BY_ANCHOR_DISTANCE( 7 )

typedef int ( *compar_func ) ( const void *a, const void *b );
static compar_func anchor_cf[ANCHORS] = {
  by_anchor_distance_0,
  by_anchor_distance_1,
  by_anchor_distance_2,
  by_anchor_distance_3,
  by_anchor_distance_4,
  by_anchor_distance_5,
  by_anchor_distance_6,
  by_anchor_distance_7
};

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
free_correlation( struct correlation *c ) {
  free( c );
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
    for ( i = 0; i < ANCHORS; i++ ) {
      printf( " %5.2f", ph->anchor_dist[i] );
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

  if ( mid + 1 >= nent )
    return;

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

static double
anchor_distance( const struct phash *ph, const double c[HASH_LEN] ) {
  double dist = 0;
  unsigned i;
  for ( i = 0; i < HASH_LEN; i++ ) {
    double dd = GET_DIM( ph, i ) - c[i];
    dist += dd * dd;
  }
  return dist;
}

static void
compute_distance( struct phash *data, unsigned anchor,
                  const double c[HASH_LEN] ) {
  struct phash *pi;
  for ( pi = data; pi; pi = pi->next ) {
    pi->anchor_dist[anchor] = anchor_distance( pi, c );
  }
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

static void
centre( const struct phash *data, double *c ) {
  const struct phash *pi;
  unsigned i, count;
  for ( i = 0; i < HASH_LEN; i++ ) {
    c[i] = 0;
  }

  for ( count = 0, pi = data; pi; pi = pi->next ) {
    for ( i = 0; i < HASH_LEN; i++ ) {
      c[i] += GET_DIM( pi, i );
    }
    count++;
  }
  for ( i = 0; i < HASH_LEN; i++ ) {
    c[i] /= count;
  }
}

static struct index *
new_index( struct phash *data,
           int ( *compar ) ( const void *a, const void *b ) ) {
  struct phash *ph;
  struct index *idx = safe_malloc( sizeof( struct index ) );
  size_t size;

  for ( size = 0, ph = data; ph; ph = ph->next )
    size++;

  idx->size = size;
  idx->data = safe_malloc( sizeof( struct phash ) * size );
  idx->compar = compar;

  for ( size = 0, ph = data; ph; ph = ph->next ) {
    idx->data[size++] = ph;
  }

  return idx;
}

static void
free_index( struct index *idx ) {
  if ( idx ) {
    free( idx->data );
    free( idx );
  }
}

static void
sort_index( struct index *idx ) {
  qsort( idx->data, idx->size, sizeof( struct phash * ), idx->compar );
}

static void
print_index( const struct index *idx ) {
  unsigned long i;
  for ( i = 0; i < idx->size; i++ ) {
    printf( "%f\n", idx->data[i]->anchor_dist[0] );
  }
}

static void
histogram( const struct phash *data, unsigned anchor,
           unsigned *hist, size_t hsize, double *pmin, double *pmax,
           double *pstep ) {
  const struct phash *ph;
  double min, max, step;
  unsigned i;

  min = max = data->anchor_dist[anchor];

  for ( ph = data->next; ph; ph = ph->next ) {
    min = MIN( min, ph->anchor_dist[anchor] );
    max = MAX( max, ph->anchor_dist[anchor] );
  }

  step = ( max - min ) / hsize;

  for ( i = 0; i < hsize; i++ ) {
    hist[i] = 0;
  }

  for ( ph = data; ph; ph = ph->next ) {
    hist[( int ) ( ( ph->anchor_dist[anchor] - min ) / step )]++;
  }

  *pmin = min;
  *pmax = max;
  *pstep = step;
}

static unsigned
max_hist_slot( const unsigned *hist, size_t hsize ) {
  unsigned i, best = 0, best_slot = 0;

  for ( i = 0; i < hsize; i++ ) {
    if ( hist[i] > best ) {
      best = hist[i];
      best_slot = i;
    }
  }

  return best_slot;
}

static void
prox_init_pos( const struct index *idx, unsigned long pos, unsigned anchor,
               struct prox_iter *iter ) {
  iter->idx = idx;
  iter->dist = idx->data[pos]->anchor_dist[anchor];
  iter->anchor = anchor;
  iter->pos_lo = pos;
  iter->pos_hi = pos + 1;
}

static void
prox_init( const struct index *idx, double dist, unsigned anchor,
           struct prox_iter *iter ) {
  unsigned long lo, hi, mid = 0;

  for ( lo = 0, hi = idx->size; lo < hi; ) {
    mid = ( lo + hi ) / 2;
    double ad = idx->data[mid]->anchor_dist[anchor];
    if ( ad < dist ) {
      lo = ++mid;
    }
    else if ( ad >= dist ) {
      hi = mid;
    }
  }

  prox_init_pos( idx, mid, anchor, iter );
  iter->dist = dist;
}

#define SLOT_DIST(iter, pos) \
  fabs( iter->idx->data[pos]->anchor_dist[iter->anchor] - iter->dist )

static struct phash *
prox_next( struct prox_iter *iter ) {
  if ( iter->pos_lo >= 0 &&
       ( iter->pos_hi >= ( long ) iter->idx->size ||
         SLOT_DIST( iter, iter->pos_lo ) <= SLOT_DIST( iter, iter->pos_hi )
        ) ) {
    return iter->idx->data[iter->pos_lo--];
  }

  if ( iter->pos_hi < ( long ) iter->idx->size ) {
    return iter->idx->data[iter->pos_hi++];
  }

  return NULL;
}

static void
bits_to_anchor( const struct phash *ph, double *c ) {
  unsigned i;
  for ( i = 0; i < HASH_LEN; i++ ) {
    c[i] = GET_DIM( ph, i );
  }
}

static void
interp_anchor( double *this, const double *that, double ratio ) {
  unsigned i;
  for ( i = 0; i < HASH_LEN; i++ ) {
    this[i] = this[i] * ( 1 - ratio ) + that[i] * ratio;
  }
}

static struct index *
compute_anchors( struct phash *data ) {
  double anchor[ANCHORS][HASH_LEN];
  struct index *idx;
  unsigned hist[HIST_SIZE];
  double min, max, step;
  double best_dist, dist_step, next_dist, cur_dist;
  unsigned slot, i;
  struct prox_iter iter;
  struct phash *ph;

  centre( data, anchor[0] );
  compute_distance( data, 0, anchor[0] );

  idx = new_index( data, anchor_cf[0] );
  sort_index( idx );

  for ( i = 1; i < ANCHORS; i++ ) {
    unsigned long pos = i * idx->size / ANCHORS;
    bits_to_anchor( idx->data[pos], anchor[i] );
    compute_distance( data, i, anchor[i] );
  }

#if 0
  histogram( data, 0, hist, HIST_SIZE, &min, &max, &step );
  slot = max_hist_slot( hist, HIST_SIZE );
  best_dist = min + step * slot + step / 2;

  dist_step = ( max - min ) / ( ANCHORS * 5 );
  next_dist = 0;

  prox_init( idx, best_dist, 0, &iter );
  for ( i = 1; i < ANCHORS; i++ ) {
  next:
    if ( ph = prox_next( &iter ), !ph )
      break;
    cur_dist = fabs( best_dist - ph->anchor_dist[0] );
    if ( cur_dist < next_dist )
      goto next;
    next_dist = cur_dist + dist_step;
    bits_to_anchor( ph, anchor[i] );
    interp_anchor( anchor[i], anchor[0], 0.5 );
    compute_distance( data, i, anchor[i] );
  }
#endif

  return idx;
}

static struct correlation *
correlate2( struct phash *data, const struct index *idx, size_t nent,
            size_t * nused, unsigned maxdist ) {
  struct correlation *c = new_correlation( nent );
  struct index *idx2;
  const struct phash *pi, *pj;
  struct prox_iter iter;
  unsigned char bitcount[65536];
  unsigned long i;
  unsigned distance;
  unsigned j;
  double mindist;
  unsigned which = 1;

  /* stats */
  unsigned long n_done = 0;
  unsigned long n_cut = 0;
  unsigned long n_skip = 0;
  unsigned long n_dist = 0;
  unsigned long n_insert = 0;
  double tot_mindist = 0;
  double min_mindist = HASH_LEN;
  double max_mindist = 0;
  unsigned skipat[ANCHORS];
  for ( i = 0; i < ANCHORS; i++ )
    skipat[i] = 0;

  *nused = 0;

  compute_bitcount( bitcount );

  idx2 = new_index( data, anchor_cf[which] );

  for ( i = 0; i < idx->size; i++ ) {
    n_done++;
    pi = idx->data[i];
    prox_init( idx2, pi->anchor_dist[which], which, &iter );

    for ( ;; ) {
    next:
      if ( pj = prox_next( &iter ), !pj )
        break;

      if ( pj == pi )
        goto next;

      for ( j = 0; j < ANCHORS; j++ ) {
        if ( ANCHOR_MINDIST( pi, pj, j ) > maxdist ) {
          skipat[j]++;
        }
      }

      mindist = ANCHOR_MINDIST( pi, pj, which );

      if ( mindist > maxdist ) {
        n_cut++;
        break;
      }

      for ( j = 0; j < ANCHORS; j++ ) {
        if ( j != which ) {
          mindist = MAX( mindist, ANCHOR_MINDIST( pi, pj, j ) );
          if ( mindist > maxdist ) {
            n_skip++;
            goto next;
          }
        }
      }

      tot_mindist += mindist;
      min_mindist = MIN( min_mindist, mindist );
      max_mindist = MAX( max_mindist, mindist );

      distance = hash_distance( pi, pj, bitcount );
      n_dist++;
      if ( distance < mindist ) {
        mention( "Interesting! distance = %u, mindist = %f", distance,
                 mindist );
      }
      if ( distance <= maxdist ) {
        n_insert++;
        insert_correlation( c, nent, nused, pi, pj, distance );
      }
    }
  }

  mention( "n_done   = %10lu Total number of items processed\n"
           "n_cut    = %10lu Search short-circuited\n"
           "n_skip   = %10lu Match invalidated early\n"
           "n_dist   = %10lu Bitwise distance computed\n"
           "n_insert = %10lu Vector pair inserted\n"
           "mindist:\n"
           "min      = %f\n"
           "max      = %f\n"
           "average  = %f\n"
           "index:\n"
           "max      = %f",
           n_done, n_cut, n_skip, n_dist, n_insert,
           min_mindist, max_mindist, tot_mindist / n_dist,
           idx->data[idx->size - 1]->anchor_dist[0]
       );
  for ( i = 0; i < ANCHORS; i++ ) {
    mention( "skip at %2d : %6lu", i, skipat[i] );
  }

  return c;
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
           "  -M, --max     <N> Maximum distance to consider a match\n"
           "  -v, --verbose     Verbose output\n"
           "  -h, --help        See this text\n" );
  exit( 1 );
}

int
main( int argc, char *argv[] ) {
  struct phash *data;
  struct correlation *c;
  struct index *idx;
  size_t nent = 1000, nused, count = 0;
  unsigned maxdist = 100;
  int ch;

  static struct option opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"keep", required_argument, NULL, 'K'},
    {"max", required_argument, NULL, 'M'},
    {NULL, 0, NULL, 0}
  };

  while ( ch = getopt_long( argc, argv, "hvK:", opts, NULL ), ch != -1 ) {
    switch ( ch ) {
    case 'v':
      verbose++;
      break;
    case 'M':
      {
        char *ep;
        maxdist = strtod( optarg, &ep );
        if ( *ep ) {
          die( "Bad number" );
        }
      }
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

  if ( !data ) {
    die( "No data loaded" );
  }

  idx = compute_anchors( data );
/*  print_index( idx );*/

  mention( "Looking for %lu correlations in %lu files",
           ( unsigned long ) nent, ( unsigned long ) count );

#ifdef DEBUG
  dump_phash( data );
#else
  c = correlate2( data, idx, nent, &nused, maxdist );
  show_correlation( c, nused );
#endif

  free_correlation( c );
  free_index( idx );
  free_phash( data );

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
