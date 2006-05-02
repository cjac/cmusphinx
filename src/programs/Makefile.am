bin_PROGRAMS = sphinx3_decode \
	       sphinx3_livepretend \
	       sphinx3_continuous \
	       sphinx3_livedecode \
	       sphinx3_decode_anytopo \
	sphinx3_align \
	sphinx3_allphone \
	sphinx3_astar \
	sphinx3_conf \
	sphinx3_dag \
	sphinx3_dp \
	sphinx3_ep \
	gausubvq \
	wave2feat \
	cepview \
	lm_convert

sphinx3_livepretend_SOURCES = main_livepretend.c 
sphinx3_continuous_SOURCES = main_continuous.c 
sphinx3_livedecode_SOURCES = main_livedecode.c 
sphinx3_decode_SOURCES = decode.c
sphinx3_decode_anytopo_SOURCES = main_decode_anytopo.c 
sphinx3_align_SOURCES = main_align.c s3_align.c
sphinx3_astar_SOURCES = main_astar.c s3_astar.c
sphinx3_conf_SOURCES = main_conf.c 
sphinx3_dag_SOURCES = main_dag.c 
sphinx3_dp_SOURCES = main_dp.c
sphinx3_allphone_SOURCES = main_allphone.c s3_allphone.c 
sphinx3_ep_SOURCES = main_ep.c
gausubvq_SOURCES = gausubvq.c
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
	$(top_builddir)/src/libutil/libs3util.la \
	-lm  @ad_libs@ 

sphinx3_livedecode_LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	$(top_builddir)/src/libs3audio/libs3audio.la \
	$(top_builddir)/src/libutil/libs3util.la \
	-lm -lpthread  @ad_libs@ 

#	-lm -ldmalloc @ad_libs@ 
#	-lm -ldmalloc @ad_libs@ 
#

# add these for solaris
# -L/usr/demo/SOUND/lib/ -laudio -lnsl -lsocket

INCLUDES = -I$(top_srcdir)/include \
        -I$(top_builddir)/src/libs3audio \
        -I$(top_builddir)/include

# add for solaris
# -I/usr/demo/SOUND/include
