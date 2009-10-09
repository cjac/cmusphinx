ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = lib4glat.la

#lib4glat_la_LDFLAGS = -version-info 0:6:0

lib4glat_la_SOURCES = \
	linklist.c \
	s3_dag.c 

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

