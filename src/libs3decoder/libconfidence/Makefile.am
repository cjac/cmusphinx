ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libconfidence.la

libconfidence_la_LDFLAGS = -version-info 0:6:0

libconfidence_la_SOURCES = \
	confidence.c 

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

