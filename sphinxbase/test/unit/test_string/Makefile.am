check_PROGRAMS = strtest test_atof

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-DDATADIR=\"$(srcdir)\"

LDADD = ${top_builddir}/src/libsphinxbase/libsphinxbase.la


TESTS = $(srcdir)/_*.test test_atof

EXTRA_DIST = $(srcdir)/_*.test _fread_line.txt
