bin_PROGRAMS = livepretend \
	livedecode \
	decode \
	gausubvq \
	align \
	astar \
	dag \
	allphone \
	ep 

livepretend_SOURCES = main_livepretend.c 
decode_SOURCES = decode.c
decode_anytopo_SOURCES = main_decode_anytopo.c s3_dag.c

align_SOURCES = main_align.c s3_align.c
astar_SOURCES = main_astar.c s3_astar.c
dag_SOURCES = main_dag.c s3_dag.c 
allphone_SOURCES = main_allphone.c s3_allphone.c 
gausubvq_SOURCES = gausubvq.c
ep_SOURCES = main_ep.c
wave2feat_SOURCES = wave2feat.c 
cepview_SOURCES = main_cepview.c
livedecode_SOURCES = main_livedecode.c


noinst_PROGRAMS = \
	wave2feat \
	cepview \
	decode_anytopo

noinst_HEADERS = \
	main_ep.h \
	s3_align.h \
	s3_allphone.h \
	s3_dag.h

# 20041205 : recent bug makes me remove this from standard compilation for a while. 
#	decode_anytopo 


LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm @ad_libs@ 
# add these for solaris
# -L/usr/demo/SOUND/lib/ -laudio -lnsl -lsocket

INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder/libAPI/ \
	-I$(top_srcdir)/src/libs3decoder/libam/ \
	-I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
	-I$(top_srcdir)/src/libs3decoder/libcommon/ \
	-I$(top_srcdir)/src/libs3decoder/libdict/ \
	-I$(top_srcdir)/src/libs3decoder/libep/ \
	-I$(top_srcdir)/src/libs3decoder/liblm/ \
	-I$(top_srcdir)/src/libs3decoder/libsearch/ \
	-I$(top_srcdir)/src/libutil/ \
        -I$(top_srcdir)/src/libs3audio \
        -I$(top_builddir)/include

# add for solaris
# -I/usr/demo/SOUND/include
