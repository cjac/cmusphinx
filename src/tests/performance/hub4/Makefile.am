INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 


CLEANFILES = gmake-hub4_flatunigram.results \
	     gmake-hub4_flatunigram.results \
	     gmake-hub4_trigram.results

HUB4_MODEL_DIR=/lab/speech/sphinx4/data/hub4_model
HUB4_DIR=/lab/speech/sphinx4/data/hub4/eval99/supervised

CONDITION = all

CTL_FILE=$(HUB4_DIR)/$(CONDITION)_hub4.batch

hub4_flatunigram:
	rm -f gmake-hub4_flatunigram.results
	/bin/cp ARGS.hub4_base ARGS.hub4_flatunigram
	echo "-lm ${HUB4_MODEL_DIR}/hub4.flat_unigram.lm.DMP" >> ARGS.hub4_flatunigram
	../batchmetrics $(CTL_FILE) / ./ARGS.hub4_flatunigram > gmake-hub4_flatunigram.results

hub4_trigram:
	rm -f gmake-hub4_trigram.results
	/bin/cp ARGS.hub4_base ARGS.hub4_trigram
	echo "-lm ${HUB4_MODEL_DIR}/language_model.arpaformat.DMP" >> ARGS.hub4_trigram
	../batchmetrics $(CTL_FILE) / ./ARGS.hub4_trigram > gmake-hub4_trigram.results

