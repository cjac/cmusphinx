# libsphinx2fe: Feature Extraction interface for Sphinx-II

lib_LTLIBRARIES = libsphinx2fe.la

libsphinx2fe_la_LDFLAGS = -version-info 0:6:0

libsphinx2fe_la_SOURCES = \
	fe_interface.c \
	fe_sigproc.c

noinst_HEADERS = fe_internal.h

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include
