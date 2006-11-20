#!/bin/sh
. ./testfuncs.sh

echo "DECODE_ANYTOPO 2ND STAGE TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

tmpout="test-decode_anytopo-2ndstage.out"

run_program sphinx3_decode_anytopo \
-mdef $hub4am/hub4opensrc.6000.mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-mean $hub4am/means \
-var $hub4am/variances \
-mixw $hub4am/mixture_weights \
-tmat $hub4am/transition_matrices \
-lm $an4lm/an4.ug.lm.DMP \
-ctl $an4lm/an4.ctl \
-cepdir $an4lm/ \
-agc none \
-varnorm no \
-cmn current \
-feat 1s_c_d_dd \
-lw 9.5 \
-wip 0.2 \
-beam 1e-80 \
-wbeam 1e-40 \
-outlatdir ./ \
-bestpath 1 \
-bestpathlw 6.5 \
> $tmpout 2>&1
	
grep "BSTPTH" $tmpout
grep "BSTXCT" $tmpout

if grep "BSTPTH" $tmpout |grep "P I T T S B U R G H" >/dev/null 2>&1; then
    pass "DECODE_ANYTOPO 2ND STAGE test"
else 
    fail "DECODE_ANYTOPO 2ND STAGE test"
fi

lmargs="-lm $an4lm/an4.ug.lm.DMP"

#Next try to read the lattices with dag and see whether dag could read it. 

margs="-mdef $hub4am/hub4opensrc.6000.mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-lw 13.0 \
-wip 0.2 \
-ctl $an4lm/an4.ctl \
-inlatdir ./ \
-logbase 1.0003 \
-backtrace 1 "

run_program sphinx3_dag $margs $lmargs > $tmpout 2>&1 

grep "BSTPTH:" $tmpout 
grep "BSTXCT" $tmpout 

if grep "BSTPTH:" $tmpout |grep "P I T T S B U R G H" > /dev/null 2>&1; then
    pass "DAG after DECODE_ANYTOPO 1st stage test" 
else
    fail "DAG after DECODE_ANYTOPO 1st stage test" 
fi

#Next try to read the lattices with astar and see whether astar could read it. 

margs="-mdef $hub4am/hub4opensrc.6000.mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-wip 0.2 \
-beam 1e-64 \
-nbest 5 \
-ctl $an4lm/an4.ctl \
-inlatdir ./ \
-logbase 1.0003 \
-nbestdir .  "

if run_program sphinx3_astar $margs $lmargs > $tmpout 2>&1; then
    pass "ASTAR after DECODE_ANYTOPO 1st stage dry run test"
else
    fail "ASTAR after DECODE_ANYTOPO 1st stage dry run test"
fi


#gzip -cfd ./`head -1 $an4lm/an4.ctl`.nbest.gz > ./pittsburgh.nbest 
#compare_table ./pittsburgh.nbest $an4lm/pittsburgh.nbest 
#then echo "ASTAR after DECODE_ANYTOPO 1st stage test"
#echo "ASTAR after DECODE_ANYTOPO 1st stage test FAILED"; fi


