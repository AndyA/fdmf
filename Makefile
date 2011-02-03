CC =  gcc
CFLAGS = -O3 -W -Wall -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib
# CFLAGS = -W -Wall -fprofile-arcs -ftest-coverage -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib

all : fdmf_sonic_reducer fdmf_correlator

fdmf_correlator : fdmf_correlator.o
	$(CC) $(CFLAGS) -lpthread $< -o $@

fdmf_sonic_reducer : fdmf_sonic_reducer.o
	$(CC) $(CFLAGS) -lfftw3 -lm $< -o $@

clean :  
	rm -f *.o fdmf_sonic_reducer fdmf_correlator $(OBJS) tags *.gcda *.gcno *.gcov *.o *.out

.PHONY: tags
tags :
	ctags *.c *.h
