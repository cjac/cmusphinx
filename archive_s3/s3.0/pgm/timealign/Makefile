#
# Makefile
# 
# HISTORY
# 
# 07-Jan-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


VPATH = .:..

include ../../../Makefile.defines

TARGET = timealign

OBJS = wdnet.o \
	align.o \
	align-main.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lmain -lmisc -lfeat -lio -lutil -lm
	rm align-main.o

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

wdnet-test : wdnet.c
	$(CC) $(S3DEBUG) -D_WDNET_TEST_=1 $(CFLAGS) -o $@ $> -lmain -lmisc -lio -lutil -lm

install: $(TARGET)
	cp $(TARGET) $(S3BINDIR)
