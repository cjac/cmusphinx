#
# Makefile
# 
# HISTORY
# 
# 05-Mar-99	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


VPATH = .:..

include ../../Makefile.defines

gausubvq: gausubvq.c
	$(CC) $(CFLAGS) -o gausubvq.beta gausubvq.c $(OBJS) -lmain -lmisc -lfeat -lio -lutil -lm

gautest: gautest.c subvq.c
	$(CC) $(CFLAGS) -o gautest.beta gautest.c subvq.c $(OBJS) -lmain -lmisc -lfeat -lio -lutil -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
