#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


include $(S3DIR)/Makefile.defines
VPATH = .:..

# Decoder
DECOBJS =	agc.o \
		ascr.o \
		beam.o \
		bio.o \
		cmn.o \
		cont_mgau.o \
		corpus.o \
		dict.o \
		dict2pid.o \
		feat.o \
		fillpen.o \
		hmm.o \
		kbcore.o \
		lextree.o \
		lm.o \
		logs3.o \
		main.o \
		mdef.o \
		subvq.o \
		tmat.o \
		vector.o \
		vithist.o \
		wid.o

install:
	(cd $(UTILDIR); $(MAKE) install)
	mkdir -p $(S3BINDIR) $(MACHINE)
	(cd $(MACHINE); $(MAKE) $(MFLAGS) -f ../Makefile decode)
	(cd $(MACHINE); $(MAKE) $(MFLAGS) -f ../Makefile gausubvq)
	(cd $(MACHINE); $(MAKE) $(MFLAGS) -f ../Makefile lmtest)
	cp $(MACHINE)/decode $(S3BINDIR)
	cp $(MACHINE)/gausubvq $(S3BINDIR)
	cp $(MACHINE)/lmtest $(S3BINDIR)

decode: $(DECOBJS)
	$(CC) $(CFLAGS) -o $@ $(DECOBJS) -lutil -lm


# Acoustic model Sub-vector quantizer
GAUSUBVQOBJS =	gausubvq.o \
		cont_mgau.o \
		subvq.o \
		vector.o \
		bio.o \
		logs3.o

# Build sub-vector quantized acoustic model for faster Gaussian density evaluation
gausubvq: $(GAUSUBVQOBJS)
	$(CC) $(CFLAGS) -o $@ $(GAUSUBVQOBJS) -lutil -lm


# Acoustic model Sub-vector quantizer
LMTESTOBJS =	lmtest.o \
		lm.o \
		logs3.o

# Build sub-vector quantized acoustic model for faster Gaussian density evaluation
lmtest: $(LMTESTOBJS)
	$(CC) $(CFLAGS) -o $@ $(LMTESTOBJS) -lutil -lm


clean:
	(cd $(UTILDIR); $(MAKE) clean)
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
	(cd $(MACHINE); rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~)
