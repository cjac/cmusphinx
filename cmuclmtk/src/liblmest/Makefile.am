noinst_LTLIBRARIES = liblmest.la
liblmest_la_SOURCES = \
		arpa_bo_ng_prob.c bo_ng_prob.c calc_mem_req.c calc_prob_of.c \
		compute_back_off.c compute_discount.c compute_unigram.c \
		counts.c decode_bo_case.c disc_meth.c disc_meth_absolute.c \
		disc_meth_good_turing.c disc_meth_linear.c \
		disc_meth_witten_bell.c disc_method_linear.c \
		gen_fb_list.c generate.c genrand.c get_ngram.c guess_mem.c \
		idngram2lm.c increment_context.c load_lm.c lookup_index_of.c \
		miscella.c ngram.c num_of_types.c parse_comline.c \
		perplexity.c short_indices.c stats.c two_byte_alphas.c \
		validate.c write_lms.c

pkginclude_HEADERS = \
	bo_ng_prob.h				\
	disc_meth_absolute.h			\
	disc_meth_good_turing.h			\
	disc_meth.h				\
	disc_meth_linear.h			\
	disc_meth_witten_bell.h			\
	evallm.h				\
	genrand.h				\
	idngram2lm.h				\
	miscella.h				\
	ngram.h					\
	stats.h					\
	toolkit.h


INCLUDES = -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/win32
