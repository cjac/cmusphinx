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

WSJ5KBATCH=/lab/speech/sphinx4/data/wsj0/wsj0/wsj5k.batch
WSJ5KNVPBATCH=/lab/speech/sphinx4/data/wsj0/wsj0/wsj5k.e.batch
PROGRAMDIR = ${top_builddir}/src/programs
BMBIN = ${top_builddir}/src/test/programs/batchmetrics
LPBIN = $(PROGRAMDIR)/livepretend
DECODEBIN = $(PROGRAMDIR)/decode
ALIGNBIN = /net/elroy/usr1/archan/src/ravi/ALIGN/linux/align 

#all : lp decode
lp : wsj5k_bigram-lp wsj5k_trigram-lp wsj5k_bigram_nvp-lp wsj5k_trigram_nvp-lp
decode : wsj5k_bigram-decode wsj5k_trigram-decode wsj5k_bigram_nvp-decode wsj5k_trigram_nvp-decode
clean :
	-rm -f *.tmp *.ctl *.sent  *.lex *.results *.match *.match.align ARGS.*-{lp,decode}

#Arthur: legacy targets, I think this one doesn't work now. 
#wsj5k_bigram-bm wsj_trigram-bm :
#	 ../../programs/batchmetrics $(WSJ5KBATCH) / ./ARGS.$(subst _bm,,(subst _nvp,,$@)) > $@.results
#wsj5k_bigram_nvp-bm wsj5k_trigram_nvp-bm:
#	 ../../programs/batchmetrics $(WSJ5KNVPBATCH) / ./ARGS.$(subst _bm,,(subst _nvp,,$@)) > $@.results
#

wsj5k_bigram-lp wsj5k_trigram-lp :
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}' $(WSJ5KBATCH) |sed "s/\.raw//" > ./$@.ctl
	cat ARGS ARGS.live ARGS.$(subst -lp,,$@) > ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(LPBIN) ./$@.ctl / ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJ5KBATCH) |sed "s/\.wav\.raw//" > $@.sent
	tr 'A-Z' 'a-z' < $@.match > $@.tmp
	- $(ALIGNBIN) -def $@.sent -hyp $@.tmp > $@.match.align

wsj5k_bigram_nvp-lp wsj5k_trigram_nvp-lp:
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}' $(WSJ5KNVPBATCH) |sed "s/\.raw//" > ./$@.ctl
	cat ARGS ARGS.live ARGS.$(subst _nvp-lp,,$@) > ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(LPBIN) ./$@.ctl / ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJ5KNVPBATCH) |sed "s/\.wav\.raw//" > $@.sent
	tr 'A-Z' 'a-z' < $@.match > $@.tmp
	- $(ALIGNBIN) -def $@.sent -hyp $@.tmp > $@.match.align

wsj5k_bigram-decode wsj5k_trigram-decode :
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}' $(WSJ5KBATCH) |sed -e "s/\.raw//" -e "s/_raw/_cepstra/" > ./$@.ctl
	cat ARGS ARGS.$(subst -decode,,$@) > ARGS.$@
	echo "-ctl $@.ctl" >> ARGS.$@
	echo "-cepdir /" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJ5KBATCH) |sed -e "s/\.wav\.raw//" -e "s/_raw/_cepstra/" > $@.sent
	tr 'A-Z' 'a-z' < $@.match > $@.tmp
	- $(ALIGNBIN) -def $@.sent -hyp $@.tmp > $@.match.align

wsj5k_bigram_nvp-decode wsj5k_trigram_nvp-decode:
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}' $(WSJ5KNVPBATCH) |sed -e "s/\.raw//" -e "s/_raw/_cepstra/"> ./$@.ctl
	cat ARGS ARGS.$(subst _nvp-decode,,$@) > ARGS.$@
	echo "-ctl $@.ctl" >> ARGS.$@
	echo "-cepdir /" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(WSJ5KNVPBATCH) |sed -e "s/\.wav\.raw//" -e "s/_raw/_cepstra/"> $@.sent
	tr 'A-Z' 'a-z' < $@.match > $@.tmp
	- $(ALIGNBIN) -def $@.sent -hyp $@.tmp > $@.match.align




