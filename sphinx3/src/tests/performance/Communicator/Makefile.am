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

CLEANFILES = gmake.com.base.lp$(MODEAFFIX).results \
	     gmake.com.base.decode$(MODEAFFIX).results \
	     gmake.com.base.decode_anytopo$(MODEAFFIX).results \
	     gmake.com.fast.lp$(MODEAFFIX).results \
	     gmake.com.fast.decode$(MODEAFFIX).results  

com-base-lp:
	rm -f gmake.com.base.lp$(MODEAFFIX).results
	/bin/cp ARGS.com.base ARGS.com.base.lp$(MODEAFFIX)
	../../../programs/livepretend ./nist.ctl.no_comment.no_id /net/wilma/usr1/archan/Communicator/nist_data ARGS.com.base.lp$(MODEAFFIX)

com-base-decode:
	rm -f gmake.com.base.decode$(MODEAFFIX).results
	/bin/cp ARGS.com.base ARGS.com.base.decode$(MODEAFFIX)
	echo "-cepdir /net/wilma/usr1/archan/Communicator/nist_data_mfc/" >> ARGS.com.base.decode$(MODEAFFIX)
	../../../programs/decode ARGS.com.base.decode$(MODEAFFIX);


com-fast-lp:
	rm -f gmake.com.fast.lp$(MODEAFFIX).results
	/bin/cp ARGS.com.fast ARGS.com.fast.lp$(MODEAFFIX)
	../../../programs/livepretend ./nist.ctl.no_comment.no_id /net/wilma/usr1/archan/Communicator/nist_data ARGS.com.fast.lp$(MODEAFFIX)

com-fast-decode:
	rm -f gmake.com.fast.decode$(MODEAFFIX).results
	/bin/cp ARGS.com.fast ARGS.com.fast.decode$(MODEAFFIX)
	echo "-cepdir /net/wilma/usr1/archan/Communicator/nist_data_mfc/" >> ARGS.com.fast.decode$(MODEAFFIX)
	../../../programs/decode ARGS.com.fast.decode$(MODEAFFIX)

#20041205 DON'T USE at this point
com-base-decode_anytopo:
	rm -f gmake.com.base.decode_anytopo$(MODEAFFIX).results
	/bin/cp ARGS.com.base.dec_any ARGS.com.base.dec_any.slow$(MODEAFFIX)
	echo "-cepdir /net/wilma/usr1/archan/Communicator/nist_data_mfc/" >> ARGS.com.base.dec_any.slow$(MODEAFFIX)
	../../../programs/decode_anytopo ARGS.com.base.dec_any.slow$(MODEAFFIX)
