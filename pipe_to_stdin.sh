#!/bin/bash
RATE=`grep -obUa $'\xff\x00' $1 | sed -n 2p | awk -F: \
 '{ print int($1 * 15.2) }'`
cat $1 | pv -L $RATE | ./xr25_diag
