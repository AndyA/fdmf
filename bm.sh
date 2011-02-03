#!/bin/bash

branch=$(git status | perl -ne '/On\s+branch\s+(\S+)/ && print $1')
echo "Testing $branch"
bmdir="bm-$branch"
mkdir -p $bmdir
make clean && make OPTIMIZE="-fprofile-arcs -ftest-coverage"
time ./fdmf_correlator -v --keep 100000 wrk/sfx-dump > /dev/null
gcov fdmf_correlator.c
mv *.gcov $bmdir

# vim:ts=2:sw=2:sts=2:et:ft=sh

