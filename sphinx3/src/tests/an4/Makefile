# ====================================================================
# Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================
#
#  Sphinx III
#
# ====================================================================
TOP=../../..
DIRNAME=src/tests/an4
BUILD_DIRS =
ALL_DIRS= $(BUILD_DIRS)
# H = cmd_ln_args.h fe_dump.h feat_dump.h new_fe_sp_dump.h live_dump.h metrics.h
# LIVEDECSRCS = parse_args_file.c fe_dump.c feat_dump.c new_fe_sp_dump.c live_dump.c metrics.c
MAINSRCS = dump_frontend.c batch_metrics.c
# OTHERSRCS = main.c
LIVEDECOBJS = $(LIVEDECSRCS:.c=.o) $(BASESRCS:.c=.o) 
FILES = Makefile $(SRCS) $(H)
LIBNAME= tests

LOCAL_INCLUDES = -I$(TOP)/src -I$(TOP)/src/libs3decoder -I$(TOP)/src/libs3audio

ALL = $(BINDIR)/batchmetrics

include $(TOP)/config/common_make_rules

ifeq ($(MODE),quick)
  MODEAFFIX = _quick
else
  MODEAFFIX = 
endif

$(BINDIR)/batchmetrics: ../batch_metrics.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ batch_metrics.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS) -Wl,-R$(LIBDIR)

an4_words_unigram:
	rm -f gmake-an4_words_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_words_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.unigram.lm.DMP" >> ARGS.an4_words_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_unigram$(MODEAFFIX) > gmake-an4_words_unigram$(MODEAFFIX).results

an4_spelling_unigram:
	rm -f gmake-an4_spelling_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_spelling_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.unigram.lm.DMP" >> ARGS.an4_spelling_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_unigram$(MODEAFFIX) > gmake-an4_spelling_unigram$(MODEAFFIX).results

an4_full_unigram:
	rm -f gmake-an4_full_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_full_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.unigram.lm.DMP" >> ARGS.an4_full_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_unigram$(MODEAFFIX) > gmake-an4_full_unigram$(MODEAFFIX).results

an4_words_flat_unigram:
	rm -f gmake-an4_words_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_words_flat_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.flat_unigram.lm.DMP" >> ARGS.an4_words_flat_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_flat_unigram$(MODEAFFIX) > gmake-an4_words_flat_unigram$(MODEAFFIX).results

an4_spelling_flat_unigram:
	rm -f gmake-an4_spelling_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.flat_unigram.lm.DMP" >> ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_flat_unigram$(MODEAFFIX) > gmake-an4_spelling_flat_unigram$(MODEAFFIX).results

an4_full_flat_unigram:
	rm -f gmake-an4_full_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_full_flat_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.flat_unigram.lm.DMP" >> ARGS.an4_full_flat_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_flat_unigram$(MODEAFFIX) > gmake-an4_full_flat_unigram$(MODEAFFIX).results

an4_words_bigram:
	rm -f gmake-an4_words_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_words_bigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.bigram.lm.DMP" >> ARGS.an4_words_bigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_bigram$(MODEAFFIX) > gmake-an4_words_bigram$(MODEAFFIX).results

an4_spelling_bigram:
	rm -f gmake-an4_spelling_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_spelling_bigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.bigram.lm.DMP" >> ARGS.an4_spelling_bigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_bigram$(MODEAFFIX) > gmake-an4_spelling_bigram$(MODEAFFIX).results

an4_full_bigram:
	rm -f gmake-an4_full_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_full_bigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.bigram.lm.DMP" >> ARGS.an4_full_bigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_bigram$(MODEAFFIX) > gmake-an4_full_bigram$(MODEAFFIX).results

an4_words_trigram:
	rm -f gmake-an4_words_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_words_trigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.trigram.lm.DMP" >> ARGS.an4_words_trigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_trigram$(MODEAFFIX) > gmake-an4_words_trigram$(MODEAFFIX).results

an4_spelling_trigram:
	rm -f gmake-an4_spelling_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_spelling_trigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.trigram.lm.DMP" >> ARGS.an4_spelling_trigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_trigram$(MODEAFFIX) > gmake-an4_spelling_trigram$(MODEAFFIX).results

an4_full_trigram:
	rm -f gmake-an4_full_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_full_trigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.trigram.lm.DMP" >> ARGS.an4_full_trigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_trigram$(MODEAFFIX) > gmake-an4_full_trigram$(MODEAFFIX).results



