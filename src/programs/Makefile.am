bin_PROGRAMS = livedecode\
	livepretend \
	decode \
	gausubvq \
	align 

livedecode_SOURCES =  main_live_example.c \
	parse_args_file.c \
	live.c

livepretend_SOURCES = main_live_pretend.c \
	parse_args_file.c \
	live.c

decode_SOURCES = decode.c \
	parse_args_file.c

align_SOURCES = main_align.c

gausubvq_SOURCES = gausubvq.c

noinst_HEADERS = \
	cmd_ln_args.h \
	live.h 

LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 
# add these for solaris
# -L/usr/demo/SOUND/lib/ -laudio -lnsl -lsocket

INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include

# add for solaris
# -I/usr/demo/SOUND/include
