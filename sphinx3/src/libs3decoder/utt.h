#ifndef _UTT_H_
#define _UTT_H_

#include <libutil/libutil.h>
#include "kb.h"

#define MAXHYPLEN       1000

void utt_begin (kb_t *kb);

void utt_end (kb_t *kb);

void utt_word_trans (kb_t *kb, int32 cf);

void utt_decode (void *data, char *uttfile, int32 sf, 
			     int32 ef, char *uttid);

/* This function decodes a block of incoming feature vectors.
 * Feature vectors have to be computed by the calling routine.
 * The utterance level index of the last feature vector decoded
 * (before the current block) must be passed.
 * The current status of the decode is stored in the kb structure that
 * is passed in.
 */

void utt_decode_block (float **block_feat,   /* Incoming block of featurevecs */
                       int32 block_nfeatvec, /* No. of vecs in cepblock */
                       int32 *curfrm,        /* Utterance level index of
                                                frames decoded so far */
                       kb_t *kb,             /* kb structure with all model
                                                and decoder info */
                       int32 maxwpf,         /* Max words per frame */
                       int32 maxhistpf,      /* Max histories per frame */
                       int32 maxhmmpf,       /* Max active HMMs per frame */
                       int32 ptranskip,      /* intervals at which wbeam
                                                is used for phone transitions */
                       FILE *hmmdumpfp);     /* dump file */

#endif
