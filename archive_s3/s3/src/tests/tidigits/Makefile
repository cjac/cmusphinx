# ====================================================================
#
#  Sphinx III
#
# ====================================================================
TOP=`(cd ../../..; pwd)`
DIRNAME=src/tests
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
# H = 
# LIVEDECSRCS = 
# MAINSRCS = 
# OTHERSRCS = main.c
# LIVEDECOBJS = $(LIVEDECSRCS:.c=.o) $(BASESRCS:.c=.o) 
FILES = Makefile
LIBNAME= tests
BINDIR = $(TOP)/bin/$(MACHINE)

tidigits:
	rm -f gmake-tidigits.results
	$(BINDIR)/s3decode-anytopo ARGS.tidigits > gmake-tidigits.results
	$(BINDIR)/align -ref tidigits.ref -hyp tidigits.match > tidigits.align

