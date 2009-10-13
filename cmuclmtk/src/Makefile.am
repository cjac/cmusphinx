AM_CPPFLAGS = -I${srcdir}/libs -I${srcdir}/liblmest -I${srcdir}/win32

SUBDIRS  = liblmest libs . programs


AC_OBJS         = libs/ac_hash.o libs/ac_lmfunc_impl.o libs/ac_parsetext.o

PC_OBJS         = libs/pc_comline.o  libs/pc_message.o

RR_OBJS         = libs/parse_line.o libs/quit.o \
                  libs/rd_wlist_arry.o libs/read_voc.o \
                  libs/read_wlist_si.o libs/rr_calloc.o \
                  libs/rr_feof.o libs/rr_fexists.o \
                  libs/rr_filesize.o libs/rr_fopen.o libs/rr_fread.o \
                  libs/rr_fseek.o libs/rr_fwrite.o libs/rr_iopen.o \
                  libs/rr_malloc.o libs/rr_oopen.o libs/salloc.o \
                  libs/sih.o libs/win32compat.o

IDNGRAM2LM_OBJS = liblmest/bo_ng_prob.o liblmest/calc_mem_req.o \
		liblmest/compute_back_off.o liblmest/compute_discount.o \
		liblmest/compute_unigram.o liblmest/get_ngram.o \
                liblmest/increment_context.o liblmest/lookup_index_of.o \
                liblmest/short_indices.o liblmest/two_byte_alphas.o \
		liblmest/write_lms.o liblmest/num_of_types.o \
		liblmest/guess_mem.o liblmest/counts.o \
		liblmest/miscella.o liblmest/ngram.o \
		liblmest/idngram2lm.o liblmest/generate.o \
	        liblmest/genrand.o

EVALLM_OBJS     = liblmest/arpa_bo_ng_prob.o liblmest/bo_ng_prob.o \
		liblmest/calc_prob_of.o liblmest/decode_bo_case.o \
		liblmest/gen_fb_list.o liblmest/load_lm.o \
                liblmest/lookup_index_of.o liblmest/parse_comline.o \
		liblmest/perplexity.o liblmest/short_indices.o \
		liblmest/stats.o liblmest/two_byte_alphas.o \
		liblmest/validate.o liblmest/num_of_types.o \
		liblmest/counts.o liblmest/disc_meth.o \
		liblmest/disc_meth_absolute.o liblmest/disc_meth_linear.o \
		liblmest/disc_meth_witten_bell.o \
		liblmest/disc_meth_good_turing.o

BINLM2ARPA_OBJS = liblmest/load_lm.o liblmest/write_lms.o \
		liblmest/short_indices.o liblmest/two_byte_alphas.o \
		liblmest/num_of_types.o liblmest/stats.o liblmest/counts.o

lib_LIBRARIES	= libSLM3.a
libSLM3_a_SOURCES = liblmest/get_ngram.c
libSLM3_a_LIBADD = $(IDNGRAM2LM_OBJS) $(EVALLM_OBJS) $(BINLM2ARPA_OBJS) \
                  liblmest/get_ngram.o $(PC_OBJS) $(RR_OBJS) $(AC_OBJS)

