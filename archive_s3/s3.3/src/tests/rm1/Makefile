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
DIRNAME=src/tests/rm1
BUILD_DIRS =
ALL_DIRS= $(BUILD_DIRS)
MAINSRCS = dump_frontend.c batch_metrics.c
LIVEDECOBJS = $(LIVEDECSRCS:.c=.o) $(BASESRCS:.c=.o) 
FILES = Makefile $(SRCS) $(H)
LIBNAME= tests

LOCAL_INCLUDES = -I$(TOP)/src -I$(TOP)/src/libs3decoder -I$(TOP)/src/libs3audio

ALL = $(BINDIR)/dumpfrontend $(BINDIR)/batchmetrics

include $(TOP)/config/common_make_rules

$(BINDIR)/dumpfrontend: dump_frontend.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ dump_frontend.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS)

$(BINDIR)/batchmetrics: batch_metrics.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ batch_metrics.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS)


rm1_unigram:
	rm -f gmake-rm1_unigram.results
	/bin/cp ARGS.rm1_base ARGS.rm1_unigram
	echo "-lm RM.unigram.arpa.DMP" >> ARGS.rm1_unigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1_unigram > gmake-rm1_unigram.results


rm1_bigram:
	rm -f gmake-rm1_bigram.results
	/bin/cp ARGS.rm1_base ARGS.rm1_bigram
	echo "-lm RM.2845.bigram.arpa.DMP" >> ARGS.rm1_bigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1_bigram > gmake-rm1_bigram.results


rm1_trigram:
	rm -f gmake-rm1_trigram.results
	/bin/cp ARGS.rm1_base ARGS.rm1_trigram
	echo "-lm RM.2845.trigram.arpa.DMP" >> ARGS.rm1_trigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1_trigram > gmake-rm1_trigram.results


rm1onrm1_flatunigram:
	rm -f gmake-rm1onrm1_flatunigram.results
	/bin/cp ARGS.rm1onrm1_base ARGS.rm1onrm1_flatunigram
	echo "-lm RM.unigram.arpa.DMP" >> ARGS.rm1onrm1_flatunigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1onrm1_flatunigram > gmake-rm1onrm1_flatunigram.results


rm1onrm1_unigram:
	rm -f gmake-rm1onrm1_unigram.results
	/bin/cp ARGS.rm1onrm1_base ARGS.rm1onrm1_unigram
	echo "-lm RM.2845.unigram.arpa.DMP" >> ARGS.rm1onrm1_unigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1onrm1_unigram > gmake-rm1onrm1_unigram.results


rm1onrm1_bigram:
	rm -f gmake-rm1onrm1_bigram.results
	/bin/cp ARGS.rm1onrm1_base ARGS.rm1onrm1_bigram
	echo "-lm RM.2845.bigram.arpa.DMP" >> ARGS.rm1onrm1_bigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1onrm1_bigram > gmake-rm1onrm1_bigram.results


rm1onrm1_trigram:
	rm -f gmake-rm1onrm1_trigram.results
	/bin/cp ARGS.rm1onrm1_base ARGS.rm1onrm1_trigram
	echo "-lm RM.2845.trigram.arpa.DMP" >> ARGS.rm1onrm1_trigram
	$(BINDIR)/batchmetrics ./rm1.batch / ./ARGS.rm1onrm1_trigram > gmake-rm1onrm1_trigram.results
