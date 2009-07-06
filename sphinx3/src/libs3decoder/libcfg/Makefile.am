ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libcfg.la

#libcfg_la_LDFLAGS = -version-info 0:6:0

libcfg_la_SOURCES = \
	s3_cfg.c \
	s3_arraylist.c \
	s3_cfg_convert.c

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@
