#!/bin/bash

dump=wrk/sfx-dump

timer="/usr/bin/time --format=sys=%S=user=%S=elapsed=%E"

branch=$(git status | perl -ne '/On\s+branch\s+(\S+)/ && print $1')
echo "Testing $branch"
bmdir="bm/$branch"
rm -rf $bmdir
mkdir -p $bmdir

make clean && make OPTIMIZE="-fprofile-arcs -ftest-coverage"
set -x
$timer ./fdmf_correlator -v --keep 100000 $dump > /dev/null
set +x
gcov fdmf_correlator.c
mv *.gcov $bmdir

# Now do a performance test
make clean && make
for keep in 10 100 1000 10000 100000; do
  log=$bmdir/t$keep.bm
  set -x
  $timer -o $log ./fdmf_correlator -v --keep $keep $dump > /dev/null
  set +x
  cat $log
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

