#!/bin/sh
. ./testfuncs.sh

echo "ALLPHONE TEST"

margs="-logbase 1.0003 \
-hmm $hub4am \
-feat 1s_c_d_dd \
-dict $an4lm/an4.phone.dict \
-fdict $an4lm/filler.dict \
-ctl $an4lm/an4.ctl \
-ctlcount 1 \
-cepdir $an4lm/ \
-hyp allphone.match \
-hypseg allphone.matchseg \
-phsegdir ./"

lmargs="-lm $an4lm/an4.tg.phone.arpa.DMP "

run_program sphinx3_allphone $margs $lmargs > test-allphone.out 2>&1
filebase=`head -1 $an4lm/an4.ctl`
compare_table "ALLPHONE PHONE TG allp test" $filebase.allp $hub4am/test.allphone.phone_tg.allp 2 0,1
compare_table "ALLPHONE PHONE TG match test" allphone.match $hub4am/test.allphone.phone_tg.match 
compare_table "ALLPHONE PHONE TG matchseg test" allphone.matchseg $hub4am/test.allphone.phone_tg.matchseg 100000

run_program sphinx3_allphone $margs >test-allphone-loop.out 2>&1
compare_table "ALLPHONE PHONE LOOP allp test" $filebase.allp $hub4am/test.allphone.allp 2 0,1
compare_table "ALLPHONE PHONE LOOP match test" allphone.match $hub4am/test.allphone.match 
compare_table "ALLPHONE PHONE LOOP matchseg test" allphone.matchseg $hub4am/test.allphone.matchseg 100000
