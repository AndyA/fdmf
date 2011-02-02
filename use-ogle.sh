#!/bin/bash

host="ogle.vpn.hexten.net"
fdmf="/home/andy/Works/Media/fdmf"
suff=$(hostname -s)
db=db

set -x
./fdmf_dump --db $db > dump.$suff
rsync -avzP dump.$suff andy@$host:$fdmf
ssh -C $host "cd $fdmf && git pull origin master && make && ./fdmf_correlator -v --keep 10000 dump.$suff > dups.$suff"
rsync -avzP andy@$host:$fdmf/dups.$suff .
./fdmf_report --db $db dups.$suff > report
set +x

# vim:ts=2:sw=2:sts=2:et:ft=sh
