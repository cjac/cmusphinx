ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libsearch.la

libsearch_la_LDFLAGS = -version-info 0:6:0

libsearch_la_SOURCES = \
	ascr.c \
	blkarray_list.c \
	dag.c \
	flat_fwd.c \
	fsg_history.c \
	fsg_lextree.c \
	fsg_psubtree.c \
	fsg_search.c \
	gmm_wrap.c \
	kb.c \
	kbcore.c \
	lextree.c \
	srch.c \
	srch_align.c \
	srch_allphone.c \
	srch_debug.c \
	srch_flat_fwd.c \
	srch_fsg.c \
	srch_output.c \
	srch_time_switch_tree.c \
	srch_word_switch_tree.c \
	vithist.c \
	whmm.c \
	word_fsg.c \
	word_graph.c \
	word_ugprob.c


pkginclude_HEADERS = \
	ascr.h \
	blkarray_list.h \
	dag.h \
	flat_fwd.h \
	fsg.h \
	fsg_history.h \
	fsg_lextree.h \
	fsg_psubtree.h \
	fsg_search.h \
	gmm_wrap.h \
	hyp.h \
	kb.h \
	kbcore.h \
	lextree.h \
	search.h \
	srch.h \
	srch_debug.h \
	srch_flat_fwd.h \
	srch_fsg.h \
	srch_output.h \
	srch_time_switch_tree.h \
	srch_word_switch_tree.h \
	vithist.h \
	whmm.h \
	word_fsg.h \
	word_graph.h \
	word_ugprob.h

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

