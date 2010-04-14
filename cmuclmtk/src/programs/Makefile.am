bin_PROGRAMS = \
	text2wfreq wfreq2vocab text2idngram idngram2lm  \
	binlm2arpa idngram2stats evallm text2wngram ngram2mgram  \
        wngram2idngram mergeidngram lm_interpolate lm_combine 

noinst_HEADERS = idngram.h

INCLUDES = -I${top_srcdir}/src/libs -I${top_srcdir}/src/liblmest -I${top_srcdir}/src/win32
LDADD = $(top_builddir)/src/libcmucuslmtk.la
