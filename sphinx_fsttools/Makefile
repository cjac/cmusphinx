sphinxbase_CFLAGS = $(shell pkg-config --cflags sphinxbase)
sphinxbase_LIBS = $(shell pkg-config --libs sphinxbase)

CPPFLAGS = -I/usr/local/include $(sphinxbase_CFLAGS)
CXXFLAGS = -Wno-deprecated -g -O2 -Wall
CFLAGS = -g -O2 -Wall
LIBS = -L/usr/local/lib -lfst -lpthread $(sphinxbase_LIBS)

PROGRAMS = sphinx_am_fst sphinx_dict_fst sphinx_lm_fst

ALL:: $(PROGRAMS)

sphinx_am_fst: sphinx_am_fst.o mdef.o
	$(CXX) -o $@ sphinx_am_fst.o mdef.o $(LDFLAGS) $(LIBS)

sphinx_dict_fst: sphinx_dict_fst.o mdef.o dict.o
	$(CXX) -o $@ sphinx_dict_fst.o mdef.o dict.o $(LDFLAGS) $(LIBS)

sphinx_lm_fst: sphinx_lm_fst.o
	$(CXX) -o $@ sphinx_lm_fst.o $(LDFLAGS) $(LIBS)

sphinx_dict_fst.o: sphinx_dict_fst.cc mdef.h dict.h

sphinx_am_fst.o: sphinx_am_fst.cc mdef.h

clean:
	$(RM) *.o $(PROGRAMS)