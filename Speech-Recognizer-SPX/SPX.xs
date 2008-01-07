/*
 * SPX.xs:  Perl interface to the PocketSphinx speech recognizer.
 *
 * Copyright (c) 2000 Cepstral LLC.
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 *
 * Written by David Huggins-Daines <dhuggins@cs.cmu.edu>
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#include <fbs.h>

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int arg)
{
    errno = 0;
    switch (*name) {
    case 'A':
	break;
    case 'B':
	if (strEQ(name, "BG_SEG_SZ"))
#ifdef BG_SEG_SZ
	    return BG_SEG_SZ;
#else
	    goto not_there;
#endif
	break;
    case 'C':
	break;
    case 'D':
	break;
    case 'E':
	break;
    case 'F':
	break;
    case 'G':
	break;
    case 'H':
	break;
    case 'I':
	break;
    case 'J':
	break;
    case 'K':
	break;
    case 'L':
	if (strEQ(name, "LM3G_ACCESS_BG"))
#ifdef LM3G_ACCESS_BG
	    return LM3G_ACCESS_BG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "LM3G_ACCESS_ERR"))
#ifdef LM3G_ACCESS_ERR
	    return LM3G_ACCESS_ERR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "LM3G_ACCESS_TG"))
#ifdef LM3G_ACCESS_TG
	    return LM3G_ACCESS_TG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "LM3G_ACCESS_UG"))
#ifdef LM3G_ACCESS_UG
	    return LM3G_ACCESS_UG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "LOG_BG_SEG_SZ"))
#ifdef LOG_BG_SEG_SZ
	    return LOG_BG_SEG_SZ;
#else
	    goto not_there;
#endif
	break;
    case 'M':
	break;
    case 'N':
	if (strEQ(name, "NO_WORD"))
#ifdef NO_WORD
	    return NO_WORD;
#else
	    goto not_there;
#endif
	if (strEQ(name, "NUM_COEFF"))
#ifdef NUM_COEFF
	    return NUM_COEFF;
#else
	    goto not_there;
#endif
	if (strEQ(name, "NUM_SMOOTH"))
#ifdef NUM_SMOOTH
	    return NUM_SMOOTH;
#else
	    goto not_there;
#endif
	break;
    case 'O':
	if (strEQ(name, "OFFSET_LIKELIHOOD"))
#ifdef OFFSET_LIKELIHOOD
	    return OFFSET_LIKELIHOOD;
#else
	    goto not_there;
#endif
	break;
    case 'P':
	break;
    case 'Q':
	break;
    case 'R':
	break;
    case 'S':
	if (strEQ(name, "SPEECH_THRESHOLD"))
#ifdef SPEECH_THRESHOLD
	    return SPEECH_THRESHOLD;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SPHINXP_PORT"))
#ifdef SPHINXP_PORT
	    return SPHINXP_PORT;
#else
	    goto not_there;
#endif
	break;
    case 'T':
	break;
    case 'U':
	break;
    case 'V':
	break;
    case 'W':
	break;
    case 'X':
	break;
    case 'Y':
	break;
    case 'Z':
	break;
    case 'a':
	break;
    case 'b':
	break;
    case 'c':
	break;
    case 'd':
	break;
    case 'e':
	break;
    case 'f':
	break;
    case 'g':
	break;
    case 'h':
	break;
    case 'i':
	break;
    case 'j':
	break;
    case 'k':
	break;
    case 'l':
	break;
    case 'm':
	break;
    case 'n':
	break;
    case 'o':
	break;
    case 'p':
	break;
    case 'q':
	break;
    case 'r':
	break;
    case 's':
	break;
    case 't':
	break;
    case 'u':
	break;
    case 'v':
	break;
    case 'w':
	break;
    case 'x':
	break;
    case 'y':
	break;
    case 'z':
	break;
    case '_':
	break;
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

typedef int32 SYSRET;
int fbs_init_done = 0;

char **
build_argv_av(AV *av, int *argc)
{
	char **argv = NULL;
	SV *argv0 = perl_get_sv("0", 0);

	*argc = av_len(av) + 2;
	if (*argc) {
		int i;

		Newz(42, argv, *argc + 1, char *);
		argv[0] = SvPV(argv0, PL_na);
		for (i = 0; i <= av_len(av); i++)
			argv[i+1] = SvPV(*av_fetch(av, i, 0), PL_na);
	}

	return argv;
}

char **
build_argv_hv(HV *hv, int *argc)
{
	char **argv = NULL;
	SV *argv0 = perl_get_sv("0", 0);
	HE *he;

	*argc = (hv_iterinit(hv) * 2) + 1;
	if (*argc) {
		int i = 1;

		Newz(42, argv, *argc + 1, char *);
		argv[0] = SvPV(argv0, PL_na);
		while ((he = hv_iternext(hv)) != NULL) {
			I32 len;
			argv[i++] = hv_iterkey(he, &len);
			argv[i++] = SvPV(hv_iterval(hv, he), PL_na);
		}
	}
	return argv;
}

SV *
new_seg_sv(search_hyp_t *hyp)
{
	AV *seg_av;

	seg_av = newAV();
	/* CAUTION: This ordering must match the one in SPX.pm */
	av_push(seg_av, newSVpv(hyp->word ? hyp->word : "", 0));
	av_push(seg_av, newSViv(hyp->sf));
	av_push(seg_av, newSViv(hyp->ef));
	av_push(seg_av, newSViv(hyp->ascr));
	av_push(seg_av, newSViv(hyp->lscr));
	av_push(seg_av, newSViv(hyp->fsg_state));
	av_push(seg_av, newSVnv(hyp->conf));
	av_push(seg_av, newSViv(hyp->latden));

	return sv_bless(newRV_noinc((SV *)seg_av),
			gv_stashpv("Speech::Recognizer::SPX::Segment", 1));
}

AV *
new_segs_av(search_hyp_t *hyp)
{
	AV * segs_av;

	segs_av = newAV();
	while (hyp) {
		av_push(segs_av, new_seg_sv(hyp));
		hyp = hyp->next;
	}
	return segs_av;
}

SV *
new_hyp_sv(search_hyp_t *hyp)
{
	AV *hyp_av, *segs_av;
	search_hyp_t *h;
	SV *sent;

	hyp_av = newAV();

	/* Reconstruct sentence from segs. */
	sent = newSVpv("", 0);
	for (h = hyp; h; h = h->next) {
		if (h->word == NULL)
			continue;
		if (strcmp(h->word, "<s>") == 0)
			continue;
		sv_catpv(sent, h->word);
		sv_catpv(sent, " ");
	}
	av_push(hyp_av, sent); /* Sentence string */
	av_push(hyp_av, newSVpv(uttproc_get_uttid(), 0)); /* Utterance ID */
	av_push(hyp_av, newSViv(0)); /* Scaling factor */
	av_push(hyp_av, newSViv(0)); /* Acoustic score (unknown here) */
	av_push(hyp_av, newSViv(0)); /* Language score (unknown here) */
	segs_av = new_segs_av(hyp); /* List of segments */
	av_push(hyp_av, newRV_noinc((SV *)segs_av));

	return sv_bless(newRV_noinc((SV *)hyp_av),
			gv_stashpv("Speech::Recognizer::SPX::Hypothesis", 1));
}

/* FIXME: These are Sphinx2 "internal" functions that shouldn't be. */
#ifndef _S2_SEARCH_H_
int32 searchFrame();
#endif
#ifndef _KB_EXPORTS_H_
char  *kb_get_word_str(int32 wid);
int32  kb_get_word_id(char const *word);
#endif

MODULE = Speech::Recognizer::SPX		PACKAGE = Speech::Recognizer::SPX		

double
constant(name,arg)
	char *		name
	int		arg

void
fbs_init(argv_ref=&PL_sv_undef)
	SV *		argv_ref
	PREINIT:
		int argc;
		char **argv;
		int rv;
	PPCODE:
		if (fbs_init_done)	
			return;

		if (SvOK(argv_ref)) {
			SV * rargv = ST(0); /* Arr, I'm a pirate! */

			if (!(SvROK(rargv)))
				goto bad_arg;
			switch(SvTYPE(SvRV(rargv))) {
			case SVt_PVAV:
				argv = build_argv_av((AV *) SvRV(rargv), &argc);
				break;
			case SVt_PVHV:
				argv = build_argv_hv((HV *) SvRV(rargv), &argc);
				break;
			bad_arg:
			default:
				croak("fbs_init: expected an array or hash reference\n");
				break;
			}
		} else {
			argv = build_argv_av(perl_get_av("ARGV", 0), &argc);
		}

		rv = fbs_init(argc, argv);
		Safefree(argv);

		if (rv == 0) {
			XPUSHs(sv_2mortal(newSVpv("0 but true", 10)));
		} else {
			XPUSHs(&PL_sv_undef);
		}

SYSRET
fbs_end()

SYSRET
uttproc_begin_utt(...)
	PREINIT:
	char *id;
	STRLEN foo;
	CODE:
		if (items > 0) {
			id = SvPV(ST(0), foo);
		} else {
			id = NULL;
		}
		RETVAL = uttproc_begin_utt(id);
	OUTPUT:
		RETVAL

SYSRET
uttproc_rawdata(raw, block=0)
	SV *		raw
	int32		block
	PREINIT:
		int16 *	buf;
		STRLEN  nsamp;
	CODE:
		buf = (int16 *) SvPV(raw, nsamp);
		nsamp /= 2;
		RETVAL = uttproc_rawdata(buf, nsamp, block);
	OUTPUT:
		RETVAL

SYSRET
uttproc_cepdata(cep, block=0)
	AV *		cep
	int32		block
	PREINIT:
		float32  *arr, **arrp;
		int 	 i, num_cepstra, num_frames;
	CODE:
		if (av_len(cep) == 0
			|| SvTYPE(SvRV(*(av_fetch(cep, 0, 0)))) != SVt_PVAV
			|| av_len((AV *)(*(av_fetch(cep, 0, 0)))) == 0) {
			croak("uttproc_cepdata: expected a non-empty array of arrays");
		}
		num_frames = av_len(cep);
		num_cepstra = av_len((AV *)(*(av_fetch(cep, 0, 0))));
		/* Blargh. */
		New(0xf0ad, arr, (num_frames * num_cepstra), float32);
		New(0xdead, arrp, num_frames, float32 *);
		/* ARRRRRGGH */
		for (i = 0; i <= num_frames; ++i) {
			AV *vec;
			int j;

			vec = (AV *)(*(av_fetch(cep, i, 0)));
			arrp[i] = arr;
			/* Don't trust av_len(vec) */
			for (j = 0; j <= num_cepstra; ++j) {
				SV *coeff;

				coeff = *(av_fetch(vec, j, 0));
				if (SvOK(coeff))
					*arr = SvNV(coeff);
				else
					*arr = 0.0;
				++arr;
			}
		}
		RETVAL = uttproc_cepdata(arrp, num_frames, block);
	OUTPUT:
		RETVAL

SYSRET
uttproc_end_utt()

SYSRET
uttproc_abort_utt()

SYSRET
uttproc_stop_utt()

SYSRET
uttproc_restart_utt()

void
uttproc_result(block=0)
	int32		block
	PREINIT:
		int32 frm;
		char *hyp = NULL;
		int res;
	PPCODE:
		res = uttproc_result(&frm, &hyp, block);
		if (res < 0)
			return; /* empty list */
		XPUSHs(sv_2mortal(newSViv(frm)));
		if (hyp != NULL) {
			PUSHs(sv_2mortal(newSVpv(hyp, 0)));
		}

void
uttproc_result_seg(block=0)
	int32		block
	PREINIT:
		int32 frm;
		search_hyp_t *hyp;
		int res;
	PPCODE:
		res = uttproc_result_seg(&frm, &hyp, block);
		if (res < 0)
			return; /* empty list */
		XPUSHs(sv_2mortal(newSViv(frm)));
		if (hyp != NULL) {
			PUSHs(sv_2mortal(new_hyp_sv(hyp)));
		}

void
uttproc_partial_result()
	PREINIT:
		int32 frm;
		char *hyp;
		int res;
	PPCODE:
		res = uttproc_partial_result(&frm, &hyp);
		if (res < 0)
			return; /* empty list */
		EXTEND(SP, 2);
		PUSHs(sv_2mortal(newSViv(frm)));
		PUSHs(sv_2mortal(newSVpv(hyp, 0)));

void
uttproc_partial_result_seg()
	PREINIT:
		int32 frm;
		search_hyp_t *hyp;
		int res;
	PPCODE:
		res = uttproc_partial_result_seg(&frm, &hyp);
		if (res < 0)
			return; /* empty list */
		EXTEND(SP, 2);
		PUSHs(sv_2mortal(newSViv(frm)));
		PUSHs(sv_2mortal(new_hyp_sv(hyp)));

char const *
uttproc_get_uttid()

SYSRET
uttproc_set_auto_uttid_prefix(prefix)
	char *		prefix

SYSRET
uttproc_set_lm(lmname)
	char *		lmname

SYSRET
uttproc_lmupdate(lmname)
	char *		lmname

SYSRET
uttproc_set_context(wd1, wd2)
	char *		wd1
	char *		wd2

SYSRET
uttproc_set_rawlogdir(dir)
	char *		dir

SYSRET
uttproc_set_mfclogdir(dir)
	char *		dir

SYSRET
uttproc_set_logfile(file)
	char *		file

SYSRET
lm_read(lmfile, lmname, lw, uw, wip)
	char *		lmfile
	char *		lmname
	double		lw
	double		uw
	double		wip

SYSRET
lm_delete(lmname)
	char *		lmname

void
search_get_alt(n, sf=0, ef=searchFrame(), w1=NULL, w2="<s>")
	int32	  	n
	int32		sf
	int32		ef
	char *		w1
	char *		w2
	PREINIT:
		int32 w1_wid, w2_wid, i;
		search_hyp_t **alt_out;
	PPCODE:
		/* Convert word strings to word-ids. */
		if (w1 != NULL)
			w1_wid = kb_get_word_id(w1);
		else
			w1_wid = -1;
		if (w2 != NULL)
			w2_wid = kb_get_word_id(w2);
		else
			w2_wid = kb_get_word_id("<s>");

		search_save_lattice();
		n = search_get_alt(n, sf, ef, w1_wid, w2_wid,
				   &alt_out);
		if (n == -1)
			return;
		EXTEND(SP, n);
		for (i = 0; i < n; ++i) {
			SV *hyp, **uttid;

			hyp = new_hyp_sv(alt_out[i]);
			uttid = av_fetch((AV *)SvRV(hyp), 1, 0);
			sv_catpvf(*uttid, "[%d]", i);
			PUSHs(sv_2mortal(hyp));
		}
