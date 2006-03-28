check_PROGRAMS = chgCase


INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libutil \
	-I$(top_builddir)/include

LDADD = ${top_builddir}/src/libutil/libs3util.la \
	-lm @ad_libs@


TESTS = $(wildcard _*.test) 


