#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-mode1.out"

echo "MODE1 DECODE TEST"

margs="-logbase 1.0003 \
-hmm $hub4am \
-pbeam 1e-30 \
-feat 1s_c_d_dd \
-dict $an4lm/an4.phone.dict \
-fdict $an4lm/filler.dict \
-ctl $an4lm/an4.ctl \
-mdef_fillers yes \
-ctlcount 1 \
-outlatdir . \
-phsegdir . \
-cepdir $an4lm/"

lmargs="-lm $an4lm/an4.tg.phone.arpa.DMP "

rm -f $tmpout
rm -f test-decode-mode1.match

run_program sphinx3_decode -op_mode 1 $margs -hyp test-decode-mode1.match \
    >> $tmpout 2>&1

filebase=`head -1 $an4lm/an4.ctl`
compare_table "MODE1 LOOP match test" test-decode-mode1.match \
    $hub4am/test.allphone.match 

rm -f test-decode-mode1-tg.match

run_program sphinx3_decode -op_mode 1 $margs $lmargs -hyp test-decode-mode1-tg.match \
    >> $tmpout 2>&1

compare_table "MODE1 TG match test" test-decode-mode1-tg.match \
    $hub4am/test.allphone.phone_tg.match 

echo "MODE1 BESTPATH TG match test"
run_program sphinx3_decode -op_mode 1 $margs $lmargs -bestpath yes \
    >> $tmpout 2>&1

