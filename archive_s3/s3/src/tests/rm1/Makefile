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
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.bigram.arpa.DMP" >> ARGS.rm1_bigram
	echo "-matchfn rm1_bigram.match" >> ARGS.rm1_bigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1_bigram > gmake-rm1_bigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1_bigram.match > rm1_bigram.align


rm1_trigram:
	rm -f gmake-rm1_trigram.results
	/bin/cp ARGS.rm1 ARGS.rm1_trigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.trigram.arpa.DMP" >> ARGS.rm1_trigram
	echo "-matchfn rm1_trigram.match" >> ARGS.rm1_trigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1_trigram > gmake-rm1_trigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1_trigram.match > rm1_trigram.align


rm1onrm1_unigram:
	rm -f gmake-rm1onrm1_unigram.results
	/bin/cp ARGS.rm1onrm1 ARGS.rm1onrm1_unigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.unigram.arpa.DMP" >> ARGS.rm1onrm1_unigram
	echo "-langwt 13" >> ARGS.rm1onrm1_unigram
	echo "-beam 1e-120" >> ARGS.rm1onrm1_unigram
	echo "-nwbeam 1e-60" >> ARGS.rm1onrm1_unigram
	echo "-matchfn rm1onrm1_unigram.match" >> ARGS.rm1onrm1_unigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1onrm1_unigram > gmake-rm1onrm1_unigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1onrm1_unigram.match > rm1onrm1_unigram.align

rm1onrm1_flatunigram:
	rm -f gmake-rm1onrm1_flatunigram.results
	/bin/cp ARGS.rm1onrm1 ARGS.rm1onrm1_flatunigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.unigram.arpa.DMP" >> ARGS.rm1onrm1_flatunigram
	echo "-langwt 8" >> ARGS.rm1onrm1_flatunigram
	echo "-beam 1e-120" >> ARGS.rm1onrm1_flatunigram
	echo "-nwbeam 1e-60" >> ARGS.rm1onrm1_flatunigram
	echo "-matchfn rm1onrm1_flatunigram.match" >> ARGS.rm1onrm1_flatunigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1onrm1_flatunigram > gmake-rm1onrm1_flatunigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1onrm1_flatunigram.match > rm1onrm1_flatunigram.align

rm1onrm1_bigram:
	rm -f gmake-rm1onrm1_bigram.results
	/bin/cp ARGS.rm1onrm1 ARGS.rm1onrm1_bigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.bigram.arpa.DMP" >> ARGS.rm1onrm1_bigram
	echo "-langwt 17" >> ARGS.rm1onrm1_bigram
	echo "-beam 1e-140" >> ARGS.rm1onrm1_bigram
	echo "-nwbeam 1e-100" >> ARGS.rm1onrm1_bigram
	echo "-matchfn rm1onrm1_bigram.match" >> ARGS.rm1onrm1_bigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1onrm1_bigram > gmake-rm1onrm1_bigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1onrm1_bigram.match > rm1onrm1_bigram.align

rm1onrm1_trigram:
	rm -f gmake-rm1onrm1_trigram.results
	/bin/cp ARGS.rm1onrm1 ARGS.rm1onrm1_trigram
	echo "-lmfn /lab/speech/sphinx4/data/rm1/lists/RM.2845.trigram.arpa.DMP" >> ARGS.rm1onrm1_trigram
	echo "-langwt 27" >> ARGS.rm1onrm1_trigram
	echo "-beam 1e-160" >> ARGS.rm1onrm1_trigram
	echo "-nwbeam 1e-120" >> ARGS.rm1onrm1_trigram
	echo "-matchfn rm1onrm1_trigram.match" >> ARGS.rm1onrm1_trigram
	$(BINDIR)/s3decode-anytopo ARGS.rm1onrm1_trigram > gmake-rm1onrm1_trigram.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1onrm1_trigram.match > rm1onrm1_trigram.align
