#include "fdmf_sonic_reducer.h"

void setup_plans(int c, f_p *ep, f_p *rp, f_p *tp, 
	f_c * ebuf, f_c * rbuf, f_c * tbuf, 
	f_c * eout, f_c * rout, f_c * tout) {
	*ep = fftw_plan_dft_1d(c, ebuf, eout, FFTW_FORWARD, FFTW_ESTIMATE);
	*rp = fftw_plan_dft_1d(c, rbuf, rout, FFTW_FORWARD, FFTW_ESTIMATE);
	*tp = fftw_plan_dft_1d(c, tbuf, tout, FFTW_FORWARD, FFTW_ESTIMATE);
}
