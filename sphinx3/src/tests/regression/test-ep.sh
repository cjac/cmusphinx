#!/bin/sh
thisdir=`dirname $0`
. $thisdir/testfuncs.sh

echo "EP TEST"
tmplog="test-ep.log"
tmpout="test-ep.out"

run_program sphinx3_ep \
-frate 105 \
-mean $sourcedir/model/ep/means \
-var $sourcedir/model/ep/variances \
-mixw $sourcedir/model/ep/mixture_weights \
-input $sourcedir/model/ep/chan3.mfc \
> $tmplog 2>&1

grep BLAH $tmplog > $tmpout

compare_table "EP test" $tmpout $sourcedir/model/ep/ep.result 0.1

 
