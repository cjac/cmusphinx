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

TARGET = libfeat.a

OBJS = feat.o

CFLAGS = ${$(MACHINE)_CFLAGS}

$(TARGET): $(OBJS)
	$(AR) crv $@ $?
	ranlib $@

install: $(TARGET)
	mv $(TARGET) $(LIBDIR)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

feattest: feat.c
	cc -g $(CFLAGS) -DFEAT_TEST=1 -L. -L$(LIBDIR) -o feattest feat.c -lio -lutil
