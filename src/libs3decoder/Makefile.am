ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

lib_LTLIBRARIES = libs3decoder.la

libs3decoder_la_LDFLAGS = -version-info 0:4:0

libs3decoder_la_SOURCES = \
        agc.c \
	approx_cont_mgau.c \
	args.c \
	ascr.c \
        beam.c \
        bio.c \
        cmn.c \
        cmn_prior.c \
        cont_mgau.c \
        corpus.c \
        dict.c \
        dict2pid.c \
	fe_interface.c \
	fe_sigproc.c \
        feat.c \
        fillpen.c \
	gs.c \
        hmm.c \
        kb.c \
        kbcore.c \
        lextree.c \
	live_decode.c \
        lm.c \
	lmclass.c \
        logs3.c \
        mdef.c \
	mllr.c \
        subvq.c \
        tmat.c \
        utt.c \
        vector.c \
        vithist.c \
        wid.c


noinst_HEADERS = \
        agc.h \
	approx_cont_mgau.h \
	args.h \
        ascr.h \
        beam.h \
        bio.h \
        cmn.h \
        cmn_prior.h \
        cont_mgau.h \
        corpus.h \
        dict.h \
        dict2pid.h \
	fe.h \
	fe_internal.h \
        feat.h \
        fillpen.h \
	gs.h \
        hmm.h \
        hyp.h \
        kb.h \
        kbcore.h \
        lextree.h \
	live_decode.h \
        lm.h \
	lmclass.h \
        logs3.h \
        mdef.h \
	mllr.h \
        s3types.h \
        subvq.h \
        tmat.h \
        utt.h \
        vector.h \
        vithist.h \
        wid.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@
