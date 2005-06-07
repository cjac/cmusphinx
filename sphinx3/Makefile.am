SUBDIRS = src \
	doc \
	scripts \
	model

EXTRA_DIST = autogen.sh \
	programs.dsw \
	src/programs/main_livedecode.c \
	win32/batch/an4.ctl \
	win32/batch/sphinx3-simple.bat \
	win32/batch/sphinx3-test.align.bat \
	win32/batch/sphinx3-test.all.bat \
	win32/batch/sphinx3-test.allphone.bat \
	win32/batch/sphinx3-test.astar.bat \
	win32/batch/sphinx3-test.bat \
	win32/batch/sphinx3-test.dag.bat \
	win32/batch/sphinx3-test.decode.bat \
	win32/batch/sphinx3-test.ep.bat \
	win32/batch/sphinx3-test.gausubvq.bat \
	win32/batch/sphinx3-test.livepretend.bat \
	win32/batch/sphinx3-test.simple.bat \
	win32/msdev/libs3audio/libs3audio.dsp \
	win32/msdev/libs3decoder/libs3decoder.dsp \
	win32/msdev/libutil/libutil.dsp \
	win32/msdev/programs/align/align.dsp \
	win32/msdev/programs/allphone/allphone.dsp \
	win32/msdev/programs/astar/astar.dsp \
	win32/msdev/programs/dag/dag.dsp \
	win32/msdev/programs/decode/allphone.dsp \
	win32/msdev/programs/decode/decode.dsp \
	win32/msdev/programs/ep/ep.dsp \
	win32/msdev/programs/gausubvq/gausubvq.dsp \
	win32/msdev/programs/livedecode/livedecode.dsp \
	win32/msdev/programs/livepretend/livepretend.dsp

# Add an item for each and every test target to the CLEANFILES variable
CLEANFILES = chan3.mfc \
	test.out \
	test-*.out \
	tmp.out \
	pittsburgh.* 

etags:
	etags `find . -name "*.[ch]"`

ctags:
	ctags `find . -name "*.[ch]"`

doxygen: 
	doxygen ./doc/doxygen.cfg

#######################################################################
#Session of testing
#######################################################################


# Override the Automake target so that make test-full gets run during
# distcheck
check: test-full
	@:

test-full : test-decode test-align test-allphone test-livepretend test-astar test-dag test-gausubvq test-ep test-cepview test-decode_anytopo test-mllr test-mllr-lp test-mllr-anytopo
test-all-app : test-decode test-align test-allphone test-livepretend test-astar test-dag test-gausubvq test-ep test-wave2feat test-cepview test-mllr test-mllr-lp test-mllr-anytopo


test-decode_anytopo \
test-decode \
test-mllr \
test-align \
test-allphone \
test-astar \
test-cepview \
test-dag \
test-livepretend \
test-mllr-lp \
test-mllr-anytopo \
test-gausubvq \
test-ep \
test-wave2feat:
	sh $(top_builddir)/src/tests/regression/$@.sh


perf-std : 
	make -C $(top_srcdir)/src/tests/performance/ti46 ti46-lp ti46-decode 
	make -C $(top_srcdir)/src/tests/performance/tidigits tidigits-lp tidigits-decode 
	make -C $(top_srcdir)/src/tests/performance/Communicator com-fast-lp com-fast-decode

perf-std-s3 : 
	make -C $(top_srcdir)/src/tests/performance/ti46 ti46-decany
	make -C $(top_srcdir)/src/tests/performance/tidigits tidigits-decany
	make -C $(top_srcdir)/src/tests/performance/Communicator com-base-decany

perf-full:	
	make -C $(top_srcdir)/src/tests/performance/ti46 quick slow
	make -C $(top_srcdir)/src/tests/performance/tidigits quick slow
	make -C $(top_srcdir)/src/tests/performance/rm1 lp
	make -C $(top_srcdir)/src/tests/performance/Communicator base fast
	make -C $(top_srcdir)/src/tests/performance/wsj5k quick slow

perf-quick:	
	make -C $(top_srcdir)/src/tests/performance/ti46 quick slow
	make -C $(top_srcdir)/src/tests/performance/tidigits quick
	make -C $(top_srcdir)/src/tests/performance/Communicator fast
	make -C $(top_srcdir)/src/tests/performance/wsj5k quick

perf-slow:	
	make -C $(top_srcdir)/src/tests/performance/ti46 slow
	make -C $(top_srcdir)/src/tests/performance/tidigits slow
	make -C $(top_srcdir)/src/tests/performance/rm1 lp
	make -C $(top_srcdir)/src/tests/performance/Communicator base
	make -C $(top_srcdir)/src/tests/performance/wsj5k slow




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
