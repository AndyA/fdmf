#!/bin/bash

time ./fdmf_correlator -v --keep 100000 wrk/sfx-dump > /dev/null
gcov fdmf_correlator.c

# vim:ts=2:sw=2:sts=2:et:ft=sh

