INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 


CLEANFILES = gmake-wsj20k_bigram$(MODEAFFIX).results \
 	     gmake-wsj20k_trigram$(MODEAFFIX).results	


wsj20k_bigram:
	rm -f gmake-wsj20k_bigram$(MODEAFFIX).results
	../../programs/batchmetrics /lab/speech/sphinx4/data/wsj0/wsj0/wsj20k.e$(MODEAFFIX).batch / ./ARGS.wsj20k_bigram > gmake-wsj20k_bigram$(MODEAFFIX).results

wsj20k_trigram:
	rm -f gmake-wsj20k_trigram$(MODEAFFIX).results
	../../programs/batchmetrics /lab/speech/sphinx4/data/wsj0/wsj0/wsj20k.e$(MODEAFFIX).batch / ./ARGS.wsj20k_trigram > gmake-wsj20k_trigram$(MODEAFFIX).results

