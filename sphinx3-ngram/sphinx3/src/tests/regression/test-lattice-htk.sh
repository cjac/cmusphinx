#!/bin/sh
. ./testfuncs.sh

tmpout="test-lattice-htk.out"

echo "HTK LATTICE TEST"

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
-outlatfmt htk \
-latext slf
-outlatdir ./ "

lmargs="-lm $an4lm/an4.ug.lm.DMP"

bn=`head -1 $an4lm/an4.ctl`

rm -f $bn.slf tmp.slf

run_program sphinx3_decode $margs $lmargs > $tmpout 2>&1
sed -ne 's/=/ = /g;/^#.*/ ! p' $bn.slf > tmp.slf && mv -f tmp.slf $bn.slf
sed -ne 's/=/ = /g;/^#.*/ ! p' $an4lm/$bn.slf > tmp.slf
compare_table "HTK lattice test" tmp.slf $bn.slf 0.011
