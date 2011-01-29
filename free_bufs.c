#include "sonic_reducer.h"

void free_bufs(f_c * ebuf, f_c * eout, f_c * rbuf, 
				f_c * rout, f_c * tbuf, f_c * tout) {
	fftw_free(ebuf); 
	fftw_free(eout); 
	fftw_free(rbuf); 
	fftw_free(rout); 
	fftw_free(tbuf); 
	fftw_free(tout);
}
