CC =  gcc
CFLAGS = -O2 -W -Wall -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib

all : fdmf_sonic_reducer fdmf_correlator

fdmf_correlator : fdmf_correlator.o
	$(CC) $(CFLAGS) -lpthread fdmf_correlator.o -o $@

fdmf_sonic_reducer : fdmf_sonic_reducer.o
	$(CC) $(CFLAGS) -lfftw3 -lm fdmf_sonic_reducer.o -o $@

clean :  
	rm -f *.o fdmf_sonic_reducer fdmf_correlator $(OBJS) tags

.PHONY: tags
tags :
	ctags *.c *.h
