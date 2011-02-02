/* fdmf_sonic_reducer.c */

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <fftw3.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CHUNKSAMPS 11025
#define CHUNKBYTES 4 * CHUNKSAMPS
#define NUM_BANDS 4
#define MAXCHUNKS 65536         /* 4.5 hours */

typedef fftw_complex f_c;
typedef fftw_plan f_p;

/*
This program reads raw 16-bit stereo native endian audio data on
STDIN and writes to STDOUT the power spectra of the chunk metrics.
It is inteneded to be called by the fdmf script to generate the
summary value that is stored in the fdmf database/index/cache.
The output spectra come out concatenated withg no delimiter.
256 compenents/metric * 3 metrics = 768 values (ascii floats).
This program uses FFTW for calculating the FFT and
uses GNU Plotutils for spline fitting the spectra
to a standard set of frequency points.  
*/

static void
pad( int bufbytes, char *buf, int byte_count ) {
  int i;
  for ( i = byte_count; i < bufbytes; i++ ) {
    buf[i] = 0;
  }
}

/* This function returns the number if valid bytes in buf. */
/* If EOF is reached it will return a number less than size. */
/* But the buffer will not have junk in it.  It will either */
/* have data in it or it will be padded with zeros */

static int
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

static void
setup_bufs( fftw_complex ** ebuf, fftw_complex ** eout,
            fftw_complex ** rbuf, fftw_complex ** rout,
            fftw_complex ** tbuf, fftw_complex ** tout ) {
  *ebuf = fftw_malloc( sizeof( fftw_complex ) * MAXCHUNKS );
  assert( ebuf != NULL );
  *eout = fftw_malloc( sizeof( fftw_complex ) * MAXCHUNKS );
  assert( eout != NULL );
  *rbuf = fftw_malloc( sizeof( fftw_complex ) * MAXCHUNKS );
  assert( rbuf != NULL );
  *rout = fftw_malloc( sizeof( fftw_complex ) * MAXCHUNKS );
  assert( rout != NULL );
  *tbuf = fftw_malloc( sizeof( fftw_complex ) * MAXCHUNKS );
  assert( tbuf != NULL );
  *tout = fftw_malloc( sizeof( fftw_complex ) * MAXCHUNKS );
  assert( tout != NULL );
}

static double *
setup_window( int len ) {
  int i;
  double *win_tbl = ( double * ) malloc( len * sizeof( double ) );
  if ( win_tbl == NULL ) {
    perror( "malloc for win_tbl" );
    exit( 1 );
  }

  for ( i = 0; i < len; i++ ) {
    win_tbl[i] = 0.5 - 0.5 * cos( 6.283 * ( ( double ) i / len ) );
  }
  return ( win_tbl );
}

/*
This routine takes in a data structure that is the output 
of an fftw FFT operation.  The input to this routine
is in the frequency domain.  The power is integrated over
NUM_BANDS non-overrlapping frequency bands.  These band
energies are returned to the caller in the array be[].
*/

void
calc_band_energies( fftw_complex * out, double *be ) {
  int xovr[NUM_BANDS + 1] = { 3, 15, 90, 600, 5000 };
  int b;
  for ( b = 0; b < NUM_BANDS; b++ ) {
    int i;
    be[b] = 0;
    for ( i = xovr[b]; i < xovr[b + 1]; i++ ) {
      double re, im;
      re = out[i][0];
      im = out[i][1];
      be[b] += re * re + im * im;
    }
    be[b] /= CHUNKSAMPS;
    be[b] = sqrt( be[b] );
  }                             /* post: be[] is full of the band energies */
}

static void
audio_to_fftw( char *buf, fftw_complex * in ) {
  unsigned char *dp = ( unsigned char * ) buf;
  int i;
  for ( i = 0; i < CHUNKSAMPS; i++ ) {
    /* Cast to short to force sign extension. Is this portable? */
    double left = ( short ) ( dp[0] | ( dp[1] << 8 ) );
    double right = ( short ) ( dp[2] | ( dp[3] << 8 ) );
    dp += 4;
    in[i][0] = left + right;
  }
}

static void
window( f_c * buf, double *win_tbl, int len ) {
  int i;
  for ( i = 0; i < len; i++ ) {
    buf[i][0] *= win_tbl[i];
  }
}

static void
chunk_metrics( double *be, f_c * e, f_c * r, f_c * t, int chunk ) {
  double energy, ratio, twist, lows, highs, evens, odds;
  lows = be[0] + be[1];
  highs = be[2] + be[3];
  evens = be[0] + be[2];
  odds = be[1] + be[3];

  /* trap zeros */
  lows = fabs( lows ) < 0.001 ? 0.001 : lows;
  odds = fabs( odds ) < 0.001 ? 0.001 : odds;

  energy = lows + highs;
  ratio = highs / lows;
  twist = evens / odds;

  ratio = fabs( ratio ) > 20 ? 20 : ratio;
  twist = fabs( twist ) > 20 ? 20 : twist;

  e[chunk][0] = energy;
  r[chunk][0] = ratio;
  t[chunk][0] = twist;
}

static int
calc_chunk_metrics( f_c * ebuf, f_c * rbuf, f_c * tbuf ) {
  char buf[CHUNKBYTES];
  double *chunk_window;
  int chunkcount = 0;
  f_c *in, *out;
  f_p p;
  in = fftw_malloc( sizeof( f_c ) * CHUNKSAMPS );
  assert( in != NULL );
  out = fftw_malloc( sizeof( f_c ) * CHUNKSAMPS );
  assert( out != NULL );
  p = fftw_plan_dft_1d( CHUNKSAMPS, in, out, FFTW_FORWARD, FFTW_ESTIMATE );
  chunk_window = setup_window( CHUNKSAMPS );
  while ( CHUNKBYTES == read_from_fd( 0, buf, CHUNKBYTES ) ) {
    /* Forget about the last fraction of a second of audio data. */
    /* Process exactly one second of audio data per iteration. */
    double be[NUM_BANDS];       /* band energies */
    audio_to_fftw( buf, in );
    window( in, chunk_window, CHUNKSAMPS );
    fftw_execute( p );          /* post: in[] -> FFT -> out[] is done */
    calc_band_energies( out, be );      /* post: be[] is valid */
    chunk_metrics( be, ebuf, rbuf, tbuf, chunkcount );
    chunkcount++;
    /* post: ebuf[], rbuf[], and tbuf[] have chunkcount valid elements */
  }                             /* post: we got to the end of the input data */
  fftw_free( in );
  fftw_free( out );
  fftw_destroy_plan( p );
  free( chunk_window );
  return chunkcount;
}

static void
setup_plans( int c, f_p * ep, f_p * rp, f_p * tp,
             f_c * ebuf, f_c * rbuf, f_c * tbuf,
             f_c * eout, f_c * rout, f_c * tout ) {
  *ep = fftw_plan_dft_1d( c, ebuf, eout, FFTW_FORWARD, FFTW_ESTIMATE );
  *rp = fftw_plan_dft_1d( c, rbuf, rout, FFTW_FORWARD, FFTW_ESTIMATE );
  *tp = fftw_plan_dft_1d( c, tbuf, tout, FFTW_FORWARD, FFTW_ESTIMATE );
}

static void
free_bufs( f_c * ebuf, f_c * eout, f_c * rbuf,
           f_c * rout, f_c * tbuf, f_c * tout ) {
  fftw_free( ebuf );
  fftw_free( eout );
  fftw_free( rbuf );
  fftw_free( rout );
  fftw_free( tbuf );
  fftw_free( tout );
}

static void
destroy_plans( fftw_plan ep, fftw_plan rp, fftw_plan tp ) {
  fftw_destroy_plan( ep );
  fftw_destroy_plan( rp );
  fftw_destroy_plan( tp );
}

static void
do_spline( fftw_complex * spline_in, int c ) {
  FILE *spline;
  int i;
  spline = popen( "spline -I a -n 255 -s", "w" );
  assert( spline != NULL );
  for ( i = 0; i < c / 2; i++ ) {
    double re, im, mag, freq;
    freq = ( double ) i / ( double ) c;
    re = spline_in[i][0];
    im = spline_in[i][1];
    mag = sqrt( re * re + im * im );
    /* this is a hack to get rid of crazy points at LF */
    if ( i < 2 )
      mag = 0;
    fprintf( spline, "%f %f\n", freq, mag );
  }
  pclose( spline );
}

int
main( void ) {
  f_c *ebuf, *eout, *rbuf, *rout, *tbuf, *tout;
  f_p ep, rp, tp;
  int chunks;
  double *track_window;
  setup_bufs( &ebuf, &eout, &rbuf, &rout, &tbuf, &tout );
  chunks = calc_chunk_metrics( ebuf, rbuf, tbuf );
  /* ebuf[], rbuf[], and tbuf[] each have chunks valid elements */
  setup_plans( chunks, &ep, &rp, &tp, ebuf, rbuf, tbuf, eout, rout, tout );
  track_window = setup_window( chunks );
  window( ebuf, track_window, chunks );
  window( rbuf, track_window, chunks );
  window( tbuf, track_window, chunks );
  fftw_execute( ep );
  fftw_execute( rp );
  fftw_execute( tp );
  /* now eout[], rout[], and tout[] are valid */
  do_spline( eout, chunks );
  do_spline( rout, chunks );
  do_spline( tout, chunks );
  free_bufs( ebuf, eout, rbuf, rout, tbuf, tout );
  free( track_window );
  destroy_plans( ep, rp, tp );
  return ( 0 );
}
