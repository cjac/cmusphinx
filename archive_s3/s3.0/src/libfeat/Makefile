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

TARGET = libfeat.a

OBJS = feat.o

$(TARGET): $(OBJS)
	ar crv $@ $?
	ranlib $@

install: $(TARGET)
	mv $(TARGET) $(S3LIBDIR)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

feattest: feat.c
	cc -g -DFEAT_TEST=1 -L. -L$(S3LIBDIR) -o feattest feat.c -lio -lutil
