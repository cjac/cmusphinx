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

INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 

WSJBATCH = ./wsj20k.e.batch
PROGRAMDIR = ${top_builddir}/src/programs
BMBIN = ${top_builddir}/src/test/programs/batchmetrics
LPBIN = $(PROGRAMDIR)/livepretend
DECODEBIN = $(PROGRAMDIR)/decode
DECANYBIN = $(PROGRAMDIR)/decode_anytopo
# Add 
# "/afs/cs.cmu.edu/user/robust/archive/third_party_packages/NIST_scoring_tools/sctk-1.2c/linux/bin" 
# to your path if you have alinux machine
ALIGNBIN = sclite -i wsj -o rsum -f 0 


#legacy targets
#wsj20k_bigram-bm wsj20k_trigram-bm:
#	rm -f $@.results 
#	../../programs/batchmetrics $(WSJBATCH) / ./ARGS.$@ > $@.results

CLEANFILES = *.tmp *.ctl *.sent  *.lex *.results *.match *.match.* ARGS.*-{lp,decode}

#all: wsj20k_bigram-lp wsj20k_trigram-lp wsj20k_bigram-decode wsj20k_trigram-decode
lp: wsj20k_bigram-lp wsj20k_trigram-lp 
decode: wsj20k_bigram-decode wsj20k_trigram-decode
decany: wsj20k_bigram-decany wsj20k_trigram-decany

wsj20k_bigram-lp wsj20k_trigram-lp:
	-rm -f $@.results $@.match $@.match.*
	awk '{print $$1}'  $(WSJBATCH) | sed "s/\.raw//" > ./$@.ctl
	cat ARGS ARGS.live ARGS.$(subst -lp,,$@) > ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(LPBIN) ./$@.ctl / ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJBATCH) | sed "s/\.raw//" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match

wsj20k_bigram-decode wsj20k_trigram-decode: 
	-rm -f $@.results $@.match $@.match.*
	awk '{print $$1}'  $(WSJBATCH) | sed -e "s/\.raw//" -e "s/_raw/_cepstra/"> ./$@.ctl
	cat ARGS ARGS.$(subst -decode,,$@) > ARGS.$@
	echo "-ctl $@.ctl" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results 2>&1
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJBATCH) | sed -e "s/\.raw//" -e "s/_raw/_cepstra/" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match


wsj20k_bigram-decany wsj20k_trigram-decany: 
	-rm -f $@.results $@.match $@.match.*
	awk '{print $$1}'  $(WSJBATCH) | sed -e "s/\.raw//" -e "s/_raw/_cepstra/"> ./$@.ctl
	cat ARGS.decany ARGS.$(subst -decany,,$@).decany > ARGS.$@
	echo "-ctl $@.ctl" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECANYBIN) ARGS.$@ > $@.results 2>&1
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJBATCH) | sed -e "s/\.raw//" -e "s/_raw/_cepstra/" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match
