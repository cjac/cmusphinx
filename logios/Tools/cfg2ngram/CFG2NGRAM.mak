INCLUDEDIR=-I/usr/local/include -I/root/spooles.2.2
CPPFLAGS=$(INCLUDEDIR)
LIBDIRS=-L/root/spooles.2.2
LIBS=-lpcre #-lspooles -lBridge
SLMDIR=/afs/cs.cmu.edu/user/tkharris/projects/usi/usi/slm/CMU-Cam_Toolkit_v2/bin
#DEBUG=-gstabs+
N=2
MASS=0.5
CFLAGS=$(DEBUG) -O3

%.arpa: %.idngram %.vocab
	$(SLMDIR)/idngram2lm -idngram $< -vocab $*.vocab -arpa $@ -context tmp.ccs -vocab_type 0 -n $(N)

%.idngram: %.wngram %.vocab
	$(SLMDIR)/wngram2idngram -vocab $*.vocab -n $(N) < $< > $@

%.vocab: %.gra makevocab
	./makevocab Utt < $< | LC_ALL=C sort > $@

makevocab: makevocab.o PCFG.o
	g++ $(CFLAGS) $^ -o $@ $(LIBDIRS) $(LIBS)

%.wngram: %.smooth.gra cfg2wngram
	./cfg2wngram $(N) < $< | LC_ALL=C sort > $@

cfg2wngram: cfg2wngram.o NGram.o PCFG.o /root/spooles.2.2/libBridge.a /root/spooles.2.2/libspooles.a
	g++ $(CFLAGS) $^ -o $@ $(LIBDIRS) $(LIBS)

%.smooth.gra: %.train.gra smooth
	./smooth $(MASS) < $< > $@

smooth: smooth.o PCFG.o
	g++ $(CFLAGS) $^ -o $@ $(LIBDIRS) $(LIBS)

%.train.gra: %.cnf.gra %.data train
	./train $*.data < $< > $@

train: train.o PCFG.o
	g++ $(CFLAGS) $^ -o $@ $(LIBDIRS) $(LIBS)

%.cnf.gra: %.gra phoenix2cnf
	./phoenix2cnf Utt < $< > $@

phoenix2cnf: phoenix2cnf.o PCFG.o
	g++ $(CFLAGS) $^ -o $@ $(LIBDIRS) $(LIBS)

%.o: %.cpp
	g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f *.o makevocab cfg2wngram smooth train phoenix2cnf

%.clean:
	rm -f $*.arpa $*.idngram $*.vocab $*.wngram $*.smooth.gra $*.train.gra $*.cnf.gra

.PRECIOUS:%.arpa %.idngram %.vocab %.wngram %.smooth.gra %.train.gra %.cnf.gra
.DELETE_ON_ERROR: