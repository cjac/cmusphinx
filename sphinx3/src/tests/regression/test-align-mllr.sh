#!/bin/sh
. ./testfuncs.sh

tmpout="test-align-mllr.out"
tmplog="test-align-mllr.log"

echo "ALIGN+MLLR TEST simple"

rm -f $tmpout

run_program sphinx3_align \
-logbase 1.0003 \
-mdef $hub4am/mdef \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-mllr $hub4am/mllr_matrices \
-feat 1s_c_d_dd \
-mdef_fillers yes \
-agc max \
-topn 1000 \
-beam 1e-80 \
-senmgau .s3cont. \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-insent $an4lm/align.correct \
-outsent $tmpout \
-wdsegdir ./ \
-phsegdir ./ \
>$tmplog 2>&1 

compare_table "ALIGN+MLLR simple output test" $tmpout $hub4am/test.align.mllr.out 
