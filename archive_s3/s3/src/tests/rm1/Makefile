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

rm1_unigram:
	rm -f gmake-rm1_unigram.results
	/bin/cp ARGS.rm1 ARGS.rm1_unigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.unigram.arpa.DMP" >> ARGS.rm1_unigram
	echo "-matchfn rm1_unigram.match" >> ARGS.rm1_unigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1_unigram > gmake-rm1_unigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1_unigram.match > rm1_unigram.align


rm1_bigram:
	rm -f gmake-rm1_bigram.results
	/bin/cp ARGS.rm1 ARGS.rm1_bigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2880.bigram.arpa.DMP" >> ARGS.rm1_bigram
	echo "-matchfn rm1_bigram.match" >> ARGS.rm1_bigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1_bigram > gmake-rm1_bigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1_bigram.match > rm1_bigram.align


rm1_trigram:
	rm -f gmake-rm1_trigram.results
	/bin/cp ARGS.rm1 ARGS.rm1_trigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2880.trigram.arpa.DMP" >> ARGS.rm1_trigram
	echo "-matchfn rm1_trigram.match" >> ARGS.rm1_trigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1_trigram > gmake-rm1_trigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1_trigram.match > rm1_trigram.align
