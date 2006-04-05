ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = liblm.la


liblm_la_LDFLAGS = -version-info 0:6:0

liblm_la_SOURCES = \
	fillpen.c \
        lm.c \
	lm_3g.c \
	lm_3g_dmp.c \
	lm_debug.c \
	lm_attfsm.c \
	lmclass.c \
	lmset.c

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

