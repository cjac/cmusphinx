#!/bin/sh

#
# Usage: ./isolatedDigits-ti46.sh > res.out
#
# where 'res.out' is the file you want to redirect the results to
#
# arguments to the batchmetrics program:
#
# ti46.ctl          : a file of all the audio files to decode
# /lab/speech/...   : where the audio files are
# ARGS              : the Sphinx 3 arguments/options file
#

../../bin.sparc-sun-solaris2.8/batchmetrics ./ti46.ctl /lab/speech/sphinx3j/data/ti46/ti20/test/test/raw ARGS


