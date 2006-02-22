ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libdict.la

libdict_la_LDFLAGS = -version-info 0:4:0

libdict_la_SOURCES = \
	dict2pid.c \
	dict.c \
	wid.c \
	lts.c \
	ctxt_table.c \
	cmu6_lts_rules.c

pkginclude_HEADERS = \
	dict2pid.h \
	dict.h \
	wid.h \
	lts.h \
	ctxt_table.h \
	cmu6_lts_rules.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/ \
	   -I$(top_srcdir)/src/libs3decoder/libam/ \
	   -I$(top_srcdir)/src/libs3decoder/libcommon/ \
	   -I$(top_srcdir)/src/libs3decoder/liblm/ \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

