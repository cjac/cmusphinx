ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`


noinst_LTLIBRARIES = libam.la

libam_la_LDFLAGS = -version-info 0:4:0

libam_la_SOURCES = \
	adaptor.c \
	approx_cont_mgau.c \
	cont_mgau.c \
	fast_algo_struct.c \
	gs.c \
	hmm.c \
	interp.c \
	mdef.c \
	mllr.c \
	ms_gauden.c \
	cb2mllr_io.c \
	ms_mllr.c \
	ms_senone.c \
	subvq.c \
	tmat.c

pkginclude_HEADERS = \
	adaptor.h \
	approx_cont_mgau.h \
	cont_mgau.h \
	fast_algo_struct.h \
	gs.h \
	hmm.h \
	interp.h \
	mdef.h \
	mllr.h \
	ms_gauden.h \
	cb2mllr_io.h \
	ms_mllr.h \
	ms_senone.h \
	subvq.h \
	tmat.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/ \
	   -I$(top_srcdir)/src/libs3decoder/libam/ \
	   -I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
	   -I$(top_srcdir)/src/libs3decoder/libdict/ \
	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
	   -I$(top_srcdir)/src/libs3decoder/libcommon/ \
	   -I$(top_srcdir)/src/libs3decoder/libsearch/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

