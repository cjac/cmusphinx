#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#

#
# For compiling this library, do the following:
#   cd to this directory (i.e., libutil)
#   setenv MACHINE linux (or alpha, sun, etc. as appropriate)
#   make
# The result is libutil.a in a $MACHINE subdirectory
#

VPATH = .:..

include Makefile.defines

TARGET = libutil.a

OBJS =	bitvec.o \
	case.o \
	ckd_alloc.o \
	cmd_ln.o \
	err.o \
	filename.o \
	glist.o \
	hash.o \
	heap.o \
	io.o \
	profile.o \
	str2words.o \
	unlimit.o

alpha_OBJS = rpcc.o

CFLAGS = ${$(MACHINE)_CFLAGS} $(CMDFLAGS)

install:
	mkdir -p $(MACHINE)
	(cd $(MACHINE); $(MAKE) $(MFLAGS) -f ../Makefile $(TARGET))

$(TARGET): $(OBJS) ${$(MACHINE)_OBJS}
	$(AR) crv $@ $?
	$(RANLIB) $@

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
	(cd $(MACHINE); rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~)
