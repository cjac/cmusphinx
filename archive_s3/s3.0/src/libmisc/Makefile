#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


include ../../../Makefile.defines

VPATH = .:..

TARGET = libmisc.a

OBJS = logs3.o vector.o corpus.o dictmisc.o

$(TARGET): $(OBJS)
	ar crv $@ $?
	ranlib $@

install: $(TARGET)
	cp $(TARGET) $(S3LIBDIR)

vq_test : vector.c
	$(CC) $(CMDFLAGS) -D_VQ_TEST_=1 $(CFLAGS) -o $@ $> -lutil -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
