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
# 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
#    endorse or promote products derived from this software without
#    prior written permission. To obtain permission, contact 
#    sphinx@cs.cmu.edu.
#
# 4. Products derived from this software may not be called "Sphinx"
#    nor may "Sphinx" appear in their names without prior written
#    permission of Carnegie Mellon University. To obtain permission,
#    contact sphinx@cs.cmu.edu.
#
# 5. Redistributions of any form whatsoever must retain the following
#    acknowledgment:
#    "This product includes software developed by Carnegie
#    Mellon University (http://www.speech.cs.cmu.edu/)."
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
TOP=../..
DIRNAME=src/libs3decoder
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
H = agc.h ascr.h beam.h bio.h cmn.h cmn_prior.h \
    cont_mgau.h corpus.h dict.h dict2pid.h feat.h fillpen.h \
    hmm.h hyp.h kb.h kbcore.h lextree.h lm.h logs3.h mdef.h \
    new_fe.h new_fe_sp.h s3types.h subvq.h tmat.h utt.h vector.h \
    vithist.h wid.h
SRCS = agc.c ascr.c beam.c bio.c cmn.c cont_mgau.c corpus.c dict.c \
	  dict2pid.c feat.c fillpen.c hmm.c kbcore.c lextree.c lm.c \
          logs3.c mdef.c subvq.c tmat.c vector.c vithist.c wid.c \
          utt.c kb.c cmn_prior.c new_fe.c new_fe_sp.c
FILES = Makefile $(SRCS) $(H)
LIBNAME= s3decoder

LOCAL_INCLUDES = -I$(TOP)/src

ALL = .build_lib

include $(TOP)/config/common_make_rules

