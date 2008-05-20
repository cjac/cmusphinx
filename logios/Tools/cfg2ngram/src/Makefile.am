bin_PROGRAMS = phoenix2pcfg phoenix2cnf cnf2corpus

phoenix2pcfg_SOURCES = phoenix2pcfg.cpp PCFG.cpp debug.cpp
phoenix2pcfg_CPPFLAGS = -gstabs+
phoenix2pcfg_LDADD = -lpcre

phoenix2cnf_SOURCES = phoenix2cnf.cpp PCFG.cpp
phoenix2cnf_CPPFLAGS = -gstabs+
phoenix2cnf_LDADD = -lpcre

cnf2corpus_SOURCES = cnf2corpus.cpp PCFG.cpp
cnf2corpus_CPPFLAGS = -O3
cnf2corpus_LDADD = -lpcre