#!/bin/bash

host="ogle.vpn.hexten.net"
fdmf="/home/andy/Works/Media/fdmf"
suff=$(hostname -s)

set -x
./fdmf_dump > dump.$suff
rsync -avzP dump.$suff andy@$host:$fdmf
ssh -C $host "cd $fdmf && ./fdmf_correlator -v --keep 10000 dump.$suff > dups.$suff"
rsync -avzP andy@$host:$fdmf/dups.$suff .
./fdmf_report dups.$suff > report
set +x
