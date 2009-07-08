#!/bin/sh
. ./testfuncs.sh

echo "LIVEPRETEND TEST"
echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

tmpout="test-livepretend.out"

rm -f $tmpout

run_program sphinx3_livepretend \
$an4lm/an4.ctl \
$an4lm \
$builddir/model/lm/an4/args.an4.test > $tmpout 2>&1

grep "FWDVIT" $tmpout
grep "FWDXCT" $tmpout

if grep "FWDVIT" $tmpout |grep "P I T T S B U R G H" >/dev/null 2>&1; then
    pass "LIVEPRETEND test"
else
    fail "LIVEPRETEND test"
fi
