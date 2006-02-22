ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libcommon.la

libcommon_la_LDFLAGS = -version-info 0:4:0

libcommon_la_SOURCES = \
	bio.c \
	corpus.c \
	encoding.c \
	logs3.c \
	misc.c \
	stat.c \
	vector.c 

pkginclude_HEADERS = \
	bio.h \
	corpus.h \
	encoding.h \
	logs3.h \
	misc.h \
	stat.h \
	vector.h \
	s3types.h \
	cmdln_macro.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/src/libs3decoder/libsearch/ \
	   -I$(top_srcdir)/src/libs3decoder/libam/ \
	   -I$(top_srcdir)/src/libs3decoder/libdict/ \
	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
	   -I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
	   -I$(top_srcdir)/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

