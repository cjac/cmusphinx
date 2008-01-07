/*
 * FE.xs: Perl interface to the Sphinx feature extraction library
 *
 * Copyright (c) 2000 Cepstral LLC.
 * This module is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 *
 * Written by David Huggins-Daines <dhuggins@cs.cmu.edu>
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <prim_type.h>
#include <fe.h>

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
	if (strEQ(name, "BB_SAMPLING_RATE"))
#ifdef BB_SAMPLING_RATE
	    return BB_SAMPLING_RATE;
#else
	    goto not_there;
#endif
	break;
    case 'C':
	break;
    case 'D':
	if (strEQ(name, "DEFAULT_BB_FRAME_SHIFT"))
#ifdef DEFAULT_BB_FRAME_SHIFT
	    return DEFAULT_BB_FRAME_SHIFT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_BB_LOWER_FILT_FREQ"))
#ifdef DEFAULT_BB_LOWER_FILT_FREQ
	    return DEFAULT_BB_LOWER_FILT_FREQ;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_BB_NUM_FILTERS"))
#ifdef DEFAULT_BB_NUM_FILTERS
	    return DEFAULT_BB_NUM_FILTERS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_BB_UPPER_FILT_FREQ"))
#ifdef DEFAULT_BB_UPPER_FILT_FREQ
	    return DEFAULT_BB_UPPER_FILT_FREQ;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_FB_TYPE"))
#ifdef DEFAULT_FB_TYPE
	    return DEFAULT_FB_TYPE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_FFT_SIZE"))
#ifdef DEFAULT_FFT_SIZE
	    return DEFAULT_FFT_SIZE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_FRAME_RATE"))
#ifdef DEFAULT_FRAME_RATE
	    return DEFAULT_FRAME_RATE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_NB_FRAME_SHIFT"))
#ifdef DEFAULT_NB_FRAME_SHIFT
	    return DEFAULT_NB_FRAME_SHIFT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_NB_LOWER_FILT_FREQ"))
#ifdef DEFAULT_NB_LOWER_FILT_FREQ
	    return DEFAULT_NB_LOWER_FILT_FREQ;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_NB_NUM_FILTERS"))
#ifdef DEFAULT_NB_NUM_FILTERS
	    return DEFAULT_NB_NUM_FILTERS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_NB_UPPER_FILT_FREQ"))
#ifdef DEFAULT_NB_UPPER_FILT_FREQ
	    return DEFAULT_NB_UPPER_FILT_FREQ;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_NUM_CEPSTRA"))
#ifdef DEFAULT_NUM_CEPSTRA
	    return DEFAULT_NUM_CEPSTRA;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_PRE_EMPHASIS_ALPHA"))
#ifdef DEFAULT_PRE_EMPHASIS_ALPHA
	    return DEFAULT_PRE_EMPHASIS_ALPHA;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_SAMPLING_RATE"))
#ifdef DEFAULT_SAMPLING_RATE
	    return DEFAULT_SAMPLING_RATE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_START_FLAG"))
#ifdef DEFAULT_START_FLAG
	    return DEFAULT_START_FLAG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFAULT_WINDOW_LENGTH"))
#ifdef DEFAULT_WINDOW_LENGTH
	    return DEFAULT_WINDOW_LENGTH;
#else
	    goto not_there;
#endif
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
	if (strEQ(name, "LOG_LINEAR"))
#ifdef LOG_LINEAR
	    return LOG_LINEAR;
#else
	    goto not_there;
#endif
	break;
    case 'M':
	if (strEQ(name, "MEL_SCALE"))
#ifdef MEL_SCALE
	    return MEL_SCALE;
#else
	    goto not_there;
#endif
	break;
    case 'N':
	if (strEQ(name, "NB_SAMPLING_RATE"))
#ifdef NB_SAMPLING_RATE
	    return NB_SAMPLING_RATE;
#else
	    goto not_there;
#endif
	break;
    case 'O':
	break;
    case 'P':
	break;
    case 'Q':
	break;
    case 'R':
	break;
    case 'S':
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

MODULE = Audio::MFCC	PACKAGE = Audio::MFCC	PREFIX = fe_

double
constant(name,arg)
	char *		name
	int		arg

fe_t *
fe_init(class, param)
	SV *		class
	SV *		param
	PREINIT:
		param_t par;
		HV * hparm;
		SV ** helm;
	CODE:
		if (!(SvROK(param) && SvTYPE(SvRV(param)) == SVt_PVHV))
			croak("expected a hash reference");
		memset(&par, 0, sizeof(par));
		hparm = (HV *)SvRV(param);
		if ((helm = hv_fetch(hparm, "sampling_rate",
				     sizeof("sampling_rate"), 0)))
     			par.SAMPLING_RATE = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "frame_rate",
				     sizeof("frame_rate"), 0)))
     			par.FRAME_RATE = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "window_length",
				     sizeof("window_length"), 0)))
     			par.WINDOW_LENGTH = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "fb_type",
				     sizeof("fb_type"), 0)))
     			par.FB_TYPE = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "num_cepstra",
				     sizeof("num_cepstra"), 0)))
     			par.NUM_CEPSTRA = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "num_filters",
				     sizeof("num_filters"), 0)))
     			par.NUM_FILTERS = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "fft_size",
				     sizeof("fft_size"), 0)))
     			par.FFT_SIZE = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "lower_filt_freq",
				     sizeof("lower_filt_freq"), 0)))
     			par.LOWER_FILT_FREQ = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "upper_filt_freq",
				     sizeof("upper_filt_freq"), 0)))
     			par.UPPER_FILT_FREQ = SvIV(*helm);
		if ((helm = hv_fetch(hparm, "pre_emphasis_alpha",
				     sizeof("pre_emphasis_alpha"), 0)))
     			par.PRE_EMPHASIS_ALPHA = SvIV(*helm);
		RETVAL = fe_init(&par);
	OUTPUT:
		RETVAL

MODULE = Audio::MFCC	PACKAGE = fe_tPtr	PREFIX = fe_

SYSRET
fe_start_utt(fe)
	fe_t *		fe

void
fe_process_utt(fe, spch, nsamps)
	fe_t *		fe
	int16 *		spch
	int32		nsamps
	PREINIT:
		float32 **cep;
		int32 i, frame_count, output_frames;
	PPCODE:
		if (fe_process_utt(fe, spch, nsamps, &cep, &output_frames) < 0)
			goto out; /* empty list */
		assert(output_frames <= frame_count);
		if (output_frames <= 0)
			goto out; /* empty list */

		EXTEND(sp, output_frames);
		for (i = 0; i < output_frames; ++i) {
			SV ** svs;
			AV * vec;
			int j;

			New(0xdeadbeef, svs, fe->NUM_CEPSTRA, SV *);
			for (j = 0; j < fe->NUM_CEPSTRA; ++j)
				svs[j] = newSVnv(cep[i][j]);

			vec = av_make(fe->NUM_CEPSTRA, svs);

			for (j = 0; j < fe->NUM_CEPSTRA; ++j)
				SvREFCNT_dec(svs[j]);
			Safefree(svs);
			PUSHs(sv_2mortal(newRV_noinc((SV *) vec)));
		}
	    out:
		fe_free_2d(cep);


SV *
fe_end_utt(fe)
	fe_t *		fe
	PREINIT:
	int32 output_frames;
	float32 *cepv;
	CODE:
		New(0xc0debabe, cepv, fe->NUM_CEPSTRA, float32);
		if (fe_end_utt(fe, cepv, &output_frames) < 0)
			output_frames = -1;

		if (output_frames > 0) { /* 1 is the only possible value */
			SV ** svs;
			AV * vec;
			int i;

			New(0xdeadbeef, svs, fe->NUM_CEPSTRA, SV *);
			for (i = 0; i < fe->NUM_CEPSTRA; ++i)
				svs[i] = newSVnv(cepv[i]);

			vec = av_make(fe->NUM_CEPSTRA, svs);

			for (i = 0; i < fe->NUM_CEPSTRA; ++i)
				SvREFCNT_dec(svs[i]);
			Safefree(svs);

			/* Will be mortalized by XS, so don't do it here */
			RETVAL = newRV_noinc((SV *) vec);
		} else if (output_frames == 0) { /* success */
			RETVAL = newSVpv("", PL_na);
		} else {
			RETVAL = &PL_sv_undef;
		}

		Safefree(cepv);
		OUTPUT:
			RETVAL

SYSRET
DESTROY(fe)
	fe_t *		fe
	CODE:
		RETVAL = fe_close(fe);
	OUTPUT:
		RETVAL
