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

TARGET = libmisc.a

OBJS =	corpus.o

CFLAGS = $(S3DEBUG) ${$(MACHINE)_CFLAGS}

$(TARGET): $(OBJS) ${$(MACHINE)_OBJS}
	$(AR) crv $@ $?
	ranlib $@

install: $(TARGET)
	mv $(TARGET) $(LIBDIR)

corpus_test: corpus.c
	$(CC) -g -D_CORPUS_TEST_=1 $(CFLAGS) -L$(LIBDIR) -o corpus_test corpus.c -lutil -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
