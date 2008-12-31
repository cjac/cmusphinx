#!/bin/sh
. ./testfuncs.sh

echo "DECODE_ANYTOPO+MLLR TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

tmpout="test-mllr-anytopo.out"

rm -f $tmpout

run_program sphinx3_decode_anytopo \
-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-mllr $hub4am/mllr_matrices \
-lm $an4lm/an4.ug.lm.DMP \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-agc none \
-varnorm no \
-cmn current \
-feat 1s_c_d_dd \
-lw 9.5 \
-wip 0.2 \
-beam 1e-80 \
-wbeam 1e-40 \
> $tmpout 2>&1
	
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" >/dev/null 2>&1; then
    pass "DECODE_ANYTOPO+MLLR test" 
else
    fail "DECODE_ANYTOPO+MLLR test"
fi
