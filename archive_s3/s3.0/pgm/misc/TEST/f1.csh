#!/bin/csh

if ($#argv < 2) then
    echo "Usage: $0 pgm output"
    exit
endif

set echo
limit coredumpsize 0

set PGM = /tmp/$1:t.$$
set logfile = $2.log

cp $1 $PGM

$PGM \
	-dict /net/alf20/local_tmp/lat/dt96-pe/nphc-ci/51k+208ap+top-ac.dict \
	-fdict /net/alf20/local_tmp/lat/dt96-pe/nphc-ci/filler-dt96+3fp.dict \
	-latdir /net/alf20/local_tmp/lat/dt96-pe/nphc-ci \
	-mdef /net/alf20/local_tmp/lat/dt96-pe/nphc-ci/bn92-96+62_all_tr-51k+3fp-fxwd.mdef\
	-ref /net/alf20/usr/rkm/eval96/dt96-pe/ref-sorted-f1-good-nofp.trn >& $logfile

rm $PGM
