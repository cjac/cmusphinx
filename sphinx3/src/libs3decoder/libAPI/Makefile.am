ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

noinst_LTLIBRARIES = libAPI.la

libAPI_la_LDFLAGS = -version-info 0:6:0

libAPI_la_SOURCES = \
	live_decode_API.c \
	live_decode_args.c \
	utt.c 

INCLUDES = -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@

