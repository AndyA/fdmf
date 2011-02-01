#include "fdmf_sonic_reducer.h"

void setup_bufs(fftw_complex **ebuf, fftw_complex **eout, fftw_complex **rbuf, 
			fftw_complex **rout, fftw_complex **tbuf, fftw_complex **tout) {
	*ebuf = fftw_malloc(sizeof(fftw_complex) * MAXCHUNKS); assert(ebuf != NULL);
	*eout = fftw_malloc(sizeof(fftw_complex) * MAXCHUNKS); assert(eout != NULL);
	*rbuf = fftw_malloc(sizeof(fftw_complex) * MAXCHUNKS); assert(rbuf != NULL);
	*rout = fftw_malloc(sizeof(fftw_complex) * MAXCHUNKS); assert(rout != NULL);
	*tbuf = fftw_malloc(sizeof(fftw_complex) * MAXCHUNKS); assert(tbuf != NULL);
	*tout = fftw_malloc(sizeof(fftw_complex) * MAXCHUNKS); assert(tout != NULL);
}
