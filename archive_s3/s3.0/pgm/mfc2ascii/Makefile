#
# Makefile
# 
# HISTORY
# 
# 07-Jan-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


VPATH = .:..

include ../../Makefile.defines

TARGET = mfc2ascii

OBJS = mfc2ascii.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lio -lutil -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

install: $(TARGET)
	cp $(TARGET) $(S3BINDIR)
