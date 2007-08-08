#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-s2semi.out"

echo "DECODE SPHINX2 TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'E I T G S B U R G H' (don't panic!)"

margs="-mdef $hmmdir/RM1_cd_semi/RM1.1000.mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-hmm $hmmdir/RM1_cd_semi \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-senmgau .s2semi. \
-ds 2 \
-lw 6.5"

lmargs="-lm $an4lm/an4.ug.lm.DMP"

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "E I T G S B U R G H" > /dev/null 2>&1; then
    pass "DECODE SPHINX2 test" 
else
    fail "DECODE SPHINX2 test"
fi



