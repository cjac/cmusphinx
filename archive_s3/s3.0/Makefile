#
# Copyright (c) 1997 Carnegie Mellon University.  ALL RIGHTS RESERVED.
# 
# HISTORY
# 
# 23-Nov-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
# 		Created.
#


# Before building:
#     setenv MACHINE alpha (or sun4, hp700_ux, linux, etc. as appropriate)
# 
# Create the following directories if not already existing:
#     lib/$(MACHINE),
#     bin/$(MACHINE),
#     src/libio/$(MACHINE),
#     src/libfeat/$(MACHINE),
#     src/libmisc/$(MACHINE),
#     src/libmain/$(MACHINE),


# This Makefile isn't foolproof; it may not ELIMINATE the need for manually
# determining dependencies, especially on updates to libraries and header files.
# When in doubt, make the necessary -clean targets first before the -install ones.
# (The -clean targets basically remove backup copies, *.o, *.a files.)

MAKE = gmake

# Machine-specific flags for "make"
#
alpha_MKFLAGS      =
alpha_osf1_MKFLAGS = $(alpha_MKFLAGS)
linux_MKFLAGS      = -I..
sun4_MKFLAGS       =
# 
MKFLAGS = ${${MACHINE}_MKFLAGS}


# Libraries (Note that src/libutil is a symbolic link to an s3-independent libutil):
# and is not maintained here.

lib-install:
	(cd src/libio/$(MACHINE);   $(MAKE) $(MKFLAGS) -f ../Makefile install)
	(cd src/libfeat/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile install)
	(cd src/libmisc/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile install)
	(cd src/libmain/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile install)

lib-clean:
	(cd src/libio/$(MACHINE);   $(MAKE) $(MKFLAGS) -f ../Makefile clean)
	(cd src/libfeat/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile clean)
	(cd src/libmisc/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile clean)
	(cd src/libmain/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile clean)

lib-src-clean:
	(cd src/libio;   $(MAKE) clean)
	(cd src/libfeat; $(MAKE) clean)
	(cd src/libmisc; $(MAKE) clean)
	(cd src/libmain; $(MAKE) clean)


# Applications:


# Time alignment (aka forced alignment)
#
timealign-install: lib-install
	(cd pgm/timealign/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile install)

timealign-clean:
	(cd pgm/timealign/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile clean)

timealign-src-clean:
	(cd pgm/timealign; $(MAKE) clean)


# Hyp extraction from multiple hypotheses files.
#
hypext-install: lib-install
	(cd pgm/hypext/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile install)

hypext-clean:
	(cd pgm/hypext/$(MACHINE); $(MAKE) $(MKFLAGS) -f ../Makefile clean)

hypext-src-clean:
	(cd pgm/hypext; $(MAKE) clean)


# All:


install: timealign-install \
	hypext-install

clean: timealign-clean \
	hypext-clean \
	lib-clean

src-clean: timealign-src-clean \
	hypext-src-clean \
	lib-src-clean
