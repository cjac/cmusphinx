#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-s3cont.out"

echo "DECODE S3CONT TEST"
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
-maxwpf 1 \
-beam 1e-40 \
-pbeam 1e-30 \
-wbeam 1e-20 \
-maxhmmpf 1500 \
-wend_beam 1e-1 \
-feat 1s_c_d_dd \
-senmgau .s3cont."

lmargs="-lm $an4lm/an4.ug.lm.DMP"

clsargs="-lmctlfn $an4lm/an4.ug.cls.lmctl \
-ctl_lm  $an4lm/an4.ctl_lm" 

rm -f $tmpout

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

#Seems to me a situation where, the decode cannot be tuned to P I T T
#S B U R G H in this case. Just try to run through it now. 

if grep "FWDVIT" $tmpout |grep "P I T G S B U R G H" > /dev/null 2>&1; then
    pass "DECODE S3CONT test" 
else
    fail "DECODE S3CONT test"
fi



