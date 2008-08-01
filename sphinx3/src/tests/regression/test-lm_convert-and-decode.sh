#!/bin/sh
. ./testfuncs.sh

tmpout="test-lm_convert-and-decode.out"

echo "LM CONVERT DMP  -> DECODE TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

tmpdmp="test-lm_convert-and-decode.DMP"

rm -f $tmpdmp

run_program sphinx3_lm_convert \
-i $an4lm/an4.ug.lm \
-o $tmpdmp \
-ofmt DMP \
> /dev/null 2>&1

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
-tighten_factor 0.4"

lmargs="-lm $tmpdmp"

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "LM_CONVERT DMP -> DECODE test"
else
    fail "LM_CONVERT DMP -> DECODE test"
fi

echo "LM CONVERT DMP32  -> DECODE TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

tmpdmp="test-lm_convert-and-decode.DMP32"

rm -f $tmpdmp

run_program sphinx3_lm_convert \
-i $an4lm/an4.ug.lm \
-o $tmpdmp \
-ofmt DMP32 \
> /dev/null 2>&1

lmargs="-lm $tmpdmp"

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "LM_CONVERT DMP32 -> DECODE test" 
else
    fail "LM_CONVERT DMP32 -> DECODE test"
fi

echo "TXT LM  -> DECODE TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

lmargs="-lm $an4lm/an4.ug.lm"

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "TXT LM  -> DECODE test" 
else
    fail "TXT LM -> DECODE test"
fi

rm -f $tmpdmp

run_program sphinx3_lm_convert \
-i $an4lm/an4.ug.lm \
-ifmt TXT32 \
-o $tmpdmp \
> /dev/null 2>&1

lmargs="-lm $tmpdmp"

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "LM_CONVERT FRO TXT32 -> DECODE test"
else
    fail "LM_CONVERT FRO TXT32 -> DECODE test"
fi

