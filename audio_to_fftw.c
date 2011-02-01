#include "fdmf_sonic_reducer.h"
/*
This routine takes raw audio data and puts it into
the data structure that fftw operates on.  
*/

void audio_to_fftw(char *buf, fftw_complex *in) {
	int i;
	short *sp;
	sp = (short *)buf;
	for(i=0; i < CHUNKSAMPS; i++) {
		double left, right;
		left =  (double) *sp++; 
		right = (double) *sp++;
		in[i][0] = left + right; 
	}
}
