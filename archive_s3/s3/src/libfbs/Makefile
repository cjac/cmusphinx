#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


# S3ROOT = /net/alf20/usr/rkm/s3
AFSS3  = /afs/cs/project/plus-2/s3

include $(S3ROOT)/Makefile.defines
VPATH = .:..

TARGET = s3decode

OBJS =	agc.o \
	cmn.o \
	gauden.o \
	senone.o \
	interp.o \
	tmat.o \
	logs3.o \
	vector.o \
	dict.o \
	mdef.o \
	lm.o \
	fillpen.o \
	mllr.o \
	fwd.o \
	lmcontext.o \
	main.o

CFLAGS = $(S3DEBUG) $(S2HMMTOPO) ${$(MACHINE)_CFLAGS}

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -L$(LIBDIR) -o $(TARGET) $(OBJS) -lfeat -lio -lmisc -lutil -lm
	rm main.o fwd.o

install: $(TARGET)
	cp $(TARGET) $(BINDIR)

afs: $(TARGET)
	cp $(TARGET) $(AFSS3)/bin/$(MACHINE)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
