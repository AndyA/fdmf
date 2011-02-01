#!/bin/bash

host="ogle.vpn.hexten.net"
fdmf="/home/andy/Works/Media/fdmf"

set -x
./fdmf_dump > dump
rsync -avzP dump andy@$host:$fdmf
ssh -C $host "cd $fdmf && ./fdmf_correlator -v --keep 10000 dump > dups"
rsync -avzP andy@$host:$fdmf/dups .
./fdmf_report dups > report
set +x
