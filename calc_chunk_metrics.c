#include "fdmf_sonic_reducer.h"
/*
This routine reads STDIN until it gets to the end.  The stream is broken
into chunks and each chunk is fourier transformed.  Some metrics are 
calculated on each chunk and the time series of these metrics is stored
in ebuf[], rbuf[], and tbuf[].
*/

int
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
