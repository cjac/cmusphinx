#
# Makefile
# 
# HISTORY
# 
# 23-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


VPATH = .:..

include ../../Makefile.defines

TARGET = sen2s2

OBJS = sen2s2.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lmain -lmisc -lfeat -lio -lutil -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

install: $(TARGET)
	cp $(TARGET) $(S3BINDIR)
