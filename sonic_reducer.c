#include "sonic_reducer.h"
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

int main(void) {
	f_c *ebuf, *eout, *rbuf, *rout, *tbuf, *tout;
	f_p ep, rp, tp;
	int chunks;
	double *track_window;
	setup_bufs(&ebuf, &eout, &rbuf, &rout, &tbuf, &tout);
 	chunks = calc_chunk_metrics(ebuf, rbuf, tbuf);
	/* ebuf[], rbuf[], and tbuf[] each have chunks valid elements */
	setup_plans(chunks, &ep, &rp, &tp, ebuf, rbuf, tbuf, eout, rout, tout);
	track_window = setup_window(chunks);
	window(ebuf, track_window, chunks); 
	window(rbuf, track_window, chunks); 
	window(tbuf, track_window, chunks);
	fftw_execute(ep); fftw_execute(rp); fftw_execute(tp);
	/* now eout[], rout[], and tout[] are valid */
	do_spline(eout, chunks); do_spline(rout, chunks); do_spline(tout, chunks); 
	free_bufs(ebuf, eout, rbuf, rout, tbuf, tout);
	free(track_window);
	destroy_plans(ep, rp, tp);
	return(0);
}

