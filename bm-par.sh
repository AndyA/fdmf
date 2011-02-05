#!/bin/bash

for j in 1 4 8 16; do
  db="wrk/j$j.db"
  rm -f $db
  set -x
  time ./fdmf -db $db -j $j sfx
  set +x
done


# vim:ts=2:sw=2:sts=2:et:ft=sh

