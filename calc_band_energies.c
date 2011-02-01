#include "fdmf_sonic_reducer.h"

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
