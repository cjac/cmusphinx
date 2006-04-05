ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libcommon.la

libcommon_la_LDFLAGS = -version-info 0:6:0

libcommon_la_SOURCES = \
	bio.c \
	corpus.c \
	encoding.c \
	logs3.c \
	misc.c \
	stat.c \
	vector.c 

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

