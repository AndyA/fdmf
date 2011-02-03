#!/bin/sh

time \
  ./fdmf_correlator -v --keep 10000 wrk/sfx-dump > /dev/null

# vim:ts=2:sw=2:sts=2:et:ft=sh

