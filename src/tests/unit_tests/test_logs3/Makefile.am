check_PROGRAMS = testlogs3


INCLUDES = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/src \
        -I$(top_srcdir)/src/libutil \
        -I$(top_srcdir)/src/libs3decoder \
        -I$(top_srcdir)/src/libs3decoder/liblm/ \
        -I$(top_srcdir)/src/libs3decoder/libcommon/ \
        -I$(top_builddir)/include

LDADD = ${top_builddir}/src/libutil/libutil.la \
	${top_builddir}/src/libs3decoder/libs3decoder.la \
        -lm @ad_libs@

TESTS = $(wildcard _*.test) 
