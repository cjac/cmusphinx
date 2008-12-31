#!/bin/sh
. ./testfuncs.sh

echo "ASTAR TEST"
tmpout="test-astar.out"

margs="-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-wip 0.2 \
-beam 1e-64 \
-nbest 5 \
-ctl $an4lm/an4.ctl.platform_independent \
-inlatdir $an4lm/ \
-logbase 1.0003 \
-nbestdir .  "

lmargs="-lm $an4lm/an4.ug.lm.DMP"

rm -f pittsburgh.nbest.gz pittsburgh.nbest

run_program sphinx3_astar $margs $lmargs > $tmpout 2>&1 

gzip -cfd ./pittsburgh.nbest.gz > ./pittsburgh.nbest 
compare_table "ASTAR test" ./pittsburgh.nbest $an4lm/pittsburgh.nbest 
