#!/bin/sh
. ./testfuncs.sh

tmpout="test-decode-mode2.out"
tmpmatch="test-decode-mode2.match"

echo "DECODE FSG TEST"

margs=" \
-dict $hmmdir/tidigits/dictionary \
-fdict  $hmmdir/tidigits/fillerdict \
-hmm  $hmmdir/tidigits/wd_dependent_phone.cd_continuous_8gau/ \
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

rm -f $tmpmatch

run_program sphinx3_decode $margs $fsgargs > $tmpout 2>&1
compare_table "DECODE FSG ARBITRARY LENGTH DIGIT STRING test" $tmpmatch $hmmdir/tidigits/tidigits.length.arb.result

fsgargs=" \
-fsg $hmmdir/tidigits/test.iso.digits.fsg \
-ctl $hmmdir/tidigits/tidigits.length.1.regression \
"

rm -f $tmpmatch

run_program sphinx3_decode $margs $fsgargs > $tmpout 2>&1
compare_table "DECODE FSG LENGTH 1 DIGIT (ISOLATED DIGIT) STRING test" $tmpmatch $hmmdir/tidigits/tidigits.length.1.result


fsgargs=" \
-fsg $hmmdir/tidigits/test.2.digits.fsg \
-ctl $hmmdir/tidigits/tidigits.length.2.regression \
"

rm -f $tmpmatch

run_program sphinx3_decode $margs $fsgargs > $tmpout 2>&1
compare_table "DECODE FSG LENGTH 2 DIGIT STRING test" $tmpmatch $hmmdir/tidigits/tidigits.length.2.result
