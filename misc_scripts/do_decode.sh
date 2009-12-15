#!/bin/bash
# decode using the speed parms and merged speakers

if [ $# != 2 ] ; then echo "usage: do_decode.sh <cfgtemplate> <testset> (e.g: 1046, PP50, etc)" ; exit ; fi

testset=$2
# nset="n0inf n15dB n10dB"  # my version of names
nset="infdB 15dB 10dB"  # Chanwoo's version
decfg=speed

# substute in proper testset info
grep -v "ctlfile" $1 > $$.temp1
grep -v "reffile" $$.temp1 > $$.temp2
echo "ctlfile = \$ctldir/$testset.ctl" >> $$.temp2
echo "reffile = \$ctldir/$testset.ref" >> $$.temp2

# go through each noise level
for lev in $nset ; do
    sed -e s/NOISELEVEL/$lev/ < $$.temp2 > $$.cfg   # set the current noise level in the config

    ./scripts/decodecep2.pl -config $$.cfg -expid ${lev}_${testset}_$decfg
    tail -2 logdir/${lev}_${testset}_$decfg/${lev}_${testset}_$decfg.align
    tail -2 logdir/${lev}_${testset}_$decfg/${lev}_${testset}_$decfg.log
    cp -p $$.cfg logdir/${lev}_${testset}_$decfg/${lev}_${testset}_$decfg.cfg

done

rm $$.*

#
