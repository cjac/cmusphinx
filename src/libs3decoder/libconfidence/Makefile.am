ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libconfidence.la

libconfidence_la_LDFLAGS = -version-info 0:6:0

libconfidence_la_SOURCES = \
	confidence.c 

pkginclude_HEADERS = \
	confidence.h 

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/src/libs3decoder/libcommon/ \
	   -I$(top_srcdir)/src/libs3decoder/libam/ \
	   -I$(top_srcdir)/src/libs3decoder/libdict/ \
	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
	   -I$(top_srcdir)/src/libs3decoder/libsearch/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

