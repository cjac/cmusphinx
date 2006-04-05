ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libam.la

libam_la_LDFLAGS = -version-info 0:6:0

libam_la_SOURCES = \
	adaptor.c \
	approx_cont_mgau.c \
	cb2mllr_io.c \
	cont_mgau.c \
	fast_algo_struct.c \
	gs.c \
	hmm.c \
	interp.c \
	kdtree.c \
	mdef.c \
	mllr.c \
	ms_gauden.c \
	ms_mgau.c \
	ms_mllr.c \
	ms_senone.c \
	subvq.c \
	tmat.c \
	log_add.c \
	s2_semi_mgau.c

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

