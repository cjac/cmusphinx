check_PROGRAMS = testhmm

INCLUDES = \
        -I$(top_srcdir)/include \
        -I$(top_builddir)/include

LDADD = ${top_builddir}/src/libs3decoder/libs3decoder.la \
        -lsphinxbase @ad_libs@ -lm
AM_LDFLAGS = -static

TESTS = _testhmm_tidigits.test
