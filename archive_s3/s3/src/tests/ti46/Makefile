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

ti46:
	rm -f gmake-ti46.results
	$(BINDIR)/s3decode-anytopo ARGS.ti46 > gmake-ti46.results
	$(BINDIR)/align -ref ti46.ref -hyp ti46.match > ti46.align

