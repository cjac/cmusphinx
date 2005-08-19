ARCH   = `uname -m | sed s/i.86/i386/`_`uname -s | tr A-Z a-z`

SUBDIRS = include

lib_LTLIBRARIES = libsphinx2.la

libsphinx2_la_LDFLAGS = -version-info 0:6:0

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
	bio.c \
	bisearch.c \
	bitvec.c \
	blk_cdcn_norm.c \
	blkarray_list.c \
	cache_lm.c \
	case.c \
	cdcn_init.c \
	cdcn_norm.c \
	cdcn_update.c \
	cdcn_utils.c \
	cep_rw.c \
	ckd_alloc.c \
	cont_mgau.c \
	dict.c \
	eht_quit.c \
	err.c \
	f2read.c \
	f2write.c \
	fbs_main.c \
	fsg_psubtree.c \
	fsg_lextree.c \
	fsg_history.c \
	fsg_search.c \
	get_a_word.c \
	glist.c \
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
	logs3.c \
	longio.c \
	mdef.c \
	nxtarg.c \
	norm.c \
	pconf.c \
	peek_length.c \
	phone.c \
	prime.c \
	r_agc_noise.c \
	resfft.c \
	s3hash.c \
	s3mdef_s2map.c \
	salloc.c \
	sc_cbook_r.c \
	sc_vq.c \
	search.c \
	searchlat.c \
	senscr.c \
	skipto.c \
	str2words.c \
	strcasecmp.c \
	time_align.c \
	unlimit.c \
	util.c \
	uttproc.c \
	vector.c \
	word_fsg.c


# libsphinx2_la_LIBADD = -lc -lm $(top_srcdir)/src/libsphinx2fe/libsphinx2fe.la

# hjf@cs.brown.edu - Added for Solaris build
# LDADD = -L/usr/demo/SOUND/lib/ -laudio

INCLUDES = -I$(top_srcdir)/src/libsphinx2/include \
           -I$(top_srcdir)/include \
           -I$(top_builddir)/include 

# add for solaris
#           -I/usr/demo/SOUND/include

AM_CPPFLAGS = -DFAST8B=1

