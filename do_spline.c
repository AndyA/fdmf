#include "sonic_reducer.h"

void do_spline(fftw_complex *spline_in, int c) {	
	FILE *spline;
	int i;
	spline = popen("spline -I a -n 255 -s", "w"); assert(spline != NULL);
	for(i = 0; i < c/2; i++) {
		double re, im, mag, freq;
		freq = (double)i / (double)c;
		re = spline_in[i][0];
		im = spline_in[i][1];
		mag = sqrt(re*re + im*im);
		/* this is a hack to get rid of crazy points at LF */
		if (i < 2) mag = 0;
		fprintf(spline, "%f %f\n", freq, mag);
	}
	fclose(spline);
}
