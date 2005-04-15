ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

lib_LTLIBRARIES = libs3decoder.la

libs3decoder_la_LDFLAGS = -version-info 0:4:0

libs3decoder_la_SOURCES = \
        agc.c \
	approx_cont_mgau.c \
	ascr.c \
        bio.c \
	cb2mllr_io.c \
	classify.c \
        cmn.c \
	cmn_prior.c \
        cont_mgau.c \
        corpus.c \
        dict.c \
        dict2pid.c \
	endptr.c \
	fast_algo_struct.c \
	fe_interface.c \
	fe_sigproc.c \
	fe.c \
        feat.c \
        fillpen.c \
	flat_fwd.c \
	gs.c \
        hmm.c \
	interp.c \
        kb.c \
        kbcore.c \
        lextree.c \
	live_decode_args.c \
	live_decode_API.c \
        lm.c \
	lmclass.c \
        logs3.c \
        mdef.c \
	misc.c \
	mllr.c \
	ms_gauden.c \
	ms_mllr.c \
	ms_senone.c \
        subvq.c \
        tmat.c \
        utt.c \
        vector.c \
        vithist.c \
        wid.c


noinst_HEADERS = \
        agc.h \
	approx_cont_mgau.h \
        ascr.h \
        bio.h \
	cb2mllr_io.h \
	classify.h \
        cmn.h \
	cmn_prior.h \
        cont_mgau.h \
        corpus.h \
        dict.h \
        dict2pid.h \
	endptr.h \
	fast_algo_struct.h \
	fe.h \
	fe_internal.h \
        feat.h \
        fillpen.h \
	flat_fwd.h \
	gs.h \
        hmm.h \
	interp.h \
        hyp.h \
        kb.h \
        kbcore.h \
        lextree.h \
	live_decode_args.h \
	live_decode_API.h \
        lm.h \
	lmclass.h \
        logs3.h \
        mdef.h \
	misc.h \
	mllr.h \
	ms_gauden.h \
	ms_mllr.h \
	ms_senone.h \
        s3types.h \
	search.h \
        subvq.h \
        tmat.h \
        utt.h \
        vector.h \
        vithist.h \
        wid.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

