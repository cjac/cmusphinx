# ====================================================================
# Copyright (c) 2004 Carnegie Mellon University.  All rights reserved.
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

DATAPATH = /lab/speech/sphinx4/data/ti46/ti20/test/test/
TI46RAWPATH = $(DATAPATH)/raw16k
TI46CEPPATH = $(DATAPATH)/cepstra 
TI46RESULTPATH = ${top_builddir}/src/tests/performance/ti46/
PROGRAMDIR = ${top_builddir}/src/programs
BMBIN = ${top_builddir}/src/test/programs/batchmetrics
LPBIN = $(PROGRAMDIR)/livepretend
DECODEBIN = $(PROGRAMDIR)/decode
DECANYBIN = $(PROGRAMDIR)/decode_anytopo
ALIGNBIN = /net/elroy/usr1/archan/src/ravi/ALIGN/linux/align 

ALLTARGET=ti46-lp ti46-quick-lp ti46-decode ti46-quick-decode

#legacy targets, For ti46, batchmetric doesn't work at all.
#ti46-bm ti46-quick-bm:
#	rm -f $@.results $@.ctl
#	cat ARGS ARGS.live > ARGS.$@
#	$(BMBIN) $(subst -bm,,$@).batch $(TI46RAWPATH) ARGS.$@ > $@.results 2>&1

#all : $(ALLTARGET)
quick : ti46-quick-lp ti46-quick-decode ti46-quick-decany
slow : ti46-lp ti46-decode ti46-decany

clean : 
	-rm -f *.ctl *.sent  *.lex *.results *.match *.match.align ARGS.ti46-*

#Targets which we are actively using. 
ti46-lp ti46-quick-lp : 
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}' $(subst -lp,,$@).batch |sed "s/\.raw//" > ./$@.ctl
	cat ARGS ARGS.live > ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(LPBIN) ./$@.ctl $(TI46RAWPATH) ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(subst -lp,,$@).batch |sed "s/\.wav\.raw//" > $@.sent
	- $(ALIGNBIN) -def $@.sent -hyp $@.match > $@.match.align

ti46-decode ti46-quick-decode: 
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}'  $(subst -decode,,$@).batch |sed "s/\.wav\.raw//" > ./$@.ctl
	/bin/cp ARGS ARGS.$@
	echo "-ctl $@.ctl" >> ARGS.$@
	echo "-cepdir $(TI46CEPPATH)" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results 2>&1
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(subst -decode,,$@).batch |sed "s/\.wav\.raw//" > $@.sent
	- $(ALIGNBIN) -def $@.sent -hyp $@.match  > $@.match.align

ti46-decany ti46-quick-decany: 
	-rm -f $@.results $@.match $@.match.align
	awk '{print $$1}'  $(subst -decany,,$@).batch |sed "s/\.wav\.raw//" > ./$@.ctl
	/bin/cp ARGS.decany ARGS.$@
	echo "-ctl $@.ctl" >> ARGS.$@
	echo "-cepdir $(TI46CEPPATH)" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECANYBIN) ARGS.$@ > $@.results 2>&1
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(subst -decany,,$@).batch |sed "s/\.wav\.raw//" > $@.sent
	- $(ALIGNBIN) -def $@.sent -hyp $@.match  > $@.match.align










