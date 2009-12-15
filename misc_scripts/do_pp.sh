#!/usr/bin/env bash
# # compute PP on an utt by utt basis;
# produce a table for mapping id to PP, coverage and WER

if [ $# != 1 ] ; then echo "usage: do_pp.sh <testset> (e.g: 1046, PP50, etc)" ; exit ; fi
testset=$1

ref=traveler_$testset.ref
ctl=traveler_$testset.ctl

CMUCLMTK=/work/air/cmusphinx/cmuclmtk/bin
lm=eng_lm_20090930_final.lm

# compute perplexities and coverage (using hacked evallm)
echo "uttperp -text etc/$ref"  \
|  $CMUCLMTK/evallm -arpa etc/$lm -context etc/cmucuslmtk.ccs \
> $$.do_pp

scripts/extract_PP_coverage.pl $$.do_pp > $$.lmtk_PP_output
rm $$.do_pp

# map uttid to PP and utterance in a 3 column file
pr -m -s \
    -T etc/$ctl \
    $$.lmtk_PP_output \
    etc/$ref \
    > traveler_$testset.pp_map

rm $$.lmtk_PP_output

#

