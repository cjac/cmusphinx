# ====================================================================
#
#  Sphinx III
#
# ====================================================================
TOP=../..
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

ti46:
	cd ti46
	rm -f gmake-ti46.results
	$(BINDIR)/s3decode-anytopo ARGS.ti46 > gmake-ti46.results
	$(BINDIR)/align -ref ti46.ref -hyp ti46.match > ti46.align

tidigits:
	cd tidigits
	rm -f gmake-tidigits.results
	$(BINDIR)/s3decode-anytopo ARGS.tidigits > gmake-tidigits.results
	$(BINDIR)/align -ref tidigits.ref -hyp tidigits.match > tidigits.align

an4_words:
	cd an4
	rm -f gmake-an4_words.results
	/bin/cp ARGS.an4_base ARGS.an4_words_unigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_words.ug.lm.DMP" >> ARGS.an4_words_unigram
	echo "-ctlfn    an4_words.ctl" >> ARGS.an4_words_unigram
	echo "-matchfn    an4_words.match" >> ARGS.an4_words_unigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_words_unigram > gmake-an4_words.results
	$(BINDIR)/align -ref an4_words.ref -hyp an4_words.match > an4_words.align

an4_spelling:
	cd an4
	rm -f gmake-an4_spelling.results
	/bin/cp ARGS.an4_base ARGS.an4_spelling_unigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling.ug.lm.DMP" >> ARGS.an4_spelling_unigram
	echo "-ctlfn    an4_spelling.ctl" >> ARGS.an4_spelling_unigram
	echo "-matchfn    an4_spelling.match" >> ARGS.an4_spelling_unigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_unigram > gmake-an4_spelling.results
	$(BINDIR)/align -ref an4_spelling.ref -hyp an4_spelling.match > an4_spelling.align

an4_full:
	cd an4
	rm -f gmake-an4_full.results
	/bin/cp ARGS.an4_base ARGS.an4_full_unigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4.ug.lm.DMP" >> ARGS.an4_full_unigram
	echo "-ctlfn    an4_full.ctl" >> ARGS.an4_full_unigram
	echo "-matchfn    an4_full.match" >> ARGS.an4_full_unigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_full_unigram > gmake-an4_full.results
	$(BINDIR)/align -ref an4_full.ref -hyp an4_full.match > an4_full.align

an4_plus:
	cd an4
	rm -f gmake-an4_plus.results
	/bin/cp ARGS.an4_base ARGS.an4_plus_unigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_plus.ug.lm.DMP" >> ARGS.an4_plus_unigram
	echo "-ctlfn    an4_full.ctl" >> ARGS.an4_plus_unigram
	echo "-matchfn    an4_plus.match" >> ARGS.an4_plus_unigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_plus_unigram > gmake-an4_plus.results
	$(BINDIR)/align -ref an4_plus.ref -hyp an4_plus.match > an4_plus.align

an4_words_trigram:
	cd an4
	rm -f gmake-an4_words_trigrams.results
	/bin/cp ARGS.an4_base ARGS.an4_words_trigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_words.trigram.DMP" >> ARGS.an4_words_trigram
	echo "-ctlfn    an4_words.ctl" >> ARGS.an4_words_trigram
	echo "-matchfn    an4_words_trigram.match" >> ARGS.an4_words_trigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_words_trigram > gmake-an4_words_trigram.result
	$(BINDIR)/align -ref an4_words.ref -hyp an4_words_trigram.match > an4_words_trigram.align

an4_spelling_trigram:
	cd an4
	rm -f gmake-an4_spelling_trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_spelling_trigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling.trigram.DMP" >> ARGS.an4_spelling_trigram
	echo "-ctlfn    an4_spelling.ctl" >> ARGS.an4_spelling_trigram
	echo "-matchfn    an4_spelling_trigram.match" >> ARGS.an4_spelling_trigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_trigram > gmake-an4_spelling_trigram.results
	$(BINDIR)/align -ref an4_spelling.ref -hyp an4_spelling_trigram.match > an4_spelling_trigram.align

an4_spelling_journey_trigram:
	cd an4
	rm -f gmake-an4_spelling_journey_trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_spelling_journey_trigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling_journey.trigram.DMP" >> ARGS.an4_spelling_journey_trigram
	echo "-ctlfn    an4_spelling.ctl" >> ARGS.an4_spelling_journey_trigram
	echo "-matchfn    an4_spelling_trigram.match" >> ARGS.an4_spelling_journey_trigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_journey_trigram > gmake-an4_spelling_journey_trigram.results
	$(BINDIR)/align -ref an4_spelling.ref -hyp an4_spelling_journey_trigram.match > an4_spelling_journey_trigram.align

an4_full_trigram:
	cd an4
	rm -f gmake-an4_full_trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_full_trigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_full.trigram.DMP" >> ARGS.an4_full_trigram
	echo "-ctlfn    an4_full.ctl" >> ARGS.an4_full_trigram
	echo "-matchfn    an4_full_trigram.match" >> ARGS.an4_full_trigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_full_trigram > gmake-an4_full_trigram.results
	$(BINDIR)/align -ref an4_full.ref -hyp an4_full_trigram.match > an4_full_trigram.align

an4_plus_trigram:
	cd an4
	rm -f gmake-an4_plus_trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_plus_trigram
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_plus.trigram.DMP" >> ARGS.an4_plus_trigram
	echo "-ctlfn    an4_full.ctl" >> ARGS.an4_plus_trigram
	echo "-matchfn    an4_full_trigram.match" >> ARGS.an4_plus_trigram
	$(BINDIR)/s3decode-anytopo ARGS.an4_plus_trigram > gmake-an4_plus_trigram.results
	$(BINDIR)/align -ref an4_full.ref -hyp an4_plus_trigram.match > an4_plus_trigram.align

rm1:
	cd rm1
	rm -f gmake-rm1.results
	$(BINDIR)/s3decode-anytopo ARGS.rm1 > gmake-rm1.results
	$(BINDIR)/align -ref rm1.ref -hyp rm1.match > rm1.align
