bin_PROGRAMS = livepretend \
	decode \
	gausubvq \
	align \
	astar \
	dag \
	allphone \
	ep

livepretend_SOURCES = main_livepretend.c
decode_SOURCES = decode.c

#decode_anytopo_SOURCES = main_decode_anytopo.c  s3_dag.

align_SOURCES = main_align.c s3_align.c
astar_SOURCES = main_astar.c s3_astar.c
dag_SOURCES = main_dag.c s3_dag.c 
allphone_SOURCES = main_allphone.c s3_allphone.c 
gausubvq_SOURCES = gausubvq.c
ep_SOURCES = main_ep.c
wave2feat_SOURCES = wave2feat.c 
cepview_SOURCES = main_cepview.c

noinst_PROGRAMS = \
	wave2feat \
	cepview

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
	-I$(top_srcdir)/src/libs3decoder \
       -I$(top_srcdir)/src/libs3audio \
       -I$(top_builddir)/include

# add for solaris
# -I/usr/demo/SOUND/include
