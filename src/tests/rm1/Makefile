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

ifeq ($(MODE),quick)
  MODEAFFIX = _quick
  CTLFILE = rm1_quick.ctl
  REFFILE = rm1_quick.ref
else
  MODE = 
  CTLFILE = rm1.ctl
  REFFILE = rm1.ref
endif


ALL = $(BINDIR)/dumpfrontend $(BINDIR)/batchmetrics

include $(TOP)/config/common_make_rules

$(BINDIR)/dumpfrontend: dump_frontend.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ dump_frontend.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS)

$(BINDIR)/batchmetrics: batch_metrics.o $(LIVEDECOBJS)
	$(CC) $(CFLAGS) -o $@ batch_metrics.o $(LIVEDECOBJS) $(LDFLAGS) $(LIBS)


rm1_flat_unigram:
	rm -f gmake-rm1_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-lm RM.unigram.arpa.DMP" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-lw 8" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-beam 1e-120" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-40" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_flat_unigram$(MODEAFFIX) > gmake-rm1_flat_unigram$(MODEAFFIX).results


rm1_unigram:
	rm -f gmake-rm1_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.unigram.arpa.DMP" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-lw 13" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-beam 1e-120" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-60" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_unigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_unigram$(MODEAFFIX) > gmake-rm1_unigram$(MODEAFFIX).results


rm1_bigram:
	rm -f gmake-rm1_bigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.bigram.arpa.DMP" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-lw 17" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-beam 1e-140" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-100" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_bigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_bigram$(MODEAFFIX) > gmake-rm1_bigram$(MODEAFFIX).results

rm1_trigram:
	rm -f gmake-rm1_trigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.trigram.arpa.DMP" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-lw 27" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-beam 1e-160" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-120" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-ctl $(CTLFILE)" >> ARGS.rm1_trigram$(MODEAFFIX)
	$(BINDIR)/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_trigram$(MODEAFFIX) > gmake-rm1_trigram$(MODEAFFIX).results

