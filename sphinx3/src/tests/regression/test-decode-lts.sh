#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-lts.out"

echo "DECODE LTS TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

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
-lts_mismatch 1 "

lmargs="-lm $an4lm/an4.ug.lm.DMP"

clsargs="-lmctlfn $an4lm/an4.ug.cls.lmctl \
-ctl_lm  $an4lm/an4.ctl_lm" 

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "DECODE LTS test" 
else
    fail "DECODE LTS test"
fi



