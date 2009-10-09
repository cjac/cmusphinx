#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-2ndstage.out"
tmpconfmatchseg="test-decode-2ndstage.confmatchseg"

echo "DECODE 2ND STAGE TEST"
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
-subvqbeam 1e-02 \
-epl 4 \
-fillprob 0.02 \
-feat 1s_c_d_dd \
-lw 9.5 \
-maxwpf 1 \
-beam 1e-40 \
-pbeam 1e-30 \
-wbeam 1e-20 \
-maxhmmpf 1500 \
-wend_beam 1e-1 \
-ci_pbeam 1e-5 \
-ds 2 \
-tighten_factor 0.4 \
-bestpath yes \
-outlatdir ./ "

lmargs="-lm $an4lm/an4.ug.lm.DMP"

clsargs="-lmctlfn $an4lm/an4.ug.cls.lmctl \
-ctl_lm  $an4lm/an4.ctl_lm" 

rm -f $tmpout
rm -f *.lat.gz *.nbest.gz

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1

grep "BSTPTH:" $tmpout 
grep "BSTXCT" $tmpout 

if grep "BSTPTH:" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "DECODE 2nd stage test"
else
    fail "DECODE 2nd stage test"
fi

#Next try to read the lattices with dag and see whether dag could read it. 

margs="-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-lw 13.0 \
-wip 0.2 \
-ctl $an4lm/an4.ctl \
-inlatdir ./ \
-logbase 1.0003 \
-backtrace 1 "

run_program sphinx3_dag $margs $lmargs >> $tmpout 2>&1 

grep "BSTPTH:" $tmpout 
grep "BSTXCT" $tmpout 

if grep "BSTPTH:" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "DAG after DECODE 1st stage test"
else
    fail "DAG after DECODE 1st stage test"
fi

#Next try to read the lattices with astar and see whether astar could read it. 

margs="-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-wip 0.2 \
-beam 1e-64 \
-nbest 5 \
-ctl $an4lm/an4.ctl \
-inlatdir ./ \
-logbase 1.0003 \
-nbestdir ./  "

if run_program sphinx3_astar $margs $lmargs >> $tmpout 2>&1; then
    pass "ASTAR after DECODE 1st stage dry run test"
else
    fail "ASTAR after DECODE 1st stage dry run test"
fi


#gzip -cfd ./`head -1 $an4lm/an4.ctl`.nbest.gz > ./pittsburgh.nbest 
#compare_table ./pittsburgh.nbest $an4lm/pittsburgh.nbest 
#then echo "ASTAR after DECODE 1st stage dry run test"
#echo "ASTAR after DECODE 1st stage dry run test FAILED"; fi
