#!/bin/bash
# collate error and perplexities for a specific run
# store results in the specific log folder


if [ $# -ne 2 ] ; then echo "usage: align_PP full|speaker <logroot>" ; exit ; fi

if [ $1 = "full" ] ; then spkr="" ;
elif [ $1 = "speaker" ] ; then spkr="f2caa01 f2uaa01 f3uba01 m2faa01 m2nba01 m3dca01"
else
    echo "first argument must be 'full' or 'speaker'"; exit
fi


log=$2
map=traveler_1046.pp_map
echo "using $map"


if [ -e logdir/$log.wer ] ; then rm logdir/$log.wer ; fi

if [ -z $spkr ] ; then
    scripts/split_align_file.pl logdir/$log/$log.align logdir/$log/$log.wer
else

# extract data from .align file, merge into one file
for s in $spkr ; do
    scripts/split_align_file.pl logdir/${log}_$s/${log}_$s.align logdir/${log}_$s/${log}_$s.wer
    cat logdir/${log}_$s/${log}_$s.wer >>logdir/$log/$log.wer
done
fi

# merge PP and WER data
scripts/merge_wer_PP.pl etc/$map logdir/$log/$log.wer logdir/$log/$log.merged

#