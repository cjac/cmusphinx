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

AUXOBJ = ../../libmain/$(MACHINE)/mdef.o ../../libmain/$(MACHINE)/dict.o ../../libmain/$(MACHINE)/misc.o
MDEFOBJ = ../../../src/libmain/$(MACHINE)/mdef.o

lat2nodes : lat2nodes.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o lat2nodes lat2nodes.o $(AUXOBJ) -lutil -lm

cepwinvar : cepwinvar.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o cepwinvar cepwinvar.o $(AUXOBJ) -lfeat -lio -lutil -lm

cepdx : cepdx.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o cepdx cepdx.o $(AUXOBJ) -lfeat -lio -lutil -lm

mfc : mfc.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o mfc mfc.o $(AUXOBJ) -lio -lutil -lm

proncount : proncount.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o proncount proncount.o $(AUXOBJ) -lutil -lm

pron : pron.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o pron pron.o $(AUXOBJ) -lutil -lm

erreg : erreg.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o erreg erreg.o $(AUXOBJ) -lutil -lm

test : test.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o test test.o $(AUXOBJ) -lfeat -lio -lutil -lm

testvq : testvq.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o testvq testvq.o $(AUXOBJ) -lutil -lm

testvq-int32 : testvq-int32.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o testvq-int32 testvq-int32.o $(AUXOBJ) -lutil -lm

cepdist-var: cepdist-var.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o cepdist-var cepdist-var.o $(AUXOBJ) -lfeat -lio -lutil -lm

cepdist: cepdist.o
	$(CC) $(S3DEBUG) $(CFLAGS) -o cepdist cepdist.o $(AUXOBJ) -lfeat -lio -lutil -lm

dpalign: dag.o dp.o dpalign.o line2wid.o
	$(CC) $(CFLAGS) -o dpalign dpalign.o dp.o dag.o line2wid.o $(AUXOBJ) -lutil

allpalign: allpalign.o dag.o dp.o
	$(CC) $(CFLAGS) -o allpalign allpalign.o dp.o dag.o $(AUXOBJ) -lutil

pronerr: pronerr.o line2wid.o
	$(CC) $(CFLAGS) -o pronerr pronerr.o line2wid.o $(AUXOBJ) -lutil

pronerralign: pronerralign.o dp.o dag.o
	$(CC) $(CFLAGS) -o pronerralign pronerralign.o dp.o dag.o $(AUXOBJ) -lutil

segnbesterr: segnbesterr.o dp.o dag.o line2wid.o
	$(CC) $(CFLAGS) -o segnbesterr segnbesterr.o dp.o dag.o line2wid.o $(AUXOBJ) -lutil

nbesterr: nbesterr.o dp.o dag.o line2wid.o
	$(CC) $(CFLAGS) -o nbesterr nbesterr.o dp.o dag.o line2wid.o $(AUXOBJ) -lutil

v8seg2ascii: v8seg2ascii.o
	$(CC) $(CFLAGS) -o v8seg2ascii v8seg2ascii.o $(MDEFOBJ) -lio -lutil

cepdist-allp: cepdist-allp.o
	$(CC) $(CFLAGS) -o cepdist-allp cepdist-allp.o $(AUXOBJ) -lfeat -lio -lutil

newlc: newlc.o
	$(CC) $(CFLAGS) -o newlc newlc.o -lutil

lc: lc.o
	$(CC) $(CFLAGS) -o lc lc.o -lutil

newrc: newrc.o
	$(CC) $(CFLAGS) -o newrc newrc.o -lutil

rc: rc.o
	$(CC) $(CFLAGS) -o rc rc.o -lutil

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
