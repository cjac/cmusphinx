#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


include ../../Makefile.defines

VPATH = .:..

TARGET = libio.a

OBJS =	bio.o

$(TARGET): $(OBJS)
	ar crv $@ $?
	ranlib $@

install: $(TARGET)
	cp $(TARGET) $(S3LIBDIR)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
