include ../../Makefile.defines

VPATH = .:..

makedict : makedict.c
	$(CC)  $(S3DEBUG) $(CFLAGS) -o $@ $> ../../../src/libmain/$(MACHINE)/dict.o ../../../src/libmain/$(MACHINE)/mdef.o -lmisc -lutil

clean :
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#
