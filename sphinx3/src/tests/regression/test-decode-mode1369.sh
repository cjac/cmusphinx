#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-mode1369.out"

echo "DECODE MODE 1369 (DEBUG MODE) TEST"
echo "This matches the current decoding routine call sequence with the default behaviour"

margs="-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-agc none \
-varnorm no \
-cmn current \
-subvqbeam 1e-02 \
-epl 4 \
-fillprob 0.02 \
-feat 1s_c_d_dd \
-lw 9.5 \
-maxwpf 1 \
-beam 1e-40 \
-pbeam 1e-30 \
-wbeam 1e-20 \
-maxhmmpf 1500 \
-wend_beam 1e-1 \
-ci_pbeam 1e-5 \
-ds 2 \
-tighten_factor 0.4 \
-op_mode 1369"

lmargs="-lm $an4lm/an4.ug.lm.DMP"

clsargs="-lmctlfn $an4lm/an4.ug.cls.lmctl \
-ctl_lm  $an4lm/an4.ctl_lm" 

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs  2>&1 | grep "SEARCH DEBUG" |sed "s/\.//g" > $tmpout

if diff $tmpout $hub4am/test.mode1369.dump > /dev/null 2>&1; then
    pass "DECODE MODE 1369 (DEBUG MODE) test"
else
    fail "DECODE MODE 1369 (DEBUG MODE) test"
fi



