SUBDIRS = src \
	doc \
	scripts \
	model

EXTRA_DIST = autogen.sh \
	win32/batch/an4.ctl \
	win32/batch/sphinx3-simple.bat \
	win32/batch/sphinx3-test.bat \
	win32/msdev/libs3audio/libs3audio.dsp \
	win32/msdev/libs3decoder/libs3decoder.dsp \
	win32/msdev/libutil/libutil.dsp \
	win32/msdev/programs/gausubvq/gausubvq.dsp \
	win32/msdev/programs/programs.dsw \
	win32/msdev/programs/livedecode/livedecode.dsp \
	win32/msdev/programs/livepretend/livepretend.dsp

# Add an item for each and every test target to the CLEANFILES variable
CLEANFILES = test.out \
	test-align.out \
	test-allphone.out \
	test-astar.out \
	test-dag.out \
	test-decode.out \
	test-full.out \
	test-gausubvq.out \
	test-livepretend.out \
	test-simple.out \
	pittsburgh.littleendian.allp \
	pittsburgh.littleendian.phseg \
	pittsburgh.littleendian.wdseg \
	pittsburgh.nbest.gz

test-simple : test-livepretend

test: test-simple

test-full : test-decode test-align test-allphone test-livepretend test-astar test-dag test-gausubvq

test-livepretend:
	@echo "LIVEPRETEND TEST"
	@echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

	$(LIBTOOL) --mode=execute src/programs/livepretend \
		$(top_srcdir)/model/lm/an4/an4.ctl \
		$(top_srcdir)/model/lm/an4 \
		$(top_srcdir)/model/lm/an4/args.an4.test > $@.out 2>&1 
	@grep "P I T T S B U R G H (" $@.out 
	@grep "FWDXCT" $@.out 

test-decode:
	@echo "DECODE TEST"
	@echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

	$(LIBTOOL) --mode=execute src/programs/decode \
	       -mdef $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
	       -fdict $(top_srcdir)/model/lm/an4/filler.dict \
	       -dict $(top_srcdir)/model/lm/an4/an4.dict \
	       -mean $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
	       -var $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
	       -mixw $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
	       -tmat $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices \
	       -lm $(top_srcdir)/model/lm/an4/an4.ug.lm.DMP \
	       -ctl $(top_srcdir)/model/lm/an4/an4.ctl \
	       -cepdir $(top_srcdir)/model/lm/an4/ \
	       -agc none \
	       -varnorm no \
	       -cmn current \
	       -subvqbeam 1e-02 \
	       -epl 4 \
	       -fillprob 0.02 \
	       -feat 1s_c_d_dd \
	       -lw 9.5 \
	       -maxwpf 1 \
	       -beam 1e-40 \
	       -pbeam 1e-30 \
	       -wbeam 1e-20 \
	       -maxhmmpf 1500 \
	       -wend_beam 1e-1 \
	       -ci_pbeam 1e-3 \
	       -ds 2  > $@.out 2>&1

	@grep "P I T T S B U R G H (" $@.out 
	@grep "FWDXCT" $@.out 

test-align:
	echo "ALIGN TEST"
	echo "THIS IS JUST A FUNCTION TEST"
	$(LIBTOOL) --mode=execute src/programs/align \
	    -logbase 1.0003 \
	    -mdeffn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -meanfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
            -varfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
            -mixwfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
            -tmatfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices \
	    -feat 1s_c_d_dd \
	    -topn 1000 \
	    -beam 1e-80 \
	    -senmgaufn .cont. \
            -fdictfn $(top_srcdir)/model/lm/an4/filler.dict \
            -dictfn $(top_srcdir)/model/lm/an4/an4.dict \
	    -ctlfn $(top_srcdir)/model/lm/an4/an4.ctl \
	    -cepdir $(top_srcdir)/model/lm/an4/ \
	    -insentfn $(top_srcdir)/model/lm/an4/align.correct \
	    -outsentfn $@.out \
	    -wdsegdir ./ \
	    -phsegdir ./


test-allphone:
	@echo "ALLPHONE TEST"
	@echo "THIS IS JUST A FUNCTION TEST"
	$(LIBTOOL) --mode=execute src/programs/allphone \
	    -logbase 1.0003 \
	    -mdeffn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -meanfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
            -varfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
            -mixwfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
            -tmatfn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices \
	    -feat 1s_c_d_dd \
	    -topn 1000 \
	    -beam 1e-80 \
	    -senmgaufn .cont. \
	    -ctlfn $(top_srcdir)/model/lm/an4/an4.ctl \
	    -cepdir $(top_srcdir)/model/lm/an4/ \
	    -phsegdir ./


test-astar:
	@echo "ASTAR TEST"
	$(LIBTOOL) --mode=execute src/programs/astar \
	    -mdeffn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -fdictfn $(top_srcdir)/model/lm/an4/filler.dict \
            -dictfn $(top_srcdir)/model/lm/an4/an4.dict \
            -lmfn $(top_srcdir)/model/lm/an4/an4.ug.lm.DMP \
	    -inspen 0.2 \
	    -beam 1e-64 \
	    -nbest 5 \
	    -ctlfn $(top_srcdir)/model/lm/an4/an4.ctl.platform_independent \
	    -inlatdir $(top_srcdir)/model/lm/an4/ \
	    -logbase 1.0003 \
	    -nbestdir . 
	@gzip -cfd pittsburgh.nbest.gz 

test-dag:
	@echo "DAG TEST"
	$(LIBTOOL) --mode=execute src/programs/dag \
	    -mdeffn $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -fdictfn $(top_srcdir)/model/lm/an4/filler.dict \
            -dictfn $(top_srcdir)/model/lm/an4/an4.dict \
            -lmfn $(top_srcdir)/model/lm/an4/an4.ug.lm.DMP \
	    -langwt 13.0 \
	    -inspen 0.2 \
	    -ctlfn $(top_srcdir)/model/lm/an4/an4.ctl.platform_independent \
	    -inlatdir $(top_srcdir)/model/lm/an4/ \
	    -logbase 1.0003 \
	    -backtrace 1 > $@.out 

	@grep "BSTPTH: P I T T S B U R G H " $@.out 
	@grep "BSTXCT" $@.out 

test-gausubvq:
	@echo "GAUSUBVQ TEST"
#	@echo "This will compare the answer with a pre-generated svq file"
	$(LIBTOOL) --mode=execute src/programs/gausubvq \
        -mean $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
        -var  $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
        -mixw  $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
        -svspec 0-38 \
        -iter 20 \
        -svqrows 16 \
        -subvq $@.out 
#	grep -v "#" subvq.out > tmp.out
#	diff tmp.out $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/test.subvq
#	rm tmp.out subvq.out




