ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libep.la


libep_la_LDFLAGS = -version-info 0:6:0

libep_la_SOURCES = \
	classify.c \
	endptr.c

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

