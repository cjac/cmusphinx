INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 


CLEANFILES = gmake-rm1_flat_unigram$(MODEAFFIX).results \
             gmake-rm1_unigram$(MODEAFFIX).results \
             gmake-rm1_trigram$(MODEAFFIX).results

rm1_flat_unigram:
	rm -f gmake-rm1_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-lm RM.unigram.arpa.DMP" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-lw 7" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-beam 1e-140" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-80" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	../batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_flat_unigram$(MODEAFFIX) > gmake-rm1_flat_unigram$(MODEAFFIX).results


rm1_unigram:
	rm -f gmake-rm1_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.unigram.arpa.DMP" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-lw 10" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-beam 1e-120" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-60" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_unigram$(MODEAFFIX)
	../batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_unigram$(MODEAFFIX) > gmake-rm1_unigram$(MODEAFFIX).results


rm1_bigram:
	rm -f gmake-rm1_bigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.bigram.arpa.DMP" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-lw 13" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-beam 1e-120" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-100" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_bigram$(MODEAFFIX)
	../batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_bigram$(MODEAFFIX) > gmake-rm1_bigram$(MODEAFFIX).results

rm1_trigram:
	rm -f gmake-rm1_trigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.trigram.arpa.DMP" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-lw 14" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-beam 1e-140" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-100" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_trigram$(MODEAFFIX)
	../batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_trigram$(MODEAFFIX) > gmake-rm1_trigram$(MODEAFFIX).results

