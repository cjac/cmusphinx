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


CLEANFILES = gmake-rm1_flat_unigram$(MODEAFFIX).results \
             gmake-rm1_unigram$(MODEAFFIX).results \
             gmake-rm1_trigram$(MODEAFFIX).results

rm1_flat_unigram:
	rm -f gmake-rm1_flat_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-lm RM.unigram.arpa.DMP" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-lw 7" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-beam 1e-140" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-80" >> ARGS.rm1_flat_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_flat_unigram$(MODEAFFIX) > gmake-rm1_flat_unigram$(MODEAFFIX).results


rm1_unigram:
	rm -f gmake-rm1_unigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.unigram.arpa.DMP" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-lw 10" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-beam 1e-120" >> ARGS.rm1_unigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-60" >> ARGS.rm1_unigram$(MODEAFFIX)
	../../programs/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_unigram$(MODEAFFIX) > gmake-rm1_unigram$(MODEAFFIX).results


rm1_bigram:
	rm -f gmake-rm1_bigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.bigram.arpa.DMP" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-lw 13" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-beam 1e-120" >> ARGS.rm1_bigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-100" >> ARGS.rm1_bigram$(MODEAFFIX)
	../../programs/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_bigram$(MODEAFFIX) > gmake-rm1_bigram$(MODEAFFIX).results

rm1_trigram:
	rm -f gmake-rm1_trigram$(MODEAFFIX).results
	/bin/cp ARGS.rm1 ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-lm RM.2845.trigram.arpa.DMP" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-lw 14" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-beam 1e-140" >> ARGS.rm1_trigram$(MODEAFFIX)
	/bin/echo "-wbeam 1e-100" >> ARGS.rm1_trigram$(MODEAFFIX)
	../../programs/batchmetrics ./rm1$(MODEAFFIX).batch / ./ARGS.rm1_trigram$(MODEAFFIX) > gmake-rm1_trigram$(MODEAFFIX).results

