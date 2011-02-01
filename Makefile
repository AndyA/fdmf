CC =  gcc
CFLAGS = -O2 -W -Wall -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib

COMOBJS = read_from_fd.o \
	pad.o 

SROBJS = chunk_metrics.o \
	calc_chunk_metrics.o \
	do_spline.o \
	window.o \
	setup_window.o \
	setup_bufs.o \
	setup_plans.o \
	free_bufs.o \
	audio_to_fftw.o \
	calc_band_energies.o \
	destroy_plans.o

all : fdmf_sonic_reducer fdmf_correlator

fdmf_correlator : fdmf_correlator.o

fdmf_sonic_reducer : fdmf_sonic_reducer.o $(COMOBJS) $(SROBJS)
	$(CC) $(CFLAGS) -lfftw3 -lm fdmf_sonic_reducer.c -o $@ $(COMOBJS) $(SROBJS) -lfftw3

clean :  
	rm -f *.o fdmf_sonic_reducer fdmf_correlator $(OBJS) tags

.PHONY: tags
tags :
	ctags *.c *.h
