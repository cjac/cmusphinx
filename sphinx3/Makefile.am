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

test:
	$(LIBTOOL) --mode=execute echo "LIVEPRETEND TEST"
	$(LIBTOOL) --mode=execute echo "YOU SHOULD THE RECOGNITION ANSWER IS P I T T S B U R G H "

	$(LIBTOOL) --mode=execute src/programs/livepretend \
		$(top_srcdir)/model/lm/an4/an4.ctl \
		$(top_srcdir)/model/lm/an4 \
		$(top_srcdir)/model/lm/an4/args.an4.test

	$(LIBTOOL) --mode=execute echo "DECODE TEST"
	$(LIBTOOL) --mode=execute echo "YOU SHOULD THE RECOGNITION ANSWER IS P I T T S B U R G H "

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
               -ds 2 



