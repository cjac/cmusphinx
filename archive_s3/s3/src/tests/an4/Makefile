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
else
  MODE = 
endif

an4_words_unigram:
	rm -f gmake-an4_words_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_words_unigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_words.unigram.lm.DMP" >> ARGS.an4_words_unigram$(MODEAFFIX)
	echo "-ctlfn    an4_words$(MODEAFFIX).ctl" >> ARGS.an4_words_unigram$(MODEAFFIX)
	echo "-matchfn    an4_words_unigram$(MODEAFFIX).match" >> ARGS.an4_words_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_words_unigram$(MODEAFFIX) > gmake-an4_words_unigram$(MODEAFFIX).resultss
	$(BINDIR)/align -ref an4_words$(MODEAFFIX).ref -hyp an4_words_unigram$(MODEAFFIX).match > an4_words_unigram$(MODEAFFIX).align

an4_spelling_unigram:
	rm -f gmake-an4_spelling_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_spelling_unigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling.unigram.lm.DMP" >> ARGS.an4_spelling_unigram$(MODEAFFIX)
	echo "-ctlfn    an4_spelling$(MODEAFFIX).ctl" >> ARGS.an4_spelling_unigram$(MODEAFFIX)
	echo "-matchfn    an4_spelling_unigram$(MODEAFFIX).match" >> ARGS.an4_spelling_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_unigram$(MODEAFFIX) > gmake-an4_spelling_unigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_spelling$(MODEAFFIX).ref -hyp an4_spelling_unigram$(MODEAFFIX).match > an4_spelling_unigram$(MODEAFFIX).align

an4_full_unigram:
	rm -f gmake-an4_full_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_full_unigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_full.unigram.lm.DMP" >> ARGS.an4_full_unigram$(MODEAFFIX)
	echo "-ctlfn    an4_full$(MODEAFFIX).ctl" >> ARGS.an4_full_unigram$(MODEAFFIX)
	echo "-matchfn    an4_full_unigram$(MODEAFFIX).match" >> ARGS.an4_full_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_full_unigram$(MODEAFFIX) > gmake-an4_full_unigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_full$(MODEAFFIX).ref -hyp an4_full_unigram$(MODEAFFIX).match > an4_full_unigram$(MODEAFFIX).align

an4_words_flat_unigram:
	rm -f gmake-an4_words_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_words_flat_unigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_words.flat_unigram.lm.DMP" >> ARGS.an4_words_flat_unigram$(MODEAFFIX)
	echo "-ctlfn    an4_words$(MODEAFFIX).ctl" >> ARGS.an4_words_flat_unigram$(MODEAFFIX)
	echo "-matchfn    an4_words_flat_unigram$(MODEAFFIX).match" >> ARGS.an4_words_flat_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_words_flat_unigram$(MODEAFFIX) > gmake-an4_words_flat_unigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_words$(MODEAFFIX).ref -hyp an4_words_flat_unigram$(MODEAFFIX).match > an4_words_flat_unigram$(MODEAFFIX).align

an4_spelling_flat_unigram:
	rm -f gmake-an4_spelling_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling.flat_unigram.lm.DMP" >> ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	echo "-ctlfn    an4_spelling$(MODEAFFIX).ctl" >> ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	echo "-matchfn    an4_spelling_flat_unigram$(MODEAFFIX).match" >> ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_flat_unigram$(MODEAFFIX) > gmake-an4_spelling_flat_unigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_spelling$(MODEAFFIX).ref -hyp an4_spelling_flat_unigram$(MODEAFFIX).match > an4_spelling_flat_unigram$(MODEAFFIX).align

an4_full_flat_unigram:
	rm -f gmake-an4_full_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_full_flat_unigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_full.flat_unigram.lm.DMP" >> ARGS.an4_full_flat_unigram$(MODEAFFIX)
	echo "-ctlfn    an4_full$(MODEAFFIX).ctl" >> ARGS.an4_full_flat_unigram$(MODEAFFIX)
	echo "-matchfn    an4_full_flat_unigram$(MODEAFFIX).match" >> ARGS.an4_full_flat_unigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_full_flat_unigram$(MODEAFFIX) > gmake-an4_full_flat_unigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_full$(MODEAFFIX).ref -hyp an4_full_flat_unigram$(MODEAFFIX).match > an4_full_flat_unigram$(MODEAFFIX).align

an4_words_bigram:
	rm -f gmake-an4_words_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_words_bigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_words.bigram.lm.DMP" >> ARGS.an4_words_bigram$(MODEAFFIX)
	echo "-ctlfn    an4_words$(MODEAFFIX).ctl" >> ARGS.an4_words_bigram$(MODEAFFIX)
	echo "-matchfn    an4_words_bigram$(MODEAFFIX).match" >> ARGS.an4_words_bigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_words_bigram$(MODEAFFIX) > gmake-an4_words_bigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_words$(MODEAFFIX).ref -hyp an4_words_bigram$(MODEAFFIX).match > an4_words_bigram$(MODEAFFIX).align

an4_spelling_bigram:
	rm -f gmake-an4_spelling_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_spelling_bigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling.bigram.lm.DMP" >> ARGS.an4_spelling_bigram$(MODEAFFIX)
	echo "-ctlfn    an4_spelling$(MODEAFFIX).ctl" >> ARGS.an4_spelling_bigram$(MODEAFFIX)
	echo "-matchfn    an4_spelling_bigram$(MODEAFFIX).match" >> ARGS.an4_spelling_bigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_bigram$(MODEAFFIX) > gmake-an4_spelling_bigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_spelling$(MODEAFFIX).ref -hyp an4_spelling_bigram$(MODEAFFIX).match > an4_spelling_bigram$(MODEAFFIX).align

an4_full_bigram:
	rm -f gmake-an4_full_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_full_bigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_full.bigram.lm.DMP" >> ARGS.an4_full_bigram$(MODEAFFIX)
	echo "-ctlfn    an4_full$(MODEAFFIX).ctl" >> ARGS.an4_full_bigram$(MODEAFFIX)
	echo "-matchfn    an4_full_bigram$(MODEAFFIX).match" >> ARGS.an4_full_bigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_full_bigram$(MODEAFFIX) > gmake-an4_full_bigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_full$(MODEAFFIX).ref -hyp an4_full_bigram$(MODEAFFIX).match > an4_full_bigram$(MODEAFFIX).align

an4_words_trigram:
	rm -f gmake-an4_words_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_words_trigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_words.trigram.lm.DMP" >> ARGS.an4_words_trigram$(MODEAFFIX)
	echo "-ctlfn    an4_words$(MODEAFFIX).ctl" >> ARGS.an4_words_trigram$(MODEAFFIX)
	echo "-matchfn    an4_words_trigram$(MODEAFFIX).match" >> ARGS.an4_words_trigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_words_trigram$(MODEAFFIX) > gmake-an4_words_trigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_words$(MODEAFFIX).ref -hyp an4_words_trigram$(MODEAFFIX).match > an4_words_trigram$(MODEAFFIX).align

an4_spelling_trigram:
	rm -f gmake-an4_spelling_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_spelling_trigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_spelling.trigram.lm.DMP" >> ARGS.an4_spelling_trigram$(MODEAFFIX)
	echo "-ctlfn    an4_spelling$(MODEAFFIX).ctl" >> ARGS.an4_spelling_trigram$(MODEAFFIX)
	echo "-matchfn    an4_spelling_trigram$(MODEAFFIX).match" >> ARGS.an4_spelling_trigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_spelling_trigram$(MODEAFFIX) > gmake-an4_spelling_trigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_spelling$(MODEAFFIX).ref -hyp an4_spelling_trigram$(MODEAFFIX).match > an4_spelling_trigram$(MODEAFFIX).align

an4_full_trigram:
	rm -f gmake-an4_full_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_full_trigram$(MODEAFFIX)
	echo "-lmfn   /lab/speech/sphinx4/data/an4/lists/an4_full.trigram.lm.DMP" >> ARGS.an4_full_trigram$(MODEAFFIX)
	echo "-ctlfn    an4_full$(MODEAFFIX).ctl" >> ARGS.an4_full_trigram$(MODEAFFIX)
	echo "-matchfn    an4_full_trigram$(MODEAFFIX).match" >> ARGS.an4_full_trigram$(MODEAFFIX)
	$(BINDIR)/s3decode-anytopo ARGS.an4_full_trigram$(MODEAFFIX) > gmake-an4_full_trigram$(MODEAFFIX).results
	$(BINDIR)/align -ref an4_full$(MODEAFFIX).ref -hyp an4_full_trigram$(MODEAFFIX).match > an4_full_trigram$(MODEAFFIX).align
