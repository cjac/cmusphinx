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

BMBIN=$(top_builddir)/src/tests/programs/batchmetrics
LPBIN=$(top_builddir)/src/programs/livepretend
DECODEBIN=$(top_builddir)/src/programs/decode
RMBATCH="./rm1.batch"
RMQUICKBATCH="./rm1_quick.batch"
RMARG="ARGS.rm1"
LVRMARG="ARGS.rm1.live"
# Add 
# "/afs/cs.cmu.edu/user/robust/archive/third_party_packages/NIST_scoring_tools/sctk-1.2c/linux/bin" 
# to your path if you have alinux machine
ALIGNBIN = sclite -i rm -o rsum -f 0 

#all : lp decode lp_quick decode_quick
slow : lp decode
quick : lp_quick decode_quick

lp : lp_quick lp_slow
lp_slow : rm1_flat_unigram-lp rm1_unigram-lp rm1_bigram-lp rm1_trigram-lp 
lp_quick : rm1_flat_unigram_quick-lp rm1_unigram_quick-lp rm1_bigram_quick-lp rm1_trigram_quick-lp

decode : decode_quick decode_slow
decode_slow : rm1_flat_unigram-decode rm1_unigram-decode rm1_bigram-decode rm1_trigram-decode
decode_quick : rm1_quick_flat_unigram-decode rm1_quick_unigram-decode rm1_quick_bigram-decode rm1_quick_trigram-decode

CLEANFILES = *.ctl *.sent *.results *.match  *.match.* ARG.rm1_*-* *-lp *-bm *-decode 

#Arthur: Should use suffix rules, didn't know how to use it at this point. Argument substitution is also ugly.

rm1_flat_unigram-lp \
rm1_unigram-lp \
rm1_bigram-lp \
rm1_trigram-lp :
	-rm -f $@.results $@.match $@.match.*
	/bin/cat $(RMARG) $(LVRMARG) ARGS.$(subst -lp,,$@) > ARGS.$@	
	awk '{print $$1}' $(RMBATCH) | sed "s/\.wav\.raw//"> ./$@.ctl
	/bin/echo "-hyp $@.match" >> ARGS.$@
	/bin/echo "-rawext .wav.raw" >> ARGS.$@
	$(LPBIN) ./$@.ctl / ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(RMBATCH) | sed "s/\.wav\.raw//" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match

rm1_flat_unigram_quick-lp \
rm1_unigram_quick-lp \
rm1_bigram_quick-lp \
rm1_trigram_quick-lp: 
	-rm -f $@.results $@.match $@.match.*
	/bin/cat $(RMARG) $(LVRMARG) ARGS.$(subst _quick,,$(subst -lp,,$@)) > ARGS.$@	
	awk '{print $$1}' $(RMQUICKBATCH) | sed "s/\.wav\.raw//"> ./$@.ctl
	/bin/echo "-hyp $@.match" >> ARGS.$@
	/bin/echo "-rawext .wav.raw" >> ARGS.$@
	$(LPBIN) ./$@.ctl / ARGS.$@ > $@.results  2>&1 
	awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' $(RMQUICKBATCH) | sed "s/\.wav\.raw//" > $@.sent
	$(ALIGNBIN) -r $@.sent -h $@.match


#rm1_flat_unigram-decode rm1_unigram-decode rm1_bigram-decode rm1_trigram-decode: 
#	 /bin/cat $(RMARG) ARGS.$(subst -decode,,$@) > ARGS.$@
#	 awk '{print $$1}' $(RMBATCH) | sed "s/\.wav\.raw//"> ./$@.ctl
#	 /bin/echo "-ctl $@.ctl" >> ARGS.$@
#	 /bin/echo "-hyp $@.match" >> ARGS.$@
#	 $(DECODEBIN) ARGS.$@ > $@.results 2>&1
#	awk -F/ '{print $$NF}' $(subst -decode,,$@).batch | awk '{for(i=2;i<=NF;i++){printf("%s ",$$i)}; printf("(%s)\n",$$1)}' | sed "s/\.wav\.raw//" > $@.sent
#	 $(ALIGNBIN) -r $@.sent -h $@.match
#

#legacy targets using BM
#rm1_flat_unigram-bm rm1_unigram-bm rm1_bigram-bm rm1_trigram-bm:
#	rm -f $@.results
#	cat $(RMARG) $(LVRMARG) ARGS.$(subst -bm,,$@) >ARGS.$@
#	$(BMBIN) $(RMBATCH) / ./ARGS.$@ > $@.results
