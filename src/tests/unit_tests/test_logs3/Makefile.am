check_PROGRAMS = testlogs3


INCLUDES = \
        -I$(top_srcdir)/include \
        -I$(top_builddir)/include

LDADD = ${top_builddir}/src/libutil/libs3util.la \
	${top_builddir}/src/libs3decoder/libs3decoder.la \
        -lm @ad_libs@

TESTS = $(wildcard _*.test) 
