LIVEDECSRCS = parse_args_file.c \
		fe_dump.c \
		feat_dump.c \
		fe_sigproc_dump.c \
		live_dump.c \
		metrics.c 

LIVEDECOBJS = $(LIVEDECSRCS:.c=.o) 


noinst_PROGRAMS = lm_test \
		  dumpfrontend \
		  analysis 

noinst_HEADERS = cmd_ln_args.h \
	fe_dump.h \
	fe_internal_dump.h \
	feat_dump.h \
	live_dump.h \
	gmm_compute.h \
	metrics.h

dumpfrontend_SOURCES = dump_frontend.c \
			${LIVEDECSRCS}

analysis_SOURCES = analysis.c \
		   gmm_compute.c 

lm_test_SOURCES = lm_test.c \
			${LIVEDECSRCS}

#lm_utt_test_SOURCES = lm_utt_test.c \
#			${LIVEDECSRCS}


INCLUDES = -I$(top_srcdir)/include \
       -I$(top_srcdir)/src/test/programs/ \
       -I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
       -I$(top_builddir)/include


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libs3util.la \
	-lm @ad_libs@ 
#	-lm -ldmalloc @ad_libs@ 



