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

$(BINDIR)/batchmetrics: ../batch_metrics.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ batch_metrics.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS) -R$(LIBDIR)

an4_words_unigram:
	rm -f gmake-an4-words-unigram.results
	/bin/cp ARGS.an4_base ARGS.an4_words_unigram
	echo "-lm an4_words.ug.lm.DMP" >> ARGS.an4_words_unigram
	$(BINDIR)/batchmetrics ./an4_words.batch / ./ARGS.an4_words_unigram > gmake-an4-words-unigram.results

an4_spelling_unigram:
	rm -f gmake-an4-spelling-unigram.results
	/bin/cp ARGS.an4_base ARGS.an4_spelling_unigram
	echo "-lm an4_spelling.ug.lm.DMP" >> ARGS.an4_spelling_unigram
	$(BINDIR)/batchmetrics ./an4_spelling.batch / ./ARGS.an4_spelling_unigram > gmake-an4-spelling-unigram.results

an4_full_unigram:
	rm -f gmake-an4-full-unigram.results
	/bin/cp ARGS.an4_base ARGS.an4_full_unigram
	echo "-lm an4.ug.lm.DMP" >> ARGS.an4_full_unigram
	$(BINDIR)/batchmetrics ./an4_full.batch / ./ARGS.an4_full_unigram > gmake-an4-full-unigram.results

an4_plus_unigram:
	rm -f gmake-an4-plus-unigram.results
	/bin/cp ARGS.an4_base ARGS.an4_plus_unigram
	echo "-lm an4_plus.ug.lm.DMP" >> ARGS.an4_plus_unigram
	$(BINDIR)/batchmetrics ./an4_full.batch / ./ARGS.an4_plus_unigram > gmake-an4-plus-unigram.results

an4_words_trigram:
	rm -f gmake-an4-words-trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_words_trigram
	echo "-lm an4_words.trigram.DMP" >> ARGS.an4_words_trigram
	$(BINDIR)/batchmetrics ./an4_words.batch / ./ARGS.an4_words_trigram > gmake-an4-words-trigram.results

an4_spelling_trigram:
	rm -f gmake-an4-spelling-trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_spelling_trigram
	echo "-lm an4_spelling.trigram.DMP" >> ARGS.an4_spelling_trigram
	$(BINDIR)/batchmetrics ./an4_spelling.batch / ./ARGS.an4_spelling_trigram > gmake-an4-spelling-trigram.results

an4_spelling_journey_trigram:
	rm -f gmake-an4-spelling-journey-trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_spelling_journey_trigram
	echo "-lm an4_spelling_journey.trigram.DMP" >> ARGS.an4_spelling_journey_trigram
	$(BINDIR)/batchmetrics ./an4_spelling.batch / ./ARGS.an4_spelling_journey_trigram > gmake-an4-spelling-journey-trigram.results

an4_full_trigram:
	rm -f gmake-an4-full-trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_full_trigram
	echo "-lm an4.trigram.DMP" >> ARGS.an4_full_trigram
	$(BINDIR)/batchmetrics ./an4_full.batch / ./ARGS.an4_full_trigram > gmake-an4-full-trigram.results

an4_plus_trigram:
	rm -f gmake-an4-plus-trigram.results
	/bin/cp ARGS.an4_base ARGS.an4_plus_trigram
	echo "-lm an4_plus.trigram.DMP" >> ARGS.an4_plus_trigram
	$(BINDIR)/batchmetrics ./an4_full.batch / ./ARGS.an4_plus_trigram > gmake-an4-plus-trigram.results

