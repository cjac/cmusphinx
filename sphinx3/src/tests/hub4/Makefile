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
DIRNAME=src/tests/hub4
BUILD_DIRS =
ALL_DIRS= $(BUILD_DIRS)
MAINSRCS = dump_frontend.c batch_metrics.c
LIVEDECOBJS = $(LIVEDECSRCS:.c=.o) $(BASESRCS:.c=.o) 
FILES = Makefile $(SRCS) $(H)
LIBNAME= tests

LOCAL_INCLUDES = -I$(TOP)/src -I$(TOP)/src/libs3decoder -I$(TOP)/src/libs3audio

ALL = $(BINDIR)/dumpfrontend $(BINDIR)/batchmetrics

include $(TOP)/config/common_make_rules

HUB4_MODEL_DIR=/lab/speech/sphinx4/data/hub4_model
HUB4_DIR=/lab/speech/sphinx4/data/hub4/eval99/supervised

ifeq ($(CONDITION),)
  CONDITION = all
endif

CTL_FILE=$(HUB4_DIR)/$(CONDITION)_hub4.batch

$(BINDIR)/dumpfrontend: dump_frontend.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ dump_frontend.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS)

$(BINDIR)/batchmetrics: batch_metrics.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ batch_metrics.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS)


hub4_flatunigram:
	rm -f gmake-hub4_flatunigram.results
	/bin/cp ARGS.hub4_base ARGS.hub4_flatunigram
	echo "-lm ${HUB4_MODEL_DIR}/hub4.flat_unigram.lm.DMP" >> ARGS.hub4_flatunigram
	$(BINDIR)/batchmetrics $(CTL_FILE) / ./ARGS.hub4_flatunigram > gmake-hub4_flatunigram.results


hub4_trigram:
	rm -f gmake-hub4_trigram.results
	/bin/cp ARGS.hub4_base ARGS.hub4_trigram
	rm -f gmake-hub4_trigram.results
	/bin/cp ARGS.hub4_base ARGS.hub4_trigram
	echo "-lm ${HUB4_MODEL_DIR}/language_model.arpaformat.DMP.Z" >> ARGS.hub4_flatunigram
	$(BINDIR)/batchmetrics $(CTL_FILE) / ./ARGS.hub4_trigram > gmake-hub4_trigram.results
