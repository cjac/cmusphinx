/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
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

s3decode:
	(cd src/libutil/$(MACHINE); make -f ../Makefile install)
	(cd src/libio/$(MACHINE);   make -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); make -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile install)

s3decode-dbg:
	(cd src/libutil/$(MACHINE)-debug; make -f ../Makefile-debug install)
	(cd src/libio/$(MACHINE)-debug;   make -f ../Makefile-debug install)
	(cd src/libfeat/$(MACHINE)-debug; make -f ../Makefile-debug install)
	(cd src/libfbs/$(MACHINE)-debug;  make -f ../Makefile-debug install)

s3decode-anytopo:
	(cd src/libutil/$(MACHINE); make -f ../Makefile install)
	(cd src/libio/$(MACHINE);   make -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); make -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-anytopo install)

s3decode-anytopodbg:
	(cd src/libutil/$(MACHINE); make -f ../Makefile-debug install)
	(cd src/libio/$(MACHINE);   make -f ../Makefile-debug install)
	(cd src/libfeat/$(MACHINE); make -f ../Makefile-debug install)
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-anytopodbg install)

s3align:
	(cd src/libutil/$(MACHINE); make -f ../Makefile install)
	(cd src/libio/$(MACHINE);   make -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); make -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-align install)

s3allphone:
	(cd src/libutil/$(MACHINE); make -f ../Makefile install)
	(cd src/libio/$(MACHINE);   make -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); make -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-allphone install)

only-s3decode:
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile install)

only-s3align:
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-align install)

only-s3allphone:
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile-allphone install)

clean:
	(cd src/libutil/$(MACHINE); make -f ../Makefile clean)
	(cd src/libio/$(MACHINE);   make -f ../Makefile clean)
	(cd src/libfeat/$(MACHINE); make -f ../Makefile clean)
	(cd src/libfbs/$(MACHINE);  make -f ../Makefile clean)
	(cd lib/$(MACHINE); rm -f *.*)
	(cd bin/$(MACHINE); rm -f *.*)
