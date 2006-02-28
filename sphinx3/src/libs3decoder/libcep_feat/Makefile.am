ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libcep_feat.la


libcep_feat_la_LDFLAGS = -version-info 0:4:0

libcep_feat_la_SOURCES = \
	agc.c \
	cmn.c \
	cmn_prior.c \
	fe.c \
	fe_interface.c \
	fe_sigproc.c \
	feat.c

pkginclude_HEADERS = \
	agc.h \
	cmn.h \
	cmn_prior.h \
	fe.h \
	fe_internal.h \
	fe_sigproc.c \
	feat.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/src/libs3decoder/libcommon \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

