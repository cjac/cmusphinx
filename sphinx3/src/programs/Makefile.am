bin_PROGRAMS = decode \
	       livepretend \
	       livedecode \
	       decode_anytopo \
	gausubvq \
	align \
	allphone \
	astar \
	conf \
	dag \
	dp \
	ep \
	lm_convert \
	cepview \
	wave2feat 


livepretend_SOURCES = main_livepretend.c 
livedecode_SOURCES = main_livedecode.c 
decode_SOURCES = decode.c
decode_anytopo_SOURCES = main_decode_anytopo.c 
align_SOURCES = main_align.c s3_align.c
astar_SOURCES = main_astar.c s3_astar.c
conf_SOURCES = main_conf.c 
dag_SOURCES = main_dag.c 
dp_SOURCES = main_dp.c
allphone_SOURCES = main_allphone.c s3_allphone.c 
gausubvq_SOURCES = gausubvq.c
ep_SOURCES = main_ep.c
cepview_SOURCES = main_cepview.c
lm_convert_SOURCES = main_lm_convert.c

#wave2feat_SOURCES = wave2feat.c 
#gauvq_read_SOURCES = main_gauvq_read.c 
#gauvq_SOURCES = main_gauvq.c 



#noinst_PROGRAMS = 

noinst_HEADERS = \
	main_ep.h \
	s3_align.h \
	s3_allphone.h 

LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm  @ad_libs@ 

livedecode_LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libutil.la \
	-lm -lpthread  @ad_libs@ 

#	-lm -ldmalloc @ad_libs@ 
#	-lm -ldmalloc @ad_libs@ 
#

# add these for solaris
# -L/usr/demo/SOUND/lib/ -laudio -lnsl -lsocket

INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/libs3decoder/libAPI/ \
	-I$(top_srcdir)/src/libs3decoder/libam/ \
	-I$(top_srcdir)/src/libs3decoder/libcep_feat/ \
	-I$(top_srcdir)/src/libs3decoder/libcommon/ \
	-I$(top_srcdir)/src/libs3decoder/libconfidence/ \
	-I$(top_srcdir)/src/libs3decoder/libdict/ \
	-I$(top_srcdir)/src/libs3decoder/libdp/ \
	-I$(top_srcdir)/src/libs3decoder/libep/ \
	-I$(top_srcdir)/src/libs3decoder/liblm/ \
	-I$(top_srcdir)/src/libs3decoder/libsearch/ \
	-I$(top_srcdir)/src/libutil/ \
        -I$(top_srcdir)/src/libs3audio \
        -I$(top_builddir)/include

# add for solaris
# -I/usr/demo/SOUND/include
