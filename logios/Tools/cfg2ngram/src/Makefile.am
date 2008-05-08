bin_PROGRAMS = phoenix2cnf cnf2corpus

phoenix2cnf_SOURCES = phoenix2cnf.cpp PCFG.cpp
phoenix2cnf_CPPFLAGS = -I../include
phoenix2cnf_LDADD = -L../pcre/lib -lpcre

cnf2corpus_SOURCES = cnf2corpus.cpp PCFG.cpp
cnf2corpus_CPPFLAGS = -O3 -I../include
cnf2corpus_LDADD = -L../pcre/lib -lpcre