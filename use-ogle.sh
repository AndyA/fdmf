#!/bin/bash

host="ogle.vpn.hexten.net"
fdmf="/home/andy/Works/Media/fdmf"

./fdmf_dump > dump
rsync -avzP dump andy@$host:$home
ssh -C $host "cd $home/dups && ./fdmf_correlator -v --keep 10000 dump > dups"
rsync -avzP andy@$host:$home/dups .
./fdmf_report dups > report
