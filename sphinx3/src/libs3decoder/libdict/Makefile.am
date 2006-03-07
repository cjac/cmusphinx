ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libdict.la

libdict_la_LDFLAGS = -version-info 0:6:0

libdict_la_SOURCES = \
	cmu6_lts_rules.c \
	ctxt_table.c \
	dict.c \
	dict2pid.c \
	lts.c \
	wid.c

pkginclude_HEADERS = \
	cmu6_lts_rules.h \
	ctxt_table.h \
	dict.h \
	dict2pid.h \
	lts.h \
	wid.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/ \
	   -I$(top_srcdir)/src/libs3decoder/libam/ \
	   -I$(top_srcdir)/src/libs3decoder/libcommon/ \
	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

