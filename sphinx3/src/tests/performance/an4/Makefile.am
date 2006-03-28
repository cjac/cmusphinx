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
	$(top_builddir)/src/libutil/libs3util.la \
	-lm @ad_libs@ 

PROGRAMDIR= ${top_builddir}/src/programs
BMBIN= ${top_builddir}/src/tests/programs/batchmetrics
LPBIN= $(PROGRAMDIR)/livepretend
DECODEBIN= $(PROGRAMDIR)/decode
DECANYBIN= $(PROGRAMDIR)/decode_anytopo
AN4RESULTPATH= ${top_builddir}/src/tests/performance/an4/
MODELSPATH=/lab/speech/sphinx4/data/an4/lists/
AN4BASE="ARGS.an4_base"
AN4LIVE="ARGS.an4_base.live"
# Add 
# "/afs/cs.cmu.edu/user/robust/archive/third_party_packages/NIST_scoring_tools/sctk-1.2c/linux/bin" 
# to your path if you have alinux machine
ALIGNBIN = sclite -i wsj -o rsum -f 0 

#	echo $(wordlist 2,3,$(subst _, ,$@))
#	echo $(word 3,$(subst _, ,$@))

CLEANFILES = *.results *.tmp *.match *.match.* *.sent *.ctl ARGS.*-{lp,decode,decany} 

SIMTASK="words spelling full"
GRAMMAR="unigram bigram flat_unigram trigram"
MODEAFFIX="quick"

#DON'T delete, single evil suffix rules for all these targets. 
#should create a foreach loop for all the below targets

#all : SLOW QUICK

slow : an4_words-unigram-lp \
	an4_spelling-unigram-lp \
	an4_full-unigram-lp \
	an4_words-bigram-lp \
	an4_spelling-bigram-lp \
	an4_full-bigram-lp \
	an4_words-trigram-lp \
	an4_spelling-trigram-lp \
	an4_full-trigram-lp \
	an4_words-flat_unigram-lp \
	an4_spelling-flat_unigram-lp \
	an4_full-flat_unigram-lp \
	an4_words-unigram-decode \
	an4_spelling-unigram-decode \
	an4_full-unigram-decode \
	an4_words-bigram-decode \
	an4_spelling-bigram-decode \
	an4_full-bigram-decode \
	an4_words-trigram-decode \
	an4_spelling-trigram-decode \
	an4_full-trigram-decode \
	an4_words-flat_unigram-decode \
	an4_spelling-flat_unigram-decode \
	an4_full-flat_unigram-decode \
	an4_words-unigram-decany \
	an4_spelling-unigram-decany \
	an4_full-unigram-decany \
	an4_words-bigram-decany \
	an4_spelling-bigram-decany \
	an4_full-bigram-decany \
	an4_words-trigram-decany \
	an4_spelling-trigram-decany \
	an4_full-trigram-decany \
	an4_words-flat_unigram-decany \
	an4_spelling-flat_unigram-decany \
	an4_full-flat_unigram-decany 

quick : an4_words_quick-unigram-lp \
	an4_spelling_quick-unigram-lp \
	an4_full_quick-unigram-lp \
	an4_words_quick-bigram-lp \
	an4_spelling_quick-bigram-lp \
	an4_full_quick-bigram-lp \
	an4_words_quick-trigram-lp \
	an4_spelling_quick-trigram-lp \
	an4_full_quick-trigram-lp \
	an4_words_quick-flat_unigram-lp \
	an4_spelling_quick-flat_unigram-lp \
	an4_full_quick-flat_unigram-lp \
	an4_words_quick-unigram-decode \
	an4_spelling_quick-unigram-decode \
	an4_full_quick-unigram-decode \
	an4_words_quick-bigram-decode \
	an4_spelling_quick-bigram-decode \
	an4_full_quick-bigram-decode \
	an4_words_quick-trigram-decode \
	an4_spelling_quick-trigram-decode \
	an4_full_quick-trigram-decode \
	an4_words_quick-flat_unigram-decany \
	an4_spelling_quick-flat_unigram-decany \
	an4_full_quick-flat_unigram-decany \
	an4_words_quick-unigram-decany \
	an4_spelling_quick-unigram-decany \
	an4_full_quick-unigram-decany \
	an4_words_quick-bigram-decany \
	an4_spelling_quick-bigram-decany \
	an4_full_quick-bigram-decany \
	an4_words_quick-trigram-decany \
	an4_spelling_quick-trigram-decany \
	an4_full_quick-trigram-decany \
	an4_words_quick-flat_unigram-decany \
	an4_spelling_quick-flat_unigram-decany \
	an4_full_quick-flat_unigram-decany 



an4_words-unigram-lp \
an4_spelling-unigram-lp \
an4_full-unigram-lp \
an4_words-bigram-lp \
an4_spelling-bigram-lp \
an4_full-bigram-lp \
an4_words-trigram-lp \
an4_spelling-trigram-lp \
an4_full-trigram-lp \
an4_words-flat_unigram-lp \
an4_spelling-flat_unigram-lp \
an4_full-flat_unigram-lp \
an4_words_quick-unigram-lp \
an4_spelling_quick-unigram-lp \
an4_full_quick-unigram-lp \
an4_words_quick-bigram-lp \
an4_spelling_quick-bigram-lp \
an4_full_quick-bigram-lp \
an4_words_quick-trigram-lp \
an4_spelling_quick-trigram-lp \
an4_full_quick-trigram-lp \
an4_words_quick-flat_unigram-lp \
an4_spelling_quick-flat_unigram-lp \
an4_full_quick-flat_unigram-lp :
	-rm -f $@.results $@.tmp $@.match $@.sent $@.ctl
	awk '{print $$1}' batch/$(word 1,$(subst -, ,$@)).batch | sed "s/\.raw//" > ./$@.ctl
	cat $(AN4BASE) $(AN4LIVE) ./ARGS/ARGS.an4_$(word 2,$(subst -, ,$@)) > ARGS.$@
	echo "-lm $(MODELSPATH)/$(subst -,.,$(subst _quick,,$(subst -lp,,$@))).lm.DMP" >> ARGS.$@
	echo "-hyp ./$@.match" >> ARGS.$@
	$(LPBIN) ./$@.ctl / ARGS.$@ > $@.results 2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' batch/$(word 1,$(subst -, ,$@)).batch  | sed "s/\.raw//" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match

an4_words-unigram-decode \
an4_spelling-unigram-decode \
an4_full-unigram-decode \
an4_words-bigram-decode \
an4_spelling-bigram-decode \
an4_full-bigram-decode \
an4_words-trigram-decode \
an4_spelling-trigram-decode \
an4_full-trigram-decode \
an4_words-flat_unigram-decode \
an4_spelling-flat_unigram-decode \
an4_full-flat_unigram-decode \
an4_words_quick-unigram-decode \
an4_spelling_quick-unigram-decode \
an4_full_quick-unigram-decode \
an4_words_quick-bigram-decode \
an4_spelling_quick-bigram-decode \
an4_full_quick-bigram-decode \
an4_words_quick-trigram-decode \
an4_spelling_quick-trigram-decode \
an4_full_quick-trigram-decode \
an4_words_quick-flat_unigram-decode \
an4_spelling_quick-flat_unigram-decode \
an4_full_quick-flat_unigram-decode :
	-rm -f $@.results $@.tmp $@.match $@.sent $@.ctl
	awk '{print $$1}' batch/$(word 1,$(subst -, ,$@)).batch | sed -e "s/\.raw//" -e "s/an4_clstk/cepstra/" > ./$@.ctl
	cat $(AN4BASE) ./ARGS/ARGS.an4_$(word 2,$(subst -, ,$@)) > ARGS.$@
	echo "-lm $(MODELSPATH)/$(subst -,.,$(subst _quick,,$(subst -decode,,$@))).lm.DMP" >> ARGS.$@
	echo "-ctl ./$@.ctl " >> ARGS.$@
	echo "-hyp ./$@.match" >> ARGS.$@
	$(DECODEBIN) ARGS.$@ > $@.results 2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' batch/$(word 1,$(subst -, ,$@)).batch  | sed -e "s/\.raw//" -e "s/an4_clstk/cepstra/" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match


an4_words-unigram-decany \
an4_spelling-unigram-decany \
an4_full-unigram-decany \
an4_words-bigram-decany \
an4_spelling-bigram-decany \
an4_full-bigram-decany \
an4_words-trigram-decany \
an4_spelling-trigram-decany \
an4_full-trigram-decany \
an4_words-flat_unigram-decany \
an4_spelling-flat_unigram-decany \
an4_full-flat_unigram-decany \
an4_words_quick-unigram-decany \
an4_spelling_quick-unigram-decany \
an4_full_quick-unigram-decany \
an4_words_quick-bigram-decany \
an4_spelling_quick-bigram-decany \
an4_full_quick-bigram-decany \
an4_words_quick-trigram-decany \
an4_spelling_quick-trigram-decany \
an4_full_quick-trigram-decany \
an4_words_quick-flat_unigram-decany \
an4_spelling_quick-flat_unigram-decany \
an4_full_quick-flat_unigram-decany :
	-rm -f $@.results $@.tmp $@.match $@.sent $@.ctl
	awk '{print $$1}' batch/$(word 1,$(subst -, ,$@)).batch | sed -e "s/\.raw//" -e "s/an4_clstk/cepstra/" > ./$@.ctl
	cat $(AN4BASE).decany ./ARGS/ARGS.an4_$(word 2,$(subst -, ,$@)).decany > ARGS.$@
	echo "-lm $(MODELSPATH)/$(subst -,.,$(subst _quick,,$(subst -decany,,$@))).lm.DMP" >> ARGS.$@
	echo "-ctl ./$@.ctl " >> ARGS.$@
	echo "-hyp ./$@.match" >> ARGS.$@
	$(DECANYBIN) ARGS.$@ > $@.results 2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' batch/$(word 1,$(subst -, ,$@)).batch  | sed -e "s/\.raw//" -e "s/an4_clstk/cepstra/" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match


#an4_words_unigram-bm \
#an4_spelling_unigram-bm \
#an4_full_unigram-bm \
#an4_words_bigram-bm \
#an4_spelling_bigram-bm \
#an4_full_bigram-bm \
#an4_words_trigram-bm \
#an4_spelling_trigram-bm \
#an4_full_trigram-bm:
#	 -rm -f $@.results $@.tmp $@.match
#	 cat $(AN4BASE) $(AN4LIVE) ./ARGS/ARGS.an4_$(word 3,$(subst _, ,$(subst -bm,, $@))) > ARGS.$@
#	 echo "-lm $(MODELSPATH)/an4_$(word 2,$(subst _, ,$@)).$(word 3,$(subst _, ,$(subst -bm,,$@)).lm.DMP" >> ARGS.$@
#	 $(BMBIN) batch/an4_$(word 2,$(subst _, ,$(subst -bm,,$@))).batch / ARGS.$@ > $@.results 2>&1
#
#
##hack!, minimal to get around limited make string manipulation functionalities. Didn't know how to awk or cut like operation at this point. 
#an4_words_flat_unigram-bm \
#an4_spelling_flat_unigram-bm \
#an4_full_flat_unigram-bm :
#	 -rm -f $@.results $@.tmp $@.match
#	 cat $(AN4BASE) $(AN4LIVE) ./ARGS/ARGS.an4_flat_unigram > ARGS.$@
#	 echo "-lm $(MODELSPATH)/an4_$(word 2,$(subst _, ,$(subst -bm,,$@))).flat_unigram.lm.DMP" >> ARGS.$@
#	 $(BMBIN) batch/an4_$(word 2,$(subst _, ,$(subst -bm,,$@))).batch / ARGS.$@ > $@.results 2>&1
