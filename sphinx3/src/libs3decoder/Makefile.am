ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

lib_LTLIBRARIES = libs3decoder.la

libs3decoder_la_LDFLAGS = -version-info 0:1:0

libs3decoder_la_SOURCES = \
        agc.c \
        ascr.c \
        beam.c \
        bio.c \
        cmn.c \
        cmn_prior.c \
        cont_mgau.c \
        corpus.c \
        dict.c \
        dict2pid.c \
        feat.c \
        fillpen.c \
        hmm.c \
        kb.c \
        kbcore.c \
        lextree.c \
        lm.c \
        logs3.c \
        mdef.c \
        new_fe.c \
        new_fe_sp.c \
        subvq.c \
        tmat.c \
        utt.c \
        vector.c \
        vithist.c \
        wid.c


noinst_HEADERS = \
        agc.h \
        ascr.h \
        beam.h \
        bio.h \
        cmn.h \
        cmn_prior.h \
        cont_mgau.h \
        corpus.h \
        dict.h \
        dict2pid.h \
        feat.h \
        fillpen.h \
        hmm.h \
        hyp.h \
        kb.h \
        kbcore.h \
        lextree.h \
        lm.h \
        logs3.h \
        mdef.h \
        new_fe.h \
        new_fe_sp.h \
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
