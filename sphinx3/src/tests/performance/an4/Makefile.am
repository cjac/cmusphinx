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


CLEANFILES = gmake-an4_words_unigram$(MODEAFFIX).results \
	gmake-an4_spelling_unigram$(MODEAFFIX).results \
        gmake-an4_full_unigram$(MODEAFFIX).results \
	gmake-an4_words_flat_unigram$(MODEAFFIX).results \
	gmake-an4_spelling_flat_unigram$(MODEAFFIX).results \
	gmake-an4_full_flat_unigram$(MODEAFFIX).results \
	gmake-an4_words_bigram$(MODEAFFIX).results \
	gmake-an4_spelling_bigram$(MODEAFFIX).results \
	gmake-an4_full_bigram$(MODEAFFIX).results \
	gmake-an4_words_trigram$(MODEAFFIX).results \
	gmake-an4_spelling_trigram$(MODEAFFIX).results \
	gmake-an4_full_trigram$(MODEAFFIX).results


an4_words_unigram:
	rm -f gmake-an4_words_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_words_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.unigram.lm.DMP" >> ARGS.an4_words_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_unigram$(MODEAFFIX) > gmake-an4_words_unigram$(MODEAFFIX).results


an4_spelling_unigram:
	rm -f gmake-an4_spelling_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_spelling_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.unigram.lm.DMP" >> ARGS.an4_spelling_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_unigram$(MODEAFFIX) > gmake-an4_spelling_unigram$(MODEAFFIX).results
an4_full_unigram:
	rm -f gmake-an4_full_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_unigram ARGS.an4_full_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.unigram.lm.DMP" >> ARGS.an4_full_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_unigram$(MODEAFFIX) > gmake-an4_full_unigram$(MODEAFFIX).results

an4_words_flat_unigram:
	rm -f gmake-an4_words_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_words_flat_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.flat_unigram.lm.DMP" >> ARGS.an4_words_flat_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_flat_unigram$(MODEAFFIX) > gmake-an4_words_flat_unigram$(MODEAFFIX).results

an4_spelling_flat_unigram:
	rm -f gmake-an4_spelling_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.flat_unigram.lm.DMP" >> ARGS.an4_spelling_flat_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_flat_unigram$(MODEAFFIX) > gmake-an4_spelling_flat_unigram$(MODEAFFIX).results

an4_full_flat_unigram:
	rm -f gmake-an4_full_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_flat_unigram ARGS.an4_full_flat_unigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.flat_unigram.lm.DMP" >> ARGS.an4_full_flat_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_flat_unigram$(MODEAFFIX) > gmake-an4_full_flat_unigram$(MODEAFFIX).results

an4_words_bigram:
	rm -f gmake-an4_words_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_words_bigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.bigram.lm.DMP" >> ARGS.an4_words_bigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_bigram$(MODEAFFIX) > gmake-an4_words_bigram$(MODEAFFIX).results

an4_spelling_bigram:
	rm -f gmake-an4_spelling_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_spelling_bigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.bigram.lm.DMP" >> ARGS.an4_spelling_bigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_bigram$(MODEAFFIX) > gmake-an4_spelling_bigram$(MODEAFFIX).results

an4_full_bigram:
	rm -f gmake-an4_full_bigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_bigram ARGS.an4_full_bigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.bigram.lm.DMP" >> ARGS.an4_full_bigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_bigram$(MODEAFFIX) > gmake-an4_full_bigram$(MODEAFFIX).results

an4_words_trigram:
	rm -f gmake-an4_words_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_words_trigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_words.trigram.lm.DMP" >> ARGS.an4_words_trigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_words$(MODEAFFIX).batch / ./ARGS.an4_words_trigram$(MODEAFFIX) > gmake-an4_words_trigram$(MODEAFFIX).results

an4_spelling_trigram:
	rm -f gmake-an4_spelling_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_spelling_trigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_spelling.trigram.lm.DMP" >> ARGS.an4_spelling_trigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_spelling$(MODEAFFIX).batch / ./ARGS.an4_spelling_trigram$(MODEAFFIX) > gmake-an4_spelling_trigram$(MODEAFFIX).results

an4_full_trigram:
	rm -f gmake-an4_full_trigram$(MODEAFFIX).results
	/bin/cp ARGS.an4_trigram ARGS.an4_full_trigram$(MODEAFFIX)
	echo "-lm   /lab/speech/sphinx4/data/an4/lists/an4_full.trigram.lm.DMP" >> ARGS.an4_full_trigram$(MODEAFFIX)
	../../programs/batchmetrics ./an4_full$(MODEAFFIX).batch / ./ARGS.an4_full_trigram$(MODEAFFIX) > gmake-an4_full_trigram$(MODEAFFIX).results

