SUBDIRS = src \
	doc \
	scripts \
	model \
	include

EXTRA_DIST = autogen.sh								\
	sphinx3.pc.in								\
	src/programs/main_livedecode.c						\
	win32/batch/an4.ctl							\
	win32/batch/sphinx3-test.decode.bat					\
	win32/batch/sphinx3-test.ep.bat						\
	win32/batch/sphinx3-test.simple.bat					\
	win32/batch/sphinx3-test.all.bat					\
	win32/batch/sphinx3-test.gausubvq.bat					\
	win32/batch/sphinx3-test.align.bat					\
	win32/batch/sphinx3-test.astar.bat					\
	win32/batch/sphinx3-simple.bat						\
	win32/batch/sphinx3-test.dag.bat					\
	win32/batch/sphinx3-test.bat						\
	win32/batch/sphinx3-test.livepretend.bat				\
	win32/msdev/libs3decoder/s3decoder.vcproj				\
	win32/msdev/programs/livepretend/sphinx3_livepretend.vcproj		\
	win32/msdev/programs/decode/sphinx3_decode.vcproj			\
	win32/msdev/programs/dp/sphinx3_dp.vcproj				\
	win32/msdev/programs/livedecode/sphinx3_livedecode.vcproj		\
	win32/msdev/programs/ep/sphinx3_ep.vcproj				\
	win32/msdev/programs/conf/sphinx3_conf.vcproj				\
	win32/msdev/programs/lm_convert/lm_convert.vcproj			\
	win32/msdev/programs/align/sphinx3_align.vcproj				\
	win32/msdev/programs/astar/sphinx3_astar.vcproj				\
	win32/msdev/programs/dag/sphinx3_dag.vcproj				\
	win32/msdev/programs/gausubvq/gausubvq.vcproj				\
	win32/msdev/programs/sphinx3_continuous/sphinx3_continuous.vcproj	\
	win32/mrcp.nsi								\
	python/setup.py								\
	python/_sphinx3module.c							\
	python/_sphinx3_test.py

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sphinx3.pc
CLEANFILES = sphinx3.pc

# NO!  Don't make .tar.Z or .shar distributions
AUTOMAKE_OPTIONS = dist-bzip2 dist-zip #dist-shar dist-tarZ

etags:
	etags `find . -name "*.[ch]"`

ctags:
	ctags `find . -name "*.[ch]"`

doxygen: 
	doxygen ./doc/doxygen.cfg

#######################################################################
#Session of testing
#######################################################################


perf-std : 
	make -C $(top_srcdir)/src/tests/performance/ti46 ti46-lp ti46-decode ti46-decany
	make -C $(top_srcdir)/src/tests/performance/tidigits tidigits-lp tidigits-decode  
	make -C $(top_srcdir)/src/tests/performance/Communicator com-fast-lp com-fast-decode
	make -C $(top_srcdir)/src/tests/performance/Communicator com-2stage-decode com-2stage-decany

perf-std-s3 : 
	make -C $(top_srcdir)/src/tests/performance/ti46 ti46-decany
	make -C $(top_srcdir)/src/tests/performance/tidigits tidigits-decany
	make -C $(top_srcdir)/src/tests/performance/Communicator com-base-decany

perf-full:	
	make -C $(top_srcdir)/src/tests/performance/ti46 quick slow
	make -C $(top_srcdir)/src/tests/performance/tidigits quick slow
	make -C $(top_srcdir)/src/tests/performance/rm1 lp
	make -C $(top_srcdir)/src/tests/performance/Communicator base fast
	make -C $(top_srcdir)/src/tests/performance/wsj5k lp decode decany
	make -C $(top_srcdir)/src/tests/performance/ICSI base


#	make -C $(top_srcdir)/src/tests/performance/spoke3 decode decany

perf-quick:	
	make -C $(top_srcdir)/src/tests/performance/ti46 quick slow
	make -C $(top_srcdir)/src/tests/performance/tidigits quick

perf-slow:	
	make -C $(top_srcdir)/src/tests/performance/ti46 slow
	make -C $(top_srcdir)/src/tests/performance/tidigits slow
	make -C $(top_srcdir)/src/tests/performance/rm1 lp
	make -C $(top_srcdir)/src/tests/performance/Communicator base
	make -C $(top_srcdir)/src/tests/performance/wsj5k lp decode decany
	make -C $(top_srcdir)/src/tests/performance/ICSI base

#	make -C $(top_srcdir)/src/tests/performance/spoke3 decode decany


#Cannot not be used because libtool will confuse valgrind.  Need to figure out later. 
#test-valgrind:
#	@echo "LIVEPRETEND TEST"
#	@echo "YOU SHOULD SEE THE RECOGNITION RESULT 'P I T T S B U R G H'"
#
#	$(LIBTOOL) --mode=execute valgrind --skin=memcheck src/programs/livepretend \
#		model/lm/an4/an4.ctl \
#		$(top_srcdir)/model/lm/an4 \
#		model/lm/an4/args.an4.test > $@.out 2>&1 
#	@grep "FWDVIT" $@.out 
#	@grep "FWDXCT" $@.out 

