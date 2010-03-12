#!/bin/sh

. ../testfuncs.sh

bn=`basename $0 .sh`

echo "Test: $bn"
run_program pocketsphinx_batch \
    -hmm $model/hmm/en_US/hub4_wsj_ptm256_3s_8k.cd_ptm_5000 \
    -lm $model/lm/en_US/wsj/wlist5o.3e-7.vp.tg.lm.DMP \
    -dict $model/lm/en_US/cmudict.0.6d \
    -ctl $data/wsj/test5k.s1.ctl \
    -cepdir $data/wsj \
    -cepext .mfc \
    -hyp $bn.match \
    -backtrace yes \
    -pl_window 2 \
    > $bn.log 2>&1

# Test whether it actually completed
if [ $? = 0 ]; then
    pass "run"
else
    fail "run"
fi

# Check the decoding results
grep AVERAGE $bn.log
$tests/word_align.pl -i $data/wsj/test5k.s1.lsn $bn.match | grep 'TOTAL Percent'
compare_table "match" $data/wsj/$bn.match $bn.match 1000000