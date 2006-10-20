#!/bin/sh
thisdir=`dirname $0`
. $thisdir/testfuncs.sh

echo "GAUSUBVQ TEST"
tmplog="test-gausubvq.log"
tmpout="test-gausubvq.out"

echo "This will compare the answer with a pre-generated svq file"

run_program gausubvq \
-mean $hub4am/means \
-var  $hub4am/variances \
-mixw  $hub4am/mixture_weights \
-svspec 0-38 \
-iter 20 \
-svqrows 16 \
-seed 1111 \
-subvq $tmplog

grep -v "#" $tmplog > $tmpout
compare_table "GAUSUBVQ test" $tmpout $hub4am/test.subvq 
 
