ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libsearch.la

libsearch_la_LDFLAGS = -version-info 0:4:0

libsearch_la_SOURCES = \
	ascr.c \
	dag.c \
	flat_fwd.c \
	gmm_wrap.c \
	kb.c \
	kbcore.c \
	lextree.c \
	vithist.c \
	srch.c \
	srch_align.c \
	srch_allphone.c \
	srch_debug.c \
	srch_fsm.c \
	srch_time_switch_tree.c \
	srch_word_switch_tree.c 

pkginclude_HEADERS = \
	ascr.h \
	dag.h \
	flat_fwd.h \
	gmm_wrap.h \
	kb.h \
	kbcore.h \
	lextree.h \
	vithist.h \
	hyp.h \
	srch.h \
	search.h \
	srch_debug.h \
	srch_time_switch_tree.h \
	srch_word_switch_tree.h 

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/src/libs3decoder/libam/ \
	   -I$(top_srcdir)/src/libs3decoder/libcommon/ \
	   -I$(top_srcdir)/src/libs3decoder/libdict/ \
	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
	   -I$(top_srcdir)/src/libs3decoder/libsearch/ \
	   -I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

