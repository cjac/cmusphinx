#
# Copyright (c) 1995 Carnegie Mellon University.  ALL RIGHTS RESERVED.
# 
# HISTORY
# 
# 23-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
# 		Created.
#

# Before building:
#     setenv MACHINE alpha (or sun4, hp700_ux, linux, etc. as appropriate)
# 
# Create the following directories if not already existing:
#     lib/$(MACHINE),
#     bin/$(MACHINE),
#     src/libutil/$(MACHINE),
#     src/libio/$(MACHINE),
#     src/libfeat/$(MACHINE),
#     src/libmisc/$(MACHINE),
#     src/libfbs/$(MACHINE),

s3decode:
	(cd src/libutil/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libio/$(MACHINE);   $(MAKE) -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libmisc/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-anytopo install)
# Note that the default Makefile for s3decode above (i.e., libfbs/Makefile) builds a
# version of s3decode in which the HMM topology is hardwired to the Sphinx-II form.
# (That is, the HMM topology is not derived from the transition matrices input.)
# The second version, libfbs/Makefile-anytopo, builds another version of s3decode that
# can handle any left-to-right HMM topology.

s3dag:
	(cd src/libutil/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libio/$(MACHINE);   $(MAKE) -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libmisc/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-dag install)

s3astar:
	(cd src/libutil/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libio/$(MACHINE);   $(MAKE) -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libmisc/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-astar install)

s3align:
	(cd src/libutil/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libio/$(MACHINE);   $(MAKE) -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libmisc/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-align install)

s3allphone:
	(cd src/libutil/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libio/$(MACHINE);   $(MAKE) -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libmisc/$(MACHINE); $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-allphone install)

only-s3decode:
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile install)
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-anytopo install)

only-s3dag:
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-dag install)

only-s3astar:
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-astar install)

only-s3align:
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-align install)

only-s3allphone:
	(cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile-allphone install)

clean:
	- (cd src/libutil/$(MACHINE); $(MAKE) -f ../Makefile clean)
	- (cd src/libio/$(MACHINE);   $(MAKE) -f ../Makefile clean)
	- (cd src/libfeat/$(MACHINE); $(MAKE) -f ../Makefile clean)
	- (cd src/libmisc/$(MACHINE); $(MAKE) -f ../Makefile clean)
	- (cd src/libfbs/$(MACHINE);  $(MAKE) -f ../Makefile clean)
	- (cd src/libutil/;	$(MAKE) -f ./Makefile clean)
	- (cd src/libio/;   	$(MAKE) -f ./Makefile clean)
	- (cd src/libfeat/; 	$(MAKE) -f ./Makefile clean)
	- (cd src/libmisc/; 	$(MAKE) -f ./Makefile clean)
	- (cd src/libfbs/;  	$(MAKE) -f ./Makefile clean)

clean-all-obj:
	- (cd src/libutil/;	rm */*.o)
	- (cd src/libio/;   	rm */*.o)
	- (cd src/libfeat/; 	rm */*.o)
	- (cd src/libmisc/; 	rm */*.o)
	- (cd src/libfbs/;  	rm */*.o)

clean-purify:
	- (cd lib; rm */*pure*.a)

all: s3decode s3dag s3astar s3align s3allphone
