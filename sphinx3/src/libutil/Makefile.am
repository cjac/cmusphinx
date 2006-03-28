ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

lib_LTLIBRARIES = libs3util.la

libs3util_la_LDFLAGS = -version-info 0:4:0

libs3util_la_SOURCES = \
        bitvec.c \
        case.c \
        ckd_alloc.c \
        cmd_ln.c \
        err.c \
        filename.c \
	genrand.c \
        glist.c \
        hash.c \
        heap.c \
        pio.c \
	info.c \
	linklist.c \
        profile.c \
	sort.c \
	nextword.c \
        str2words.c \
        unlimit.c

pkginclude_HEADERS = \
        bitvec.h \
        case.h \
        ckd_alloc.h \
        cmd_ln.h \
        err.h \
        filename.h \
	genrand.h \
        glist.h \
        hash.h \
        heap.h \
        pio.h \
	info.h \
	linklist.h \
        libutil.h \
        prim_type.h \
        profile.h \
	sort.h \
	nextword.h \
        str2words.h \
        unlimit.h

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

LIBOBJS = @LIBOBJS@
