#include "fdmf_sonic_reducer.h"

void
destroy_plans( fftw_plan ep, fftw_plan rp, fftw_plan tp ) {
  fftw_destroy_plan( ep );
  fftw_destroy_plan( rp );
  fftw_destroy_plan( tp );
}
