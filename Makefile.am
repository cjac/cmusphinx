SUBDIRS = src \
	doc \
	scripts \
	model

EXTRA_DIST = autogen.sh \
	programs.dsw \
	win32/batch/an4.ctl \
	win32/batch/sphinx3-simple.bat \
	win32/batch/sphinx3-test.align.bat \
	win32/batch/sphinx3-test.all.bat \
	win32/batch/sphinx3-test.allphone.bat \
	win32/batch/sphinx3-test.astar.bat \
	win32/batch/sphinx3-test.bat \
	win32/batch/sphinx3-test.dag.bat \
	win32/batch/sphinx3-test.decode.bat \
	win32/batch/sphinx3-test.ep.bat \
	win32/batch/sphinx3-test.gausubvq.bat \
	win32/batch/sphinx3-test.livepretend.bat \
	win32/batch/sphinx3-test.simple.bat \
	win32/msdev/libs3audio/libs3audio.dsp \
	win32/msdev/libs3decoder/libs3decoder.dsp \
	win32/msdev/libutil/libutil.dsp \
	win32/msdev/programs/align/align.dsp \
	win32/msdev/programs/allphone/allphone.dsp \
	win32/msdev/programs/astar/astar.dsp \
	win32/msdev/programs/dag/dag.dsp \
	win32/msdev/programs/decode/allphone.dsp \
	win32/msdev/programs/decode/decode.dsp \
	win32/msdev/programs/ep/ep.dsp \
	win32/msdev/programs/gausubvq/gausubvq.dsp \
	win32/msdev/programs/livedecode/livedecode.dsp \
	win32/msdev/programs/livepretend/livepretend.dsp

# Add an item for each and every test target to the CLEANFILES variable
CLEANFILES = chan3.mfc \
	test.out \
	test-*.out \
	pittsburgh.littleendian.allp \
	pittsburgh.littleendian.phseg \
	pittsburgh.littleendian.wdseg \
	pittsburgh.nbest.gz


etags:
	etags `find . -name "*.[ch]"`

ctags:
	ctags `find . -name "*.[ch]"`

doxygen: 
	doxygen ./doc/doxygen.cfg

#######################################################################
#Session of testing
#######################################################################
test-simple : test-livepretend

test: test-simple

test-full : test-decode test-align test-allphone test-livepretend test-astar test-dag test-gausubvq-match test-ep-match test-cepview 

test-all-app : test-decode test-align test-allphone test-livepretend test-astar test-dag test-gausubvq-match test-ep-match test-wave2feat test-cepview 


test-ep : test-ep-match
test-gausubvq : test-gausubvq-match

test-livepretend:
	$(top_builddir)/src/tests/regression/test_livepretend.sh

test-valgrind:
	@echo "LIVEPRETEND TEST"
	@echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"

	$(LIBTOOL) --mode=execute valgrind --skin=memcheck src/programs/livepretend \
		model/lm/an4/an4.ctl \
		$(top_srcdir)/model/lm/an4 \
		model/lm/an4/args.an4.test > $@.out 2>&1 
	@grep "FWDVIT" $@.out 
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
	       -ctl $(top_builddir)/model/lm/an4/an4.ctl \
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
	       -ci_pbeam 1e-5 \
	       -ds 2  > $@.out 2>&1

	@grep "FWDVIT" $@.out 
	@grep "FWDXCT" $@.out 

test-decode_anytopo:
	@echo "DECODE_ANYTOPO TEST"
	@echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"
	$(LIBTOOL) --mode=execute src/programs/decode_anytopo \
	       -mdef $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
	       -fdict $(top_srcdir)/model/lm/an4/filler.dict \
	       -dict $(top_srcdir)/model/lm/an4/an4.dict \
	       -mean $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
	       -var $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
	       -mixw $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
	       -tmat $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices \
	       -lm $(top_srcdir)/model/lm/an4/an4.ug.lm.DMP \
	       -ctl $(top_builddir)/model/lm/an4/an4.ctl \
	       -cepdir $(top_srcdir)/model/lm/an4/ \
	       -agc none \
	       -varnorm no \
	       -cmn current \
	       -feat 1s_c_d_dd \
	       -lw 9.5 \
	       -inspen 0.2 \
               -beam 1e-80 \
               -nwbeam 1e-40 \
		 > $@.out 2>&1

	@grep "FWDVIT" $@.out 
	@grep "FWDXCT" $@.out 

test-align:
	echo "ALIGN TEST"
	echo "THIS IS JUST A FUNCTION TEST"
	$(LIBTOOL) --mode=execute src/programs/align \
	    -logbase 1.0003 \
	    -mdef $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -mean $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
            -var $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
            -mixw $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
            -tmat $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices \
	    -feat 1s_c_d_dd \
	    -topn 1000 \
	    -beam 1e-80 \
	    -senmgau .cont. \
            -fdict $(top_srcdir)/model/lm/an4/filler.dict \
            -dict $(top_srcdir)/model/lm/an4/an4.dict \
	    -ctl $(top_builddir)/model/lm/an4/an4.ctl \
	    -cepdir $(top_srcdir)/model/lm/an4/ \
	    -insent $(top_srcdir)/model/lm/an4/align.correct \
	    -outsent $@.out \
	    -wdsegdir ./ \
	    -phsegdir ./
	@if (diff test-align.out  $(top_srcdir)/model/hmm//hub4_cd_continuous_8gau_1s_c_d_dd/test.align.out > /dev/null 2>&1); \
	then echo "align output test PASSED"; else \
	echo "align output test FAILED"; fi 
	@if (diff `head -1 $(top_builddir)/model/lm/an4/an4.ctl`.wdseg $(top_srcdir)/model/hmm//hub4_cd_continuous_8gau_1s_c_d_dd/test.align.wdseg > /dev/null 2>&1); \
	then echo "ALIGN wdseg test PASSED"; else \
	echo "ALIGN wdseg test FAILED"; fi 
	@if (diff `head -1 $(top_builddir)/model/lm/an4/an4.ctl`.phseg $(top_srcdir)/model/hmm//hub4_cd_continuous_8gau_1s_c_d_dd/test.align.phseg > /dev/null 2>&1 ); \
	then echo "ALIGN phseg test PASSED"; else \
	echo "ALIGN phseg test FAILED"; fi 

test-allphone:
	@echo "ALLPHONE TEST"
	@echo "THIS IS JUST A FUNCTION TEST"
	$(LIBTOOL) --mode=execute src/programs/allphone \
	    -logbase 1.0003 \
	    -mdef $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -mean $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
            -var $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
            -mixw $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
            -tmat $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices \
	    -feat 1s_c_d_dd \
	    -topn 1000 \
	    -beam 1e-80 \
	    -senmgau .cont. \
	    -ctl $(top_builddir)/model/lm/an4/an4.ctl \
	    -cepdir $(top_srcdir)/model/lm/an4/ \
	    -phsegdir ./ \
	    -phlatdir ./
	@if(diff `head -1 $(top_builddir)/model/lm/an4/an4.ctl`.allp $(top_srcdir)/model/hmm//hub4_cd_continuous_8gau_1s_c_d_dd/test.allphone.allp > /dev/null 2>&1); \
	then echo "allphone allp test PASSED"; else \
	echo "allphone allp test FAILED"; fi
	@if(diff `head -1 $(top_builddir)/model/lm/an4/an4.ctl`.phlat $(top_srcdir)/model/hmm//hub4_cd_continuous_8gau_1s_c_d_dd/test.allphone.phlat > /dev/null 2>&1); \
	then echo "allphone phlat test PASSED"; else \
	echo "allphone phlat test FAILED"; fi


test-astar:
	@echo "ASTAR TEST"
	$(LIBTOOL) --mode=execute src/programs/astar \
	    -mdef $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -fdict $(top_srcdir)/model/lm/an4/filler.dict \
            -dict $(top_srcdir)/model/lm/an4/an4.dict \
            -lm $(top_srcdir)/model/lm/an4/an4.ug.lm.DMP \
	    -inspen 0.2 \
	    -beam 1e-64 \
	    -nbest 5 \
	    -ctl $(top_srcdir)/model/lm/an4/an4.ctl.platform_independent \
	    -inlatdir $(top_srcdir)/model/lm/an4/ \
	    -logbase 1.0003 \
	    -nbestdir . 
	@gzip -cfd pittsburgh.nbest.gz 

test-dag:
	@echo "DAG TEST"
	$(LIBTOOL) --mode=execute src/programs/dag \
	    -mdef $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef \
            -fdict $(top_srcdir)/model/lm/an4/filler.dict \
            -dict $(top_srcdir)/model/lm/an4/an4.dict \
            -lm $(top_srcdir)/model/lm/an4/an4.ug.lm.DMP \
	    -lw 13.0 \
	    -inspen 0.2 \
	    -ctl $(top_srcdir)/model/lm/an4/an4.ctl.platform_independent \
	    -inlatdir $(top_srcdir)/model/lm/an4/ \
	    -logbase 1.0003 \
	    -backtrace 1 > $@.out 

	@grep "BSTPTH:" $@.out 
	@grep "BSTXCT" $@.out 

test-gausubvq-match:
	@echo "GAUSUBVQ TEST"
	@echo "This will compare the answer with a pre-generated svq file"
	$(LIBTOOL) --mode=execute src/programs/gausubvq \
        -mean $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means \
        -var  $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances \
        -mixw  $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights \
        -svspec 0-38 \
        -iter 20 \
        -svqrows 16 \
	-seed 1111 \
        -subvq $@.out 
	grep -v "#" $@.out > tmp.out
	@if(diff tmp.out $(top_srcdir)/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/test.subvq > /dev/null 2>&1); \
	then echo "gausubvq test PASSED"; else \
	echo "gausubvq test FAILED"; fi 


test-ep-match:
	@echo "EP TEST"
	$(LIBTOOL) --mode=execute src/programs/ep \
		-i $(top_srcdir)/model/ep/chan3.raw  \
	        -srate 11025 \
	        -frate 105 \
	        -wlen 0.024 \
	        -alpha 0.97 \
	        -ncep 13 \
	        -nfft 512 \
	        -nfilt 36 \
	        -upperf 5400 \
	        -lowerf 130 \
	        -blocksize 262500 \
	        -mean $(top_srcdir)/model/ep/means \
	        -var $(top_srcdir)/model/ep/variances \
	        -mixw $(top_srcdir)/model/ep/mixture_weights \
	        -o $@.tmp \
		-raw 1 \
		2>&1 |grep "Utt_" |perl -pe 's/(\d+\.\d+)/sprintf "%.3f", $$1/ge' > $@.out	
	@if (perl -pe 's/(\d+\.\d+)/sprintf "%.3f", $$1/ge' \
			 $(top_srcdir)/model/ep/ep.result | diff $@.out - > /dev/null 2>&1); \
	then echo "EP test PASSED"; else \
	echo "EP test FAILED"; fi 
#	rm $@.out		


test-wave2feat:
	@echo "WAVE2FEAT TEST"
	$(LIBTOOL) --mode=execute src/programs/wave2feat \
		 -srate 11025 \
		 -frate 105 \
		 -wlen 0.024 \
		 -alpha 0.97 \
		 -ncep 13 \
		 -nfft 512 \
		 -nfilt 36 \
		 -upperf 5400 \
		 -lowerf 130 \
		 -blocksize 262500 \
		 -i $(top_srcdir)/model/ep/chan3.raw \
	         -o $@.mfc  \
		 -raw 1
	@if(cmp $@.mfc $(top_srcdir)/model/ep/chan3.mfc > /dev/null 2>&1) ; \
	then echo "wave2feat test PASSED"; else \
	echo "wave2feat test FAILED"; fi

test-cepview:
	@echo "CEPVIEW TEST"
	$(LIBTOOL) --mode=execute src/programs/cepview \
	-i 13 \
	-d 13 \
	-f $(top_srcdir)/model/ep/chan3.mfc \
	> $@.out
	@if (diff $@.out $(top_srcdir)/model/ep/chan3.cepview > /dev/null 2>&1); \
	then echo "Cepview test PASSED"; else \
	echo "Cepview test FAILED"; fi 
