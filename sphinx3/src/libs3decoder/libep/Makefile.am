ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libep.la


libep_la_LDFLAGS = -version-info 0:6:0

libep_la_SOURCES = \
	classify.c \
	endptr.c

pkginclude_HEADERS = \
	classify.h \
	endptr.h


INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/src/libs3decoder/libam \
	   -I$(top_srcdir)/src/libs3decoder/libcep_feat \
	   -I$(top_srcdir)/src/libs3decoder/libcommon \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

