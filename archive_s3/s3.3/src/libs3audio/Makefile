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
#  Audio access
#
# ====================================================================
TOP=../..
DIRNAME=src/libs3audio
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
H = cont_ad.h mulaw.h audio_utils_sunos.h ad.h ad.h.in
AUDIOFILES= ad_osf.c ad_sunos.c mulaw_base.c \
            ad_base.c ad_oss.c audio_utils_sunos.c play_win32.c \
            ad_irix.c ad_oss_bsd.c cont_ad_base.c rec_win32.c ad_alsa.c
BASEFILES = mulaw_base.c ad_base.c cont_ad_base.c 
AUDIOFILE= ad_base.c 
SRCS = $(BASEFILES) $(AUDIOFILE)

FILES = Makefile $(H) $(BASEFILES) $(AUDIOFILES)
LIBNAME= s3audio

LOCAL_INCLUDES = -I$(TOP)/src -I$(TOP)/src/libs3decoder

ALL = .build_lib

include $(TOP)/config/common_make_rules

