ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

SUBDIRS = include

lib_LTLIBRARIES = libsphinx2.la

libsphinx2_la_SOURCES = \
	CM_funcs.c \
	add-table.c \
	agc_emax.c \
	allphone.c \
	areadchar.c \
	areaddouble.c \
	areadfloat.c \
	areadint.c \
	areadshort.c \
	awritechar.c \
	awritedouble.c \
	awritefloat.c \
	awriteint.c \
	awriteshort.c \
	bisearch.c \
	blk_cdcn_norm.c \
	cache_lm.c \
	cdcn_init.c \
	cdcn_norm.c \
	cdcn_update.c \
	cep_rw.c \
	dict.c \
	eht_quit.c \
	err.c \
	fbs_main.c \
	f2read.c \
	f2write.c \
	get_a_word.c \
	hash.c \
	hmm_tied_r.c \
	kb_main.c \
	lab.c \
	linklist.c \
	list.c \
	live_norm.c \
	lm.c \
	lm_3g.c \
	lmclass.c \
	logmsg.c \
	longio.c \
	nxtarg.c \
	norm.c \
	pconf.c \
	peek_length.c \
	phone.c \
	prime.c \
	r_agc_noise.c \
	resfft.c \
	salloc.c \
	sc_cbook_r.c \
	sc_vq.c \
	search.c \
	searchlat.c \
	skipto.c \
	str2words.c \
	strcasecmp.c \
	time_align.c \
	unlimit.c \
	util.c \
	uttproc.c

libsphinx2_la_LDFLAGS = -version-info 1:0:1

# libsphinx2_la_LIBADD = -lc -lm $(top_srcdir)/src/libsphinx2fe/libsphinx2fe.la

# hjf@cs.brown.edu - Added for Solaris build
# LDADD = -L/usr/demo/SOUND/lib/ -laudio

INCLUDES = -I$(top_srcdir)/src/libsphinx2/include \
           -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

# add for solaris
#           -I/usr/demo/SOUND/include

CPPFLAGS = -DFAST8B=1

LIBOBJS = @LIBOBJS@
