#!/bin/bash
# run decoding over each speaker individually
# this is a wrapper for decodecep2.pl and takes the same arguments
# [20091009] (air)

if [ $# -ne 2 ] ; then  echo "usage: by_speaker <cfg> <id_stub>"; exit; fi

# the speakers in the traveler dev_test...
spkr="f2caa01 f2uaa01 f3uba01 m2faa01 m2nba01 m3dca01"

for s in $spkr ; do 

    # remove pointers from base file
    grep -v "ctlfile" $1 > tmp.$$
    grep -v "reffile" tmp.$$ > tempcfg.$$
    rm tmp.$$
    # put in the speaker-specific ones
    echo "ctlfile = \$ctldir/byspeaker/$s.fileid" >>tempcfg.$$
    echo "reffile = \$ctldir/byspeaker/$s.ref" >>tempcfg.$$

    expid=$2_$s
    scripts/decodecep2.pl -config tempcfg.$$ -expid $expid
    #remember this...
    mv tempcfg.$$ logdir/$expid/$expid.cfg  # save the config file

    # quick look at result
    echo "--- $expid --------------------------------------------------"
    tail logdir/$expid/$expid.align
    echo ""

done

#
