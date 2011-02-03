CC =  gcc
# Use OPTIMIZE="-g3" for debug build
# Use OPTIMIZE="-fprofile-arcs -ftest-coverage" for gcov build
OPTIMIZE=-O3
CFLAGS = $(OPTIMIZE) -W -Wall -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib

all: fdmf_sonic_reducer fdmf_correlator

fdmf_correlator: fdmf_correlator.o
	$(CC) $(CFLAGS) $< -o $@

fdmf_sonic_reducer: fdmf_sonic_reducer.o
	$(CC) $(CFLAGS) -lfftw3 -lm $< -o $@

clean:  
	rm -f *.o fdmf_sonic_reducer fdmf_correlator $(OBJS) tags *.gcda *.gcno *.gcov *.o *.out

.PHONY: tags
tags:
	ctags *.c *.h

test:
	prove -r t
