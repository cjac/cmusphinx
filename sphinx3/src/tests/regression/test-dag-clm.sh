#!/bin/sh
. ./testfuncs.sh

echo "DAG CLASS-BASED LM TEST"
tmpout="test-dag-clm.out"

margs="-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-lw 13.0 \
-wip 0.2 \
-ctl $an4lm/an4.ctl.platform_independent \
-inlatdir $an4lm/ \
-logbase 1.0003 \
-backtrace 1 "

clsargs="-lmctlfn $builddir/model/lm/an4/an4.ug.cls.lmctl -ctl_lm $an4lm/an4.ctl_lm" 

rm -f $tmpout

run_program sphinx3_dag $margs $clsargs > $tmpout 2>&1 

grep "BSTPTH:" $tmpout 
grep "BSTXCT" $tmpout 

if grep "BSTPTH:" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "DAG CLASS-BASED LM test"
else
    fail "DAG CLASS-BASED LM test"
fi
