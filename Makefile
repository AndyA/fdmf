CC =  gcc
# Use OPTIMIZE="-g3" for debug build
# Use OPTIMIZE="-fprofile-arcs -ftest-coverage" for gcov build
OPTIMIZE=-O3
CFLAGS = $(OPTIMIZE) -W -Wall -I/usr/local/include -L/usr/local/lib -I/opt/local/include -L/opt/local/lib

all: fdmf_sonic_reducer fdmf_correlator tools/closure

tools/closure.o: tools/closure.h

tools/closure: tools/closure.o
	$(CC) $(CFLAGS) $< -o $@

anchor_compar.c anchor_compar.h : anchor_compar.cl
	perl tools/closure.pl anchor_compar.cl

fdmf_correlator.c: anchor_compar.h

fdmf_correlator: fdmf_correlator.o anchor_compar.o
	$(CC) $(CFLAGS) $+ -o $@

fdmf_sonic_reducer: fdmf_sonic_reducer.o
	$(CC) $(CFLAGS) -lfftw3 -lm $+ -o $@

clean:  
	rm -f *.o \
	  fdmf_sonic_reducer fdmf_correlator \
	  $(OBJS) tags *.gcda *.gcno *.gcov *.o *.out \
	  anchor_compar.c anchor_compar.h

.PHONY: tags
tags:
	ctags *.c *.h

test: all
	prove -r t
