SUBDIRS = pcfgsrc

bin_PROGRAMS = phoenix2pcfg pcfg2corpus phoenix2corpus

LDADD = -lpcre pcfgsrc/libPCFG.a

phoenix2pcfg_SOURCES = phoenix2pcfg.cpp

phoenix2corpus_SOURCES = phoenix2corpus.cpp

#phoenix2cnf_SOURCES = phoenix2cnf.cpp PCFG.cpp
#phoenix2cnf_CPPFLAGS = -gstabs+
#phoenix2cnf_LDADD = -lpcre

pcfg2corpus_SOURCES = pcfg2corpus.cpp
