SUBDIRS = pcfgsrc

bin_PROGRAMS = phoenix2pcfg pcfg2corpus phoenix2corpus

LDADD = pcfgsrc/libPCFG.a /usr/lib/libpcre.a
AM_CXXFLAGS = -DPCRE_STATIC

phoenix2pcfg_SOURCES = phoenix2pcfg.cpp
phoenix2corpus_SOURCES = phoenix2corpus.cpp
pcfg2corpus_SOURCES = pcfg2corpus.cpp
