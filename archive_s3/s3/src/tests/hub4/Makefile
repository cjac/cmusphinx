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

hub4_flatunigram:
	rm -f gmake-hub4_flatunigram.results
	/bin/cp ARGS.hub4 ARGS.hub4_flatunigram
	echo "-lmfn /lab/speech/sphinx4/data/hub4_model/hub4.flat_unigram.lm.DMP" >> ARGS.hub4_flatunigram
	echo "-matchfn hub4_flatunigram.match" >> ARGS.hub4_flatunigram
	$(BINDIR)/s3decode-anytopo ARGS.hub4_flatunigram > gmake-hub4_flatunigram.results
	$(BINDIR)/align -ref hub4.ref -hyp hub4_flatunigram.match > hub4_flatunigram.align


hub4_trigram:
	rm -f gmake-hub4_trigram.results
	/bin/cp ARGS.hub4 ARGS.hub4_trigram
	echo "-lmfn /lab/speech/sphinx4/data/hub4_model/language_model.arpaformat.DMP.Z" >> ARGS.hub4_trigram
	echo "-matchfn hub4_trigram.match" >> ARGS.hub4_trigram
	$(BINDIR)/s3decode-anytopo ARGS.hub4_trigram > gmake-hub4_trigram.results
	$(BINDIR)/align -ref hub4.ref -hyp hub4_trigram.match > hub4_trigram.align


