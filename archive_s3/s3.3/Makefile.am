SUBDIRS = src \
	doc \
	model \
	scripts

EXTRA_DIST = autogen.sh 

test:
	$(LIBTOOL) --mode=execute src/programs/livepretend \
		$(top_srcdir)/model/lm/an4/an4.ctl \
		$(top_srcdir)/model/lm/an4 \
		$(top_srcdir)/model/lm/an4/args.an4.test
