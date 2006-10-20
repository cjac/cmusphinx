#!/bin/sh
thisdir=`pwd`
. $thisdir/testfuncs.sh

tmpout="test-decode-s2semi.out"

echo "DECODE SPHINX2 TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'E I T G S B U R G H' (don't panic!)"

margs="-mdef $hmmdir/RM1_cd_semi/RM1.1000.mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-mean $hmmdir/RM1_cd_semi/means \
-var $hmmdir/RM1_cd_semi/variances \
-mixw $hmmdir/RM1_cd_semi/mixture_weights \
-tmat $hmmdir/RM1_cd_semi/transition_matrices \
-kdtree $hmmdir/RM1_cd_semi/kdtrees \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-senmgau .s2semi. \
-agc none \
-varnorm no \
-cmn current \
-ds 2 \
-lw 6.5 \
-feat s2_4x"

lmargs="-lm $an4lm/an4.ug.lm.DMP"

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "E I T G S B U R G H" > /dev/null 2>&1; then
    pass "DECODE SPHINX2 test" 
else
    fail "DECODE SPHINX2 test"
fi



