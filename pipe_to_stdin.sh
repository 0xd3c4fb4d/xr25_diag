#!/bin/bash
# frame length is computed subtracting offsets of the first two frames
RATE=`grep -obUa $'\xff\x00' $1 | sed -n 1,2p | awk -F: \
 '{ if (NR == 1) frame0_off = $1; else print int(($1 - frame0_off) * 15.2) }'`
cat $1 | pv -L $RATE | ./xr25_diag
