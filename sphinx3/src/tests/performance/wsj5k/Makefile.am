INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 


CLEANFILES = gmake-wsj5k_bigram$(MODEAFFIX).results \
	     gmake-wsj5k_bigram$(MODEAFFIX).nvp.results	 \
  	     gmake-wsj5k_trigram$(MODEAFFIX).results \
    	     gmake-wsj5k_trigram$(MODEAFFIX).nvp.results

wsj5k_bigram:
	rm -f gmake-wsj5k_bigram$(MODEAFFIX).results
	../batchmetrics /lab/speech/sphinx4/data/wsj0/wsj0/wsj5k$(MODEAFFIX).batch / ./ARGS.wsj5k_bigram > gmake-wsj5k_bigram$(MODEAFFIX).results

# bigram with no verbalized punctuation

wsj5k_bigram_nvp:
	rm -f gmake-wsj5k_bigram$(MODEAFFIX).results
	../batchmetrics /lab/speech/sphinx4/data/wsj0/wsj0/wsj5k.e$(MODEAFFIX).batch / ./ARGS.wsj5k_bigram > gmake-wsj5k_bigram$(MODEAFFIX).nvp.results

# trigram with verbalized punctuation
wsj5k_trigram:
	rm -f gmake-wsj5k_trigram$(MODEAFFIX).results
	../batchmetrics /lab/speech/sphinx4/data/wsj0/wsj0/wsj5k$(MODEAFFIX).batch / ./ARGS.wsj5k_trigram > gmake-wsj5k_trigram$(MODEAFFIX).results

# trigram with no verbalized punctuation

wsj5k_trigram_nvp:
	rm -f gmake-wsj5k_trigram$(MODEAFFIX).results
	../batchmetrics /lab/speech/sphinx4/data/wsj0/wsj0/wsj5k.e$(MODEAFFIX).batch / ./ARGS.wsj5k_trigram > gmake-wsj5k_trigram$(MODEAFFIX).nvp.results


