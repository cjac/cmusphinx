# ====================================================================
# Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
# reserved.
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
#
#
# Copyright (c) 1995 Carnegie Mellon University.  ALL RIGHTS RESERVED.
# 
# HISTORY
# 
# 23-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
# 		Created.
#

# Before building:
# setenv MACHINE alpha {or sun4, hp700_ux, linux, etc. as appropriate}
# 
# Also, create the following directories if not already existing:
#     lib/$(MACHINE),
#     bin/$(MACHINE),
#     src/libutil/$(MACHINE),
#     src/libio/$(MACHINE),
#     src/libfeat/$(MACHINE),
#     src/libfbs/$(MACHINE),


all: verify s3decode-anytopo s3align s3allphone s3dag s3astar s3nbestrescore

verify:
	@if test x$(MACHINE) = x; then echo "Define MACHINE, e.g. \"make MACHINE=linux\""; exit 1; fi
	@if test x$(S3ROOT) = x; then echo "Define S3ROOT, e.g. \"make S3ROOT=\`pwd\`\""; exit 1; fi
	@if test ! -d lib; then mkdir lib; fi
	@if test ! -d lib/$(MACHINE); then mkdir lib/$(MACHINE); fi
	@if test ! -d bin; then mkdir bin; fi
	@if test ! -d bin/$(MACHINE); then mkdir bin/$(MACHINE); fi
	@if test ! -d src/libutil/$(MACHINE); then mkdir src/libutil/$(MACHINE); fi
	@if test ! -d src/libio/$(MACHINE); then mkdir src/libio/$(MACHINE); fi
	@if test ! -d src/libfeat/$(MACHINE); then mkdir src/libfeat/$(MACHINE); fi
	@if test ! -d src/libfbs/$(MACHINE); then mkdir src/libfbs/$(MACHINE); fi

libutil:
	(cd src/libutil/$(MACHINE); make -f ../Makefile install)

libio:
	(cd src/libio/$(MACHINE);   make -f ../Makefile install)

libfeat:
	(cd src/libfeat/$(MACHINE); make -f ../Makefile install)

libutil-dbg:
	(cd src/libutil/$(MACHINE)-debug; make -f ../Makefile-debug install)

libio-dbg:
	(cd src/libio/$(MACHINE)-debug;   make -f ../Makefile-debug install)

libfeat-dbg:
	(cd src/libfeat/$(MACHINE)-debug; make -f ../Makefile-debug install)

s3decode: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile install)

s3decode-dbg: libutil-dbg libio-dbg libfeat-dbg
	(cd src/libfbs/$(MACHINE)-debug;  make -f ../Makefile-debug install)

s3decode-anytopo: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-anytopo install)

s3decode-anytopodbg:
	(cd src/libfbs/$(MACHINE)-debug;  make -f ../Makefile-anytopodbg install)

s3align: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-align install)

s3allphone: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-allphone install)

s3dag: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-dag install)

s3astar: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-astar install)

s3nbestrescore: libutil libio libfeat
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-nbestrescore install)

only-s3decode:
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile install)

only-s3align:
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-align install)

only-s3allphone:
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-allphone install)

clean: verify
	-(cd src/libutil/$(MACHINE) && make -f ../Makefile clean)
	-(cd src/libio/$(MACHINE) &&   make -f ../Makefile clean)
	-(cd src/libfeat/$(MACHINE) && make -f ../Makefile clean)
	-(cd src/libfbs/$(MACHINE) &&  make -f ../Makefile clean)
	-(cd lib/$(MACHINE) && rm -f *.*)
	-(cd bin/$(MACHINE) && rm -f *.*)
