#ifndef _SONIC_REDUCER_H
#define _SONIC_REDUCER_H
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>
#include <assert.h>
#include "common.h"

#define CHUNKSAMPS 11025
#define CHUNKBYTES 4 * CHUNKSAMPS
#define NUM_BANDS 4
#define MAXCHUNKS 65536         /* 4.5 hours */

typedef fftw_complex f_c;
typedef fftw_plan f_p;

void chunk_metrics( double *, f_c *, f_c *, f_c *, int );
int calc_chunk_metrics( f_c *, f_c *, f_c * );
void do_spline( f_c *, int );
void print_vec( f_c *, int );
double *setup_window( int );
void window( f_c *, double *, int );
void setup_bufs( f_c **, f_c **, f_c **, f_c **, f_c **, f_c ** );
void setup_plans( int, f_p *, f_p *, f_p *, f_c *, f_c *, f_c *,
                  f_c *, f_c *, f_c * );
void free_bufs( f_c *, f_c *, f_c *, f_c *, f_c *, f_c * );
void free_plans( f_p, f_p, f_p, int );
void audio_to_fftw( char *, f_c * );
void destroy_plans( f_p, f_p, f_p );
void calc_band_energies( f_c *, double *be );
#endif                          /* !_SONIC_REDUCER_H */
