#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
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

CFLAGS = $(DEBUG) ${$(MACHINE)_CFLAGS}

$(TARGET): $(OBJS) ${$(MACHINE)_OBJS}
	ar crv $@ $?
	ranlib $@

install:
	(cd $(MACHINE); make -f ../Makefile)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
	(cd $(MACHINE); rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~)
