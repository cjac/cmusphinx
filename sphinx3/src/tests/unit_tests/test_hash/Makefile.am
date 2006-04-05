check_PROGRAMS = displayhash deletehash


INCLUDES = \
        -I$(top_srcdir)/include \
        -I$(top_builddir)/include

LDADD = ${top_builddir}/src/libutil/libs3util.la \
        -lm @ad_libs@

TESTS = $(wildcard _*.test) 
