SUBDIRS = src \
	doc \
	scripts

EXTRA_DIST = autogen.sh \
	win32/batch/an4.ctl \
	win32/batch/sphinx3-simple.bat \
	win32/batch/sphinx3-test.bat \
	win32/msdev/libs3audio/libs3audio.dsp \
	win32/msdev/libs3decoder/libs3decoder.dsp \
	win32/msdev/libutil/libutil.dsp \
	win32/msdev/programs/gausubvq/gausubvq.dsp \
	win32/msdev/programs/programs.dsw \
	win32/msdev/programs/livedecode/livedecode.dsp \
	win32/msdev/programs/livepretend/livepretend.dsp

test:
	$(LIBTOOL) --mode=execute src/programs/livepretend \
		$(top_srcdir)/model/lm/an4/an4.ctl \
		$(top_srcdir)/model/lm/an4 \
		$(top_srcdir)/model/lm/an4/args.an4.test
