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
#  Top Level Makefile for Sphinx 3
#
# ====================================================================
TOP=.
DIRNAME=.
BUILD_DIRS = src
ALL_DIRS=config lib doc $(BUILD_DIRS)
CONFIG=config.guess config.sub configure configure.in \
       install-sh missing mkinstalldirs 
FILES = Makefile README ReleaseNotes $(CONFIG)

ALL = 

# Try and see if config hasn't been created
config_dummy := $(shell test -f config/config || ( echo '*** '; echo '*** Running configure to make default config file ***'; echo '*** '; ./configure; )  >&2)

# force a check on the system file
system_dummy := $(shell test -f config/system.mak || config/system.sh >config/system.mak)

include $(TOP)/config/common_make_rules

backup: time-stamp
	@ $(RM) -f $(TOP)/FileList
	@ $(MAKE) file-list
	@ echo .time-stamp >>FileList
	@ sed 's/^\.\///' <FileList | sed 's/^/'$(PROJECT_PREFIX)'\//' >.file-list-all
	@ (cd ..; tar zcvf $(PROJECT_PREFIX)/$(PROJECT_PREFIX)-$(PROJECT_VERSION)-$(PROJECT_STATE).tar.gz `cat $(PROJECT_PREFIX)/.file-list-all`)
	@ $(RM) -f $(TOP)/.file-list-all
	@ ls -l $(PROJECT_PREFIX)-$(PROJECT_VERSION)-$(PROJECT_STATE).tar.gz

tags:
	@ $(RM) -f $(TOP)/FileList
	@ $(MAKE) file-list
	etags `cat FileList | grep "\.[ch]$$"`

time-stamp :
	@ echo $(PROJECT_NAME) >.time-stamp
	@ echo $(PROJECT_PREFIX) >>.time-stamp
	@ echo $(PROJECT_VERSION) >>.time-stamp
	@ echo $(PROJECT_DATE) >>.time-stamp
	@ echo $(PROJECT_STATE) >>.time-stamp
	@ echo $(LOGNAME) >>.time-stamp
	@ hostname >>.time-stamp
	@ date >>.time-stamp

config/config: config/config.in config.status
	./config.status

configure: configure.in
	autoconf

test:
	@ $(MAKE) --no-print-directory -C testsuite test

