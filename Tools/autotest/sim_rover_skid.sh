#!/bin/bash

set -x

killall -q APMrover2.elf
pkill -f sim_rover.py
set -e

: ${APMTERM=xterm}

autotest=$(dirname $(readlink -e $0))
pushd $autotest/../../APMrover2
make clean sitl

tfile=$(mktemp)
(
echo r
) > $tfile
#$APMTERM -e "gdb -x $tfile --args /tmp/APMrover2.build/APMrover2.elf" &
$APMTERM -e /tmp/APMrover2.build/APMrover2.elf &
#$APMTERM -e "valgrind --db-attach=yes -q /tmp/APMrover2.build/APMrover2.elf" &
sleep 2
rm -f $tfile
#$APMTERM -e "../Tools/autotest/pysim/sim_rover.py --home=-35.362938,149.165085,584,270 --rate=400" &
$APMTERM -e "../Tools/autotest/pysim/sim_rover.py --skid-steering --home=-35.362938,149.165085,584,270 --rate=400" &
sleep 2
popd
mavproxy.py --aircraft=test --master tcp:127.0.0.1:5760 --sitl 127.0.0.1:5501 --out 127.0.0.1:14550 --out 127.0.0.1:14551
