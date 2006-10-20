#!/bin/sh
thisdir=`dirname $0`
. $thisdir/testfuncs.sh

tmpout="test-decode-mode2.out"
tmpmatch="test-decode-mode2.match"

echo "DECODE FSG TEST"

margs=" \
-mdef  $hmmdir/tidigits/wd_dependent_phone.500.mdef \
-dict $hmmdir/tidigits/dictionary \
-fdict  $hmmdir/tidigits/fillerdict \
-mean  $hmmdir/tidigits/wd_dependent_phone.cd_continuous_8gau/means \
-var   $hmmdir/tidigits/wd_dependent_phone.cd_continuous_8gau/variances \
-mixw  $hmmdir/tidigits/wd_dependent_phone.cd_continuous_8gau/mixture_weights \
-tmat  $hmmdir/tidigits/wd_dependent_phone.cd_continuous_8gau/transition_matrices \
-cepdir $hmmdir/tidigits/cepstra/ \
-hyp $tmpmatch \
-agc none \
-varnorm no \
-cmn current \
-lw 9.5 \
-op_mode 2 \
"

fsgargs=" \
-fsg $hmmdir/tidigits/test.digits.fsg \
-ctl $hmmdir/tidigits/tidigits.length.arb.regression \
"

run_program sphinx3_decode $margs $fsgargs > $tmpout 2>&1
compare_table "DECODE FSG ARBITRARY LENGTH DIGIT STRING test" $tmpmatch $hmmdir/tidigits/tidigits.length.arb.result


fsgargs=" \
-fsg $hmmdir/tidigits/test.iso.digits.fsg \
-ctl $hmmdir/tidigits/tidigits.length.1.regression \
"

run_program sphinx3_decode $margs $fsgargs > $tmpout 2>&1
compare_table "DECODE FSG LENGTH 1 DIGIT (ISOLATED DIGIT) STRING test" $tmpmatch $hmmdir/tidigits/tidigits.length.1.result 0 0


fsgargs=" \
-fsg $hmmdir/tidigits/test.2.digits.fsg \
-ctl $hmmdir/tidigits/tidigits.length.2.regression \
"

run_program sphinx3_decode $margs $fsgargs > $tmpout 2>&1
compare_table "DECODE FSG LENGTH 2 DIGIT STRING test" $tmpmatch $hmmdir/tidigits/tidigits.length.2.result 0 0,1
