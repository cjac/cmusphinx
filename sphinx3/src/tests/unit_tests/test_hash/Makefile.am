check_PROGRAMS = displayhash deletehash


INCLUDES = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/src \
        -I$(top_srcdir)/src/libutil \
        -I$(top_builddir)/include

LDADD = ${top_builddir}/src/libutil/libutil.la \
        -lm @ad_libs@

TESTS = $(wildcard _*.test) 
