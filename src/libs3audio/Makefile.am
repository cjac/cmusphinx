# libs3audio: Audio Device interface for Sphinx-3

lib_LTLIBRARIES = libs3audio.la

libs3audio_la_SOURCES = cont_ad_base.c

EXTRA_libs3audio_la_SOURCES = ad_base.c \
	ad_oss.c \
	ad_oss_bsd.c \
	ad_alsa.c \
	ad_osf.c \
	ad_irix.c \
	ad_sunos.c \
	audio_utils_sunos.c \
	cont_ad_base.c \
	play_win32.c \
	rec_win32.c \
	mulaw_base.c

noinst_HEADERS = audio_utils_sunos.h mulaw.h ad.h

libs3audio_la_DEPENDENCIES = @ad_files@

libs3audio_la_LIBADD = @ad_files@ @ad_libs@

libs3audio_la_LDFLAGS = -version-info 0:4:0

INCLUDES = -I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libutil \
	-I$(top_srcdir)/src/libs3decoder \
	-I$(top_builddir)/include
