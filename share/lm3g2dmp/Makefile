#
# lm3g2dmp -- Makefile
# 
# HISTORY
# 
# 12-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
#		Created.
#


# Before compilation, set UTILDIR to point to the libutil directory installation
# UTILDIR = /net/alf20/usr2/rkm/libutil

# include $(UTILDIR)/Makefile.defines
MACHINE = sun

VPATH = .:..

TARGET = lm3g2dmp

OBJS =	lm3g2dmp.o \
	CM_funcs.o \
	hash.o \
	list.o \
	salloc.o \
	strcasecmp.o \
	err.o

install:
	(cd $(MACHINE); $(MAKE) -f ../Makefile $(TARGET))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
	(cd $(MACHINE); rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~)
