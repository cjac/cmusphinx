# ====================================================================
#
#  Sphinx III
#
# ====================================================================
TOP=`(cd ../../..; pwd)`
DIRNAME=src/tests
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
# H = 
# LIVEDECSRCS = 
# MAINSRCS = 
# OTHERSRCS = main.c
# LIVEDECOBJS = $(LIVEDECSRCS:.c=.o) $(BASESRCS:.c=.o) 
FILES = Makefile
LIBNAME= tests
BINDIR = $(TOP)/bin/$(MACHINE)

ifeq ($(MODE),quick)
  MODEAFFIX = _quick
  CTLFILE = rm1_quick.ctl
  REFFILE = rm1_quick.ref
else
  MODE = 
  CTLFILE = rm1.ctl
  REFFILE = rm1.ref
endif

rm1_unigram:
	rm -f gmake-rm1_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_unigram$(MODEAFFIX)
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.unigram.arpa.DMP" >> ARGS.rm1_unigram$(MODEAFFIX)
	echo "-langwt 13" >> ARGS.rm1_unigram$(MODEAFFIX)
	echo "-beam 1e-120" >> ARGS.rm1_unigram$(MODEAFFIX)
	echo "-nwbeam 1e-60" >> ARGS.rm1_unigram$(MODEAFFIX)
	echo "-ctlfn $(CTLFILE)" >> ARGS.rm1_unigram$(MODEAFFIX)
	echo "-matchfn rm1_unigram$(MODEAFFIX).match" >> ARGS.rm1_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.rm1_unigram$(MODEAFFIX) > gmake-rm1_unigram$(MODEAFFIX).results
	$(BINDIR)/align -ref $(REFFILE) -hyp rm1_unigram$(MODEAFFIX).match > rm1_unigram$(MODEAFFIX).align

rm1_flatunigram:
	rm -f gmake-rm1_flatunigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_flatunigram$(MODEAFFIX)
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.unigram.arpa.DMP" >> ARGS.rm1_flatunigram$(MODEAFFIX)
	echo "-langwt 8" >> ARGS.rm1_flatunigram$(MODEAFFIX)
	echo "-beam 1e-120" >> ARGS.rm1_flatunigram$(MODEAFFIX)
	echo "-nwbeam 1e-60" >> ARGS.rm1_flatunigram$(MODEAFFIX)
	echo "-ctlfn $(CTLFILE)" >> ARGS.rm1_flatunigram$(MODEAFFIX)
	echo "-matchfn rm1_flatunigram$(MODEAFFIX).match" >> ARGS.rm1_flatunigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.rm1_flatunigram$(MODEAFFIX) > gmake-rm1_flatunigram$(MODEAFFIX).results
	$(BINDIR)/align -ref $(REFFILE) -hyp rm1_flatunigram$(MODEAFFIX).match > rm1_flatunigram$(MODEAFFIX).align

rm1_bigram:
	rm -f gmake-rm1_bigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_bigram$(MODEAFFIX)
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.bigram.arpa.DMP" >> ARGS.rm1_bigram$(MODEAFFIX)
	echo "-langwt 17" >> ARGS.rm1_bigram$(MODEAFFIX)
	echo "-beam 1e-140" >> ARGS.rm1_bigram$(MODEAFFIX)
	echo "-nwbeam 1e-100" >> ARGS.rm1_bigram$(MODEAFFIX)
	echo "-ctlfn $(CTLFILE)" >> ARGS.rm1_bigram$(MODEAFFIX)
	echo "-matchfn rm1_bigram$(MODEAFFIX).match" >> ARGS.rm1_bigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.rm1_bigram$(MODEAFFIX) > gmake-rm1_bigram$(MODEAFFIX).results
	$(BINDIR)/align -ref $(REFFILE) -hyp rm1_bigram$(MODEAFFIX).match > rm1_bigram$(MODEAFFIX).align

rm1_trigram:
	rm -f gmake-rm1_trigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_trigram$(MODEAFFIX)
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.trigram.arpa.DMP" >> ARGS.rm1_trigram$(MODEAFFIX)
	echo "-langwt 27" >> ARGS.rm1_trigram$(MODEAFFIX)
	echo "-beam 1e-160" >> ARGS.rm1_trigram$(MODEAFFIX)
	echo "-nwbeam 1e-120" >> ARGS.rm1_trigram$(MODEAFFIX)
	echo "-ctlfn $(CTLFILE)" >> ARGS.rm1_trigram$(MODEAFFIX)
	echo "-matchfn rm1_trigram$(MODEAFFIX).match" >> ARGS.rm1_trigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.rm1_trigram$(MODEAFFIX) > gmake-rm1_trigram$(MODEAFFIX).results
	$(BINDIR)/align -ref $(REFFILE) -hyp rm1_trigram$(MODEAFFIX).match > rm1_trigram$(MODEAFFIX).align
