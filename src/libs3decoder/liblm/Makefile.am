ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = liblm.la


liblm_la_LDFLAGS = -version-info 0:4:0

liblm_la_SOURCES = \
	fillpen.c \
        lm.c \
	lm_3g.c \
	lm_3g_dmp.c \
	lm_debug.c \
	lm_attfsm.c \
	lmclass.c \
	lmset.c



pkginclude_HEADERS = \
	fillpen.h \
        lm.h \
	lmclass.h 

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src \
	   -I$(top_srcdir)/src/libutil/ \
	   -I$(top_srcdir)/src/libs3decoder/libam \
	   -I$(top_srcdir)/src/libs3decoder/libdict \
	   -I$(top_srcdir)/src/libs3decoder/libcommon \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

