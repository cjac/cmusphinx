SUBDIRS = libam libcep_feat libconfidence libcommon  libdict libep liblm libsearch libAPI

lib_LTLIBRARIES = libs3decoder.la

#lib_LTLIBRARIES = libam.la
#lib_LTLIBRARIES = libs3decoder.la

libs3decoder_la_LDFLAGS = -version-info 0:6:0
libs3decoder_la_SOURCES = 


libs3decoder_la_LIBADD = \
	libam/libam.la \
	libcep_feat/libcep_feat.la \
	libconfidence/libconfidence.la \
	libcommon/libcommon.la \
	libdict/libdict.la \
	libep/libep.la \
	liblm/liblm.la \
	libsearch/libsearch.la \
	libAPI/libAPI.la 

#lib_LTLIBRARIES = libs3decoder.la
#
#libs3decoder_la_LDFLAGS = -version-info 0:4:0
#
#libs3decoder_la_SOURCES = \
#	libam/adaptor.c \
#	libam/approx_cont_mgau.c \
#	libam/cont_mgau.c \
#	libam/fast_algo_struct.c \
#	libam/gs.c \
#	libam/hmm.c \
#	libam/interp.c \
#	libam/mdef.c \
#	libam/mllr.c \
#	libam/ms_gauden.c \
#	libam/ms_mllr.c \
#	libam/ms_senone.c \
#	libam/subvq.c \
#	libam/tmat.c \
#	libcep_feat/agc.c \
#	libcep_feat/cmn.c \
#	libcep_feat/cmn_prior.c \
#	libcep_feat/feat.c \
#	libcep_feat/fe.c \
#	libcep_feat/fe_interface.c \
#	libcep_feat/fe_sigproc.c \
#	libcommon/bio.c \
#	libcommon/corpus.c \
#	libcommon/logs3.c \
#	libcommon/misc.c \
#	libcommon/stat.c \
#	libcommon/vector.c  \
#	libdict/dict2pid.c \
#	libdict/dict.c \
#	libdict/wid.c \
#	libep/classify.c \
#	libep/endptr.c \
#	liblm/fillpen.c \
#        liblm/lm.c \
#	liblm/lmclass.c \
#	libsearch/ascr.c \
#	libsearch/dag.c \
#	libsearch/flat_fwd.c \
#	libsearch/gmm_wrap.c \
#	libsearch/kb.c \
#	libsearch/kbcore.c \
#	libsearch/lextree.c \
#	libsearch/vithist.c \
#	libsearch/srch.c \
#	libsearch/srch_align.c \
#	libsearch/srch_allphone.c \
#	libsearch/srch_debug.c \
#	libsearch/srch_fsm.c \
#	libsearch/srch_time_switch_tree.c \
#	libsearch/srch_word_switch_tree.c \
#	libAPI/live_decode_API.c \
#	libAPI/live_decode_args.c \
#	libAPI/utt.c 
#
#noinst_HEADERS = \
#	libam/adaptor.h \
#	libam/approx_cont_mgau.h \
#	libam/cont_mgau.h \
#	libam/fast_algo_struct.h \
#	libam/gs.h \
#	libam/hmm.h \
#	libam/interp.h \
#	libam/mdef.h \
#	libam/mllr.h \
#	libam/ms_gauden.h \
#	libam/ms_mllr.h \
#	libam/ms_senone.h \
#	libam/subvq.h \
#	libam/tmat.h \
#	libcep_feat/agc.h \
#	libcep_feat/cmn.h \
#	libcep_feat/cmn_prior.h \
#	libcep_feat/feat.h \
#	libcep_feat/fe.h \
#	libcep_feat/fe_internal.h \
#	libcommon/bio.h \
#	libcommon/corpus.h \
#	libcommon/logs3.h \
#	libcommon/misc.h \
#	libcommon/stat.h \
#	libcommon/vector.h  \
#	libdict/dict2pid.h \
#	libdict/dict.h \
#	libdict/wid.h \
#	libep/classify.h \
#	libep/endptr.h \
#	liblm/fillpen.h \
#        liblm/lm.h \
#	liblm/lmclass.h \
#	libsearch/ascr.h \
#	libsearch/dag.h \
#	libsearch/flat_fwd.h \
#	libsearch/gmm_wrap.h \
#	libsearch/kb.h \
#	libsearch/kbcore.h \
#	libsearch/lextree.h \
#	libsearch/vithist.h \
#	libsearch/hyp.h \
#	libsearch/srch.h \
#	libsearch/search.h \
#	libsearch/srch_debug.h \
#	libsearch/srch_time_switch_tree.h \
#	libsearch/srch_word_switch_tree.h \
#	libAPI/live_decode_API.h \
#	libAPI/live_decode_args.h \
#	libAPI/utt.h
#
#INCLUDES = -I$(top_srcdir) \
#	   -I$(top_srcdir)/include \
#	   -I$(top_srcdir)/src/libutil/ \
#	   -I$(top_srcdir)/src/libs3decoder/libam/ \
#	   -I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
#	   -I$(top_srcdir)/src/libs3decoder/libcommon/ \
#	   -I$(top_srcdir)/src/libs3decoder/libdict/ \
#	   -I$(top_srcdir)/src/libs3decoder/libep/ \
#	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
#	   -I$(top_srcdir)/src/libs3decoder/libsearch/ \
#	   -I$(top_srcdir)/src/libs3decoder/libAPI/
#
#LIBOBJS = @LIBOBJS@
