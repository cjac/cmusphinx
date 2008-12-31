#!/bin/sh
. ./testfuncs.sh

tmpout="test-conf.out"
tmphypseg="test-conf.hypseg"
tmpconfhypseg="test-conf.confhypseg"

echo "DECODE -> CONFIDENCE TEST"

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
-outlatdir . \
-hypseg $tmphypseg \
"

lmargs="-lm $an4lm/an4.ug.lm.DMP"

rm -f $tmphypseg

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1

margs="-mdef $hub4am/mdef \
-fdict $an4lm/filler.dict \
-dict $an4lm/an4.dict \
-inlatdir ./ \
-logbase 1.003 \
-ctl $an4lm/an4.ctl \
-inhypseg $tmphypseg \
-output $tmpconfhypseg \
-lm $an4lm/an4.ug.lm.DMP \
"

rm -f $tmpconfhypseg

run_program sphinx3_conf $margs >> $tmpout 2>&1 
compare_table "DECODE -> CONF test" $tmpconfhypseg $hub4am/test-conf.confhypseg 2 
