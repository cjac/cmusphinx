#!/bin/tcsh

../4glat \
    -gram 4\
	-lmfn ./data/lm4g.DMP \
	-fdictfn ./data/dico-noyau.filler \
	-dictfn ./data/minus_JAN2005 \
	-ctlfn ./data/bl.fileids \
	-inlatdir ./data//nplpsat20_5 \
	-logbase 1.0003 \
	-min_endfr 1 \
	-noisepen 0.02 \
	-inspen 0.7 \
	-langwt 9.5 \
	-maxlpf 100000 \
	-maxlink	20 \
	-matchsegfn outDagfree.bck \
	-dagfudge 0 \
	-lminmemory 0
