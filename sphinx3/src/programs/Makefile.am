bin_PROGRAMS = sphinx3_decode \
        sphinx3_livepretend \
        sphinx3_continuous \
        sphinx3_livedecode \
	sphinx3_align \
	sphinx3_astar \
	sphinx3_conf \
	sphinx3_dag \
	sphinx3_dp \
	sphinx3_ep \
	sphinx3_gausubvq \
	sphinx3_lm_convert \
	sphinx3_cfg2fsg

sphinx3_livepretend_SOURCES = main_livepretend.c 
sphinx3_continuous_SOURCES = main_continuous.c 
sphinx3_livedecode_SOURCES = main_livedecode.c 
sphinx3_decode_SOURCES = main_decode.c
sphinx3_align_SOURCES = main_align.c s3_align.c
sphinx3_astar_SOURCES = main_astar.c
sphinx3_conf_SOURCES = main_conf.c 
sphinx3_dag_SOURCES = main_dag.c 
sphinx3_dp_SOURCES = main_dp.c
sphinx3_ep_SOURCES = main_ep.c
sphinx3_gausubvq_SOURCES = main_gausubvq.c
sphinx3_lm_convert_SOURCES = main_lm_convert.c
sphinx3_cfg2fsg_SOURCES = main_cfg2fsg.c

noinst_HEADERS = \
	s3_align.h

LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	-lsphinxbase -lsphinxad -lm

sphinx3_livedecode_LDADD = $(top_builddir)/src/libs3decoder/libs3decoder.la \
	-lsphinxbase -lsphinxad -lm -lpthread  @ad_libs@ 

INCLUDES = -I$(top_srcdir)/include \
        -I$(top_builddir)/include
