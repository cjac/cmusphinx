#!/bin/sh
. ./testfuncs.sh

echo "LIVEPRETEND CLASS-BASED LM TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

tmpout="test-livepretend-clm.out"

rm -f $tmpout

run_program sphinx3_livepretend \
$an4lm/an4.ctl \
$an4lm \
$builddir/model/lm/an4/args.an4.test.cls > $tmpout 2>&1

grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" >/dev/null 2>&1; then
    pass "LIVEPRETEND CLASS-BASED LM test"
else
    fail "LIVEPRETEND CLASS-BASED LM test"
fi

