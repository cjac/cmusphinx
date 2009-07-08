#!/bin/sh
. ./testfuncs.sh

echo "DP TEST"
tmpout="test-dp.out"

echo "TEST THE WORD ALIGNMENT ROUTINE. "

rm -f $tmpout

run_program sphinx3_dp \
-hypfile $hub4am/test.dp.hyp \
-reffile $hub4am/test.dp.ref \
> $tmpout 

compare_table "DP SIMPLE test" $tmpout $hub4am/test.dp.simple.log 
 

#run_program sphinx3_dp \
#-d 1 \
#-hypfile $hub4am/test.dp.hyp \
#-reffile $hub4am/test.dp.ref \
#> $tmpout

#compare_table $tmpout $hub4am/test.dp.detail.log 
#then echo "DP DETAIL test"
#echo "DP DETAIL test FAILED"; fi 

