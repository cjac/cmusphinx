#
# Makefile
# 
# HISTORY
# 
# 07-Jan-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


include ../../Makefile.defines

VPATH = .:..

OBJS = 	../../libmain/$(MACHINE)/misc.o \
	../../libmain/$(MACHINE)/gauden.o \
	../../libmain/$(MACHINE)/senone.o \
	../../libmain/$(MACHINE)/am.o \
	../../libmain/$(MACHINE)/mdef.o \
	../../libmain/$(MACHINE)/dict.o \
	../../libmain/$(MACHINE)/cmn.o \
	../../libmain/$(MACHINE)/agc.o \
	../../libmain/$(MACHINE)/logs3.o \
	../../libmain/$(MACHINE)/vector.o

sensprd : sensprd.o
	$(CC)  $(S3DEBUG) $(CFLAGS) -o $@ $> $(OBJS) -lfeat -lio -lutil -lm
