#!/bin/sh
thisdir=`pwd`
. $thisdir/testfuncs.sh

echo "ALLPHONE+MLLR PHONE TG TEST"

margs="-logbase 1.0003 \
-mdef $hub4am/hub4opensrc.6000.mdef \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-mllr $hub4am/mllr_matrices \
-feat 1s_c_d_dd \
-topn 1000 \
-beam 1e-80 \
-senmgau .s3cont. \
-ctl $an4lm/an4.ctl \
-ctlcount 1 \
-cepdir $an4lm/ \
-hyp allphone.match \
-hypseg allphone.matchseg \
-phsegdir ./ \
-phlatdir ./"

lmargs="-lm $an4lm/an4.tg.phone.arpa.DMP "

run_program sphinx3_allphone $margs $lmargs >test-allphone-mllr.out 2>&1
filebase=`head -1 $an4lm/an4.ctl`
compare_table "ALLPHONE+MLLR PHONE TG allp test" $filebase.allp $hub4am/test.allphone.phone_tg.mllr.allp 2 0,1
compare_table "ALLPHONE+MLLR PHONE TG phlat test" $filebase.phlat $hub4am/test.allphone.phone_tg.mllr.phlat
compare_table "ALLPHONE+MLLR PHONE TG match test" allphone.match $hub4am/test.allphone.phone_tg.mllr.match 
compare_table "ALLPHONE+MLLR PHONE TG matchseg test" allphone.matchseg $hub4am/test.allphone.phone_tg.mllr.matchseg 100000

run_program sphinx3_allphone $margs >test-allphone-mllr-loop.out 2>&1
compare_table "ALLPHONE+MLLR PHONE LOOP allp test" $filebase.allp $hub4am/test.allphone.mllr.allp 2 0,1
compare_table "ALLPHONE+MLLR PHONE LOOP phlat test" $filebase.phlat $hub4am/test.allphone.mllr.phlat 
compare_table "ALLPHONE+MLLR PHONE LOOP match test" allphone.match $hub4am/test.allphone.mllr.match 
compare_table "ALLPHONE+MLLR PHONE LOOP matchseg test" allphone.matchseg $hub4am/test.allphone.mllr.matchseg 100000
