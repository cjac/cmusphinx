#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#

# S3ROOT = /net/alf20/usr/rkm/s3
include $(S3ROOT)/Makefile.defines
VPATH = .:..

TARGET = libutil.a

OBJS = cmd_ln.o \
	ckd_alloc.o \
	err.o \
	nextword.o \
	hash.o \
	linklist.o \
	unlimit.o \
	line2words.o \
	io.o \
	profile.o

alpha_OBJS = rpcc.o

CFLAGS = $(S3DEBUG) ${$(MACHINE)_CFLAGS}

$(TARGET): $(OBJS) ${$(MACHINE)_OBJS}
	$(AR) crv $@ $?
	ranlib $@

install: $(TARGET)
	mv $(TARGET) $(LIBDIR)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
