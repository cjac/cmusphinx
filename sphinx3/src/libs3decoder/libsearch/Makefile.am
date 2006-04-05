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

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

