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


PROGRAMDIR = ${top_builddir}/src/programs
BMBIN = ${top_builddir}/src/test/programs/batchmetrics
LPBIN = $(PROGRAMDIR)/livepretend
DECODEBIN = $(PROGRAMDIR)/decode
DECANYBIN = $(PROGRAMDIR)/decode_anytopo
# Add 
# "/afs/cs.cmu.edu/user/robust/archive/third_party_packages/NIST_scoring_tools/sctk-1.2c/linux/bin" 
# to your path if you have alinux machine
ALIGNBIN = sclite -i wsj -o rsum -f 0 
CTLFILE = ./nist.ctl.no_comment.no_id
TRANSFILE = ./nist.trans.no_comment.no_id
COMPATH = /net/wilma/usr1/archan/Communicator

#all : base fast
base : com-base-lp com-base-decode com-base-decany
fast : com-fast-lp com-fast-decode

CLEANFILES = *.tmp *.ctl *.sent  *.lex *.results *.match *.match.* *.hyp* ARGS.*-{lp,decode}

com-base-lp com-fast-lp:
	-rm -f $@.results $@.match $@.match.* $@.hyp
	/bin/cp ARGS.$(subst -lp,,$@) ARGS.$@
	cat ARGS.live >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(LPBIN) $(CTLFILE) $(COMPATH)/nist_data/ ARGS.$@ > $@.results 2>&1 
	awk '{$$NF=""};{print $$0}' $@.match | sed -e "s/_/ /g" | paste - $(CTLFILE) | awk '{$$NF = "("$$NF")"};{print $$0}' > $@.hyp
	$(ALIGNBIN) -r $(TRANSFILE) -h $@.hyp

com-base-decode com-fast-decode: 
	-rm -f $@.results $@.match $@.match.* $@.hyp
	/bin/cp ARGS.$(subst -decode,,$@) ARGS.$@
	echo "-ctl $(CTLFILE)" >> ARGS.$@
	echo "-cepdir $(COMPATH)/nist_data_mfc/" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results 2>&1 
	awk '{$$NF=""};{print $$0}' $@.match | sed -e "s/_/ /g" | paste - $(CTLFILE) | awk '{$$NF = "("$$NF")"};{print $$0}' > $@.hyp
	$(ALIGNBIN) -r $(TRANSFILE) -h $@.hyp

com-2stage-decode:
	-rm -f $@.results $@.match $@.match.* $@.hyp
	/bin/cp ARGS.$(subst -decode,,$@) ARGS.$@
	echo "-ctl $(CTLFILE)" >> ARGS.$@
	echo "-cepdir $(COMPATH)/nist_data_mfc/" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results 2>&1 
	grep BSTPTH $@.results |awk '{for(i=1;i<=NF;i++){printf("%s ",$i)}printf("\n")}'  > $@.match
	awk '{$$NF=""};{print $$0}' $@.match | sed -e "s/_/ /g" | paste - $(CTLFILE) | awk '{$$NF = "("$$NF")"};{print $$0}' > $@.hyp
	$(ALIGNBIN) -r $(TRANSFILE) -h $@.hyp

com-base-decany:
	-rm -f $@.results $@.match $@.match.* $@.hyp
	/bin/cp ARGS.com-base.decany ARGS.$@
	echo "-ctl $(CTLFILE)" >> ARGS.$@
	echo "-cepdir $(COMPATH)/nist_data_mfc/" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECANYBIN) ARGS.$@ > $@.results 2>&1 
	awk '{$$NF=""};{print $$0}' $@.match | sed -e "s/_/ /g" | paste - $(CTLFILE) | awk '{$$NF = "("$$NF")"};{print $$0}' > $@.hyp
	$(ALIGNBIN) -r $(TRANSFILE) -h $@.hyp

com-2stage-decany:
	-rm -f $@.results $@.match $@.match.* $@.hyp
	/bin/cp ARGS.com-base.2stage-decany ARGS.$@
	echo "-ctl $(CTLFILE)" >> ARGS.$@
	echo "-cepdir $(COMPATH)/nist_data_mfc/" >> ARGS.$@
	echo "-hyp $@.match" >> ARGS.$@
	$(DECANYBIN) ARGS.$@ > $@.results 2>&1 
	grep BSTPTH $@.results |awk '{for(i=1;i<=NF;i++){printf("%s ",$i)}printf("\n")}'  > $@.match
	awk '{$$NF=""};{print $$0}' $@.match | sed -e "s/_/ /g" | paste - $(CTLFILE) | awk '{$$NF = "("$$NF")"};{print $$0}' > $@.hyp
	$(ALIGNBIN) -r $(TRANSFILE) -h $@.hyp

#	rm -f com.base.decode_anytopo.results
#	/bin/cp ARGS.com.base.dec_any ARGS.com.base.dec_any.slow
#	echo "-cepdir /net/wilma/usr1/archan/Communicator/nist_data_mfc/" >> ARGS.com.base.dec_any.slow
#	../../../programs/decode_anytopo ARGS.com.base.dec_any.slow
