#
# Makefile
# 
# HISTORY
# 
# 02-Mar-99	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


VPATH = .:..

include ../../Makefile.defines

TARGET = treedec

OBJS = lextree.o main.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lmain -lmisc -lfeat -lio -lutil -lm
	rm main.o

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

install: $(TARGET)
	cp $(TARGET) $(S3BINDIR)
