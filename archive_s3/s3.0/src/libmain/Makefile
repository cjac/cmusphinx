#
# Makefile
# 
# HISTORY
# 
# 07-Jan-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


include ../../../Makefile.defines

VPATH = .:..

TARGET = libmain.a

OBJS =  agc.o \
	am.o \
	cmn.o \
	dict.o \
	fillpen.o \
	gauden.o \
	hmm.o \
	hyp.o \
	interp.o \
	kbcore.o \
	lm.o \
	mdef.o \
	senone.o \
	tmat.o \
	vithist.o \
	wid.o \
	wnet2pnet.o

$(TARGET) : $(OBJS)
	ar crv $@ $?
	ranlib $@

install: $(TARGET)
	cp $(TARGET) $(S3LIBDIR)

segnbest2hmmnet : segnbest2hmmnet.o \
	hmm.o \
	wid.o \
	mdef.o \
	dict.o \
	gauden.o \
	senone.o \
	tmat.o \
	lm.o \
	vit_hist.o \
	fillpen.o \
	cmn.o \
	agc.o \
	$(CC) $(S3DEBUG) $(CFLAGS) -o $@ $> -lmisc -lfeat -lio -lutil -lm

interp : interp.c
	$(CC) $(S3DEBUG) -D_INTERP_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lio -lutil -lm
	rm interp.o

tmat : tmat.c
	$(CC) $(S3DEBUG) -D_TMAT_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lio -lutil -lm
	rm tmat.o

gauden : gauden.c cmn.o agc.o
	$(CC) $(S3DEBUG) -D_GAUDEN_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lfeat -lio -lutil -lm
	rm gauden.o

sen2s2 : sen2s2.c mdef.o
	$(CC) $(S3DEBUG) -D_SENONE_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lfeat -lio -lutil -lm

senone : senone.c gauden.o cmn.o agc.o
	$(CC) $(S3DEBUG) -D_SENONE_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lfeat -lio -lutil -lm
	rm senone.o

mdef : mdef.c
	$(CC) $(S3DEBUG) -D_MDEF_TEST_=1 $(CFLAGS) -o $@ $> -lutil
	rm mdef.o

dict : dict.c mdef.o
	$(CC) $(S3DEBUG) -D_DICT_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lutil
	rm dict.o

lm : lm.c
	$(CC) $(S3DEBUG) -D_LM_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lutil -lm
	rm lm.o

kbcore : kbcore.c mdef.o dict.o lm.o fillpen.o wid.o gauden.o senone.o am.o tmat.o
	$(CC) $(S3DEBUG) -D_KBCORE_TEST_=1 $(CFLAGS) -o $@ $> -lmisc -lfeat -lio -lutil -lm
	rm kbcore.o

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
