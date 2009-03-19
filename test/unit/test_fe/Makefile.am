check_PROGRAMS = test_fe test_pitch test_pncc

TESTS = test_fe test_pitch test_pncc
INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-DDATADIR=\"$(top_srcdir)/test/regression\"

noinst_HEADERS = test_macros.h

LDADD = ${top_builddir}/src/libsphinxbase/libsphinxbase.la
