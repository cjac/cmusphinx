#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-mode3.out"

echo "DECODE MODE 3 TEST"
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
-op_mode 3 \
-subvqbeam 1e-02 \
-epl 4 \
-fillprob 0.02 \
-feat 1s_c_d_dd \
-lw 9.5 \
-beam 1e-40 \
-wbeam 1e-20 \
-wend_beam 1e-1 \
-ci_pbeam 1e-5 \
-ds 2 \
-tighten_factor 0.4"

lmargs="-lm $an4lm/an4.ug.lm.DMP"

rm -f $tmpout
rm -f *.lat.gz

run_program sphinx3_decode $margs $lmargs -outlatdir . > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "DECODE MODE 3 test" 
else
    fail "DECODE MODE 3 test"
fi

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs -inlatdir . > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "RESCORE MODE 3 test" 
else
    fail "RESCORE MODE 3 test"
fi

