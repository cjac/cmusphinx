SUBDIRS = pcfgsrc

bin_PROGRAMS = phoenix2pcfg pcfg2corpus phoenix2corpus

LDADD = pcfgsrc/libPCFG.a /usr/lib/libpcre.a

# you must link to the actual MinGW libraries, not the Cygwin versions
# change the following paths according to your installation
AM_CCFLAGS = "-DPCRE_STATIC -I/cygdrive/e/MinGW/include $(CCFLAGS)"
AM_LDFLAGS="-L/cygdrive/e/MinGW/lib -L/cygdrive/e/MinGW/lib/gcc/mingw32/3.4.5 $(LDFLAGS)"

phoenix2pcfg_SOURCES = phoenix2pcfg.cpp
phoenix2corpus_SOURCES = phoenix2corpus.cpp
pcfg2corpus_SOURCES = pcfg2corpus.cpp
