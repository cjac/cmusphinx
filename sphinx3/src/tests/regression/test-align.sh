#!/bin/sh
. ./testfuncs.sh

echo "ALIGN TEST simple"

tmpout="test-align-simple.seg"

rm -f $tmpout

#Simple test
run_program sphinx3_align \
-logbase 1.0003 \
-mdef $hub4am/mdef \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-feat 1s_c_d_dd \
-topn 1000 \
-beam 1e-80 \
-senmgau .s3cont. \
-agc max \
-mdef_fillers yes \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-insent $an4lm/align.correct \
-outsent $tmpout \
-wdsegdir ./ \
-phsegdir ./ \
-stsegdir ./ \
-s2stsegdir ./ \
> test-align-simple.out 2>&1

filebase=`head -1 $an4lm/an4.ctl`
compare_table "ALIGN simple output test" $tmpout  $hub4am/test.align.out
compare_table "ALIGN simple wdseg test" $filebase.wdseg $hub4am/test.align.wdseg 0 0,1
compare_table "ALIGN simple phseg test" $filebase.phseg $hub4am/test.align.phseg 0 0,1
tar cf test-align-simple.tar $tmpout $filebase.wdseg $filebase.phseg

echo "ALIGN TEST cepext"

tmpout="test-align-cepext.seg"

rm -f $tmpout

#Program we used -agc max. This is an exception
#test for extension
run_program sphinx3_align \
-logbase 1.0003 \
-mdef $hub4am/mdef \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-feat 1s_c_d_dd \
-topn 1000 \
-beam 1e-80 \
-agc max \
-senmgau .s3cont. \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-cepext .abcd \
-insent $an4lm/align.correct \
-mdef_fillers yes \
-outsent $tmpout \
-wdsegdir ./ \
-phsegdir ./ \
-stsegdir ./ \
-s2stsegdir ./ \
-s2cdsen yes \
> test-align-cepext.out 2>&1

filebase=`head -1 $an4lm/an4.ctl`
compare_table "ALIGN cepext output test" $tmpout  $hub4am/test.align.out
compare_table "ALIGN cepext wdseg test" $filebase.wdseg $hub4am/test.align.wdseg 0 0,1
compare_table "ALIGN cepext phseg test" $filebase.phseg $hub4am/test.align.phseg 0 0,1
tar cf test-align-cepext.tar $tmpout $filebase.wdseg $filebase.phseg
