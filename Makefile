# This makefile should not exist as such.  There should be a configure script.

CC =  gcc
CFLAGS = -O2 -W -Wall -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib

COMOBJS = read_from_fd.o \
	pad.o 

VPOBJS = prescan_db.o \
	process_args.o \
	print_usage.o \
	run_tests.o \
	setup_basehashes.o \
	setup_bitcount_tbl.o \
	setup_datablock.o \
	setup_db.o \
	setup_filename_element.o \
	load_datablock.o \
	setup_filenames_array.o 

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

all : vector_pairs sonic_reducer fdmf_correlator

fdmf_correlator : fdmf_correlator.o

sonic_reducer : sonic_reducer.o $(COMOBJS) $(SROBJS)
	$(CC) $(CFLAGS) -lfftw3 -lm sonic_reducer.c -o $@ $(COMOBJS) $(SROBJS) -lfftw3

vector_pairs : vector_pairs.h vector_pairs.o $(VPOBJS)
	$(CC) $(CFLAGS) vector_pairs.c -lm -lgdbm -o $@ $(VPOBJS) 

clean :  
	rm -f *.o vector_pairs sonic_reducer fdmf_correlator $(OBJS) tags

.PHONY: tags
tags :
	ctags *.c *.h
