/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */




/*
 * kbcore.c -- Routines for initializing the main models.
 * 
 * 
 * HISTORY
 * 
 * 28-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libfeat/libfeat.h>
#include "kbcore.h"


kbcore_t *kbcore_init (float64 logbase,
		       char *feattype,
		       char *mdeffile,
		       char *dictfile,
		       char *fdictfile,
		       char *compsep,
		       char *lmfile,
		       char *fillpenfile,
		       float64 silprob,
		       float64 fillprob,
		       float64 lw,
		       float64 wip,
		       char *meanfile,
		       char *varfile,
		       float64 varfloor,
		       char *sen2mgau,
		       char *mixwfile,
		       float64 mixwfloor,
		       char *tmatfile,
		       float64 tmatfloor)
{
    kbcore_t *kb;
    int32 i;
    
    E_INFO("Initializing core models:\n");
    
    kb = (kbcore_t *) ckd_calloc (1, sizeof(kbcore_t));
    kb->fcb = NULL;
    kb->mdef = NULL;
    kb->dict = NULL;
    kb->lm = NULL;
    kb->fillpen = NULL;
    kb->tmat = NULL;
    kb->dict2lmwid = NULL;
    kb->gau = NULL;
    kb->sen = NULL;
    
    logs3_init (logbase);
    
    if (feattype) {
	if ((kb->fcb = feat_init (feattype)) == NULL)
	    E_FATAL("feat_init(%s) failed\n", feattype);
    }

    if (mdeffile) {
	if ((kb->mdef = mdef_init (mdeffile)) == NULL)
	    E_FATAL("mdef_init(%s) failed\n", mdeffile);
    }

    if (dictfile) {
	if (! compsep)
	    compsep = "";
	else if ((compsep[0] != '\0') && (compsep[1] != '\0')) {
	    E_FATAL("Compound word separator(%s) must be empty or single character string\n",
		    compsep);
	}
	if ((kb->dict = dict_init (kb->mdef, dictfile, fdictfile, compsep[0])) == NULL)
	    E_FATAL("dict_init(%s,%s,%s) failed\n", dictfile,
		    fdictfile ? fdictfile : "",
		    compsep);
    }

    if (lmfile) {
	if ((kb->lm = lm_read (lmfile, lw, wip)) == NULL)
	    E_FATAL("lm_read(%s) failed\n", lmfile);
    }
    
    if (fillpenfile || (lmfile && kb->dict)) {
	if (! kb->dict)		/* Sic */
	    E_FATAL("No dictionary specified for associating filler penalty file(%s)\n",
		    fillpenfile);
	if ((kb->fillpen = fillpen_init (kb->dict, fillpenfile, silprob, fillprob,
					 lw, wip)) == NULL)
	    E_FATAL("fillpen_init(%s) failed\n", fillpenfile);
    }
    
    if (meanfile) {
	if (! varfile)
	    E_FATAL("Varfile not specified along with meanfile(%s)\n", meanfile);
	if ((kb->gau = gauden_init (meanfile, varfile, varfloor, TRUE)) == NULL)
	    E_FATAL("gauden_init(%s, %s, %e) failed\n", meanfile, varfile, varfloor);
    }
    
    if (mixwfile) {
	if (! sen2mgau)
	    E_FATAL("sen2mgau argument not specified for mixwfile(%s)\n", mixwfile);
	if ((kb->sen = senone_init (mixwfile, sen2mgau, mixwfloor)) == NULL)
	    E_FATAL("senone_init (%s, %s, %e) failed\n", mixwfile, sen2mgau, mixwfloor);
    }

    if (tmatfile) {
	if ((kb->tmat = tmat_init (tmatfile, tmatfloor)) == NULL)
	    E_FATAL("tmat_init (%s, %e) failed\n", tmatfile, tmatfloor);
    }
    
    if (kb->dict && kb->lm) {
	/* Initialize dict2lmwid */
	if ((kb->dict2lmwid = wid_dict_lm_map (kb->dict, kb->lm)) == NULL)
	    E_FATAL("Dict/LM word-id mapping failed\n");
    }
    
    /* ***************** Verifications ***************** */
    E_INFO("Verifying models consistency:\n");
    
    if (kb->fcb && kb->gau) {
	/* Verify feature streams against gauden codebooks */
	if (feat_n_stream(kb->fcb) != gauden_n_stream(kb->gau))
	    E_FATAL("#Feature streams mismatch: feat(%d), gauden(%d)\n",
		    feat_n_stream(kb->fcb), gauden_n_stream(kb->gau));
	for (i = 0; i < feat_n_stream(kb->fcb); i++) {
	    if (feat_stream_len(kb->fcb, i) != gauden_stream_len(kb->gau, i))
		E_FATAL("Feature streamlen[%d] mismatch: feat(%d), gauden(%d)\n", i,
			feat_stream_len(kb->fcb, i), gauden_stream_len(kb->gau, i));
	}
    }

    if (kb->fcb && kb->sen) {
	/* Verify senone parameters against gauden parameters */
	if (senone_n_stream(kb->sen) != feat_n_stream(kb->fcb))
	    E_FATAL("#Feature mismatch: feat(%d), senone(%d)\n",
		    feat_n_stream(kb->fcb), senone_n_stream(kb->sen));
    }
    
    if (kb->sen && kb->gau) {
	if (senone_n_mgau(kb->sen) != gauden_n_mgau(kb->gau))
	    E_FATAL("#Mixture gaussians mismatch: senone(%d), gauden(%d)\n",
		    senone_n_mgau(kb->sen), gauden_n_mgau(kb->gau));
    }
    
    if (kb->mdef && kb->sen) {
	/* Verify senone parameters against model definition parameters */
	if (kb->mdef->n_sen != senone_n_sen(kb->sen))
	    E_FATAL("#Senones mismatch: Model definition(%d) senone(%d)\n",
		    kb->mdef->n_sen, senone_n_sen(kb->sen));
    }
    
    if (kb->mdef && kb->tmat) {
	/* Verify transition matrices parameters against model definition parameters */
	if (kb->mdef->n_tmat != kb->tmat->n_tmat)
	    E_FATAL("Model definition has %d tmat; but #tmat= %d\n",
		    kb->mdef->n_tmat, kb->tmat->n_tmat);
	if (kb->mdef->n_emit_state != kb->tmat->n_state)
	    E_FATAL("#Emitting states in model definition = %d, #states in tmat = %d\n",
		    kb->mdef->n_emit_state, kb->tmat->n_state);
    }
    
    return kb;
}


#if (_KBCORE_TEST_)
static arg_t arglist[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-feat",
      ARG_STRING,
      NULL,
      "Feature type: s3_1x39 / s2_4x / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
    { "-mdef",
      ARG_STRING,
      NULL,
      "Model definition input file" },
    { "-dict",
      ARG_STRING,
      NULL,
      "Pronunciation dictionary input file" },
    { "-fdict",
      ARG_STRING,
      NULL,
      "Filler word pronunciation dictionary input file" },
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
    { "-lm",
      ARG_STRING,
      NULL,
      "Word trigram language model input file" },
    { "-fillpen",
      ARG_STRING,
      NULL,
      "Filler word probabilities input file" },
    { "-silprob",
      ARG_FLOAT32,
      "0.1",
      "Default silence word probability" },
    { "-fillprob",
      ARG_FLOAT32,
      "0.02",
      "Default non-silence filler word probability" },
    { "-lw",
      ARG_FLOAT32,
      "9.5",
      "Language weight" },
    { "-wip",
      ARG_FLOAT32,
      "0.2",
      "Word insertion penalty" },
    { "-mean",
      ARG_STRING,
      NULL,
      "Mixture gaussian means input file" },
    { "-var",
      ARG_STRING,
      NULL,
      "Mixture gaussian variances input file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to -var file)" },
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixw",
      ARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to -mixw file)" },
    { "-mgaubeam",
      ARG_FLOAT32,
      "1e-3",
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" },
    { "-tmat",
      ARG_STRING,
      NULL,
      "HMM state transition matrix input file" },
    { "-tmatfloor",
      ARG_FLOAT32,
      "0.0001",
      "HMM state transition probability floor (applied to -tmat file)" },
    
    { NULL, ARG_INT32, NULL, NULL }
};
	

main (int32 argc, char *argv[])
{
    cmd_ln_parse (arglist, argc, argv);
    kbcore_init (cmd_ln_float32("-logbase"),
		 cmd_ln_str("-feat"),
		 cmd_ln_str("-mdef"),
		 cmd_ln_str("-dict"),
		 cmd_ln_str("-fdict"),
		 cmd_ln_str("-compsep"),
		 cmd_ln_str("-lm"),
		 cmd_ln_str("-fillpen"),
		 cmd_ln_float32("-silprob"),
		 cmd_ln_float32("-fillprob"),
		 cmd_ln_float32("-lw"),
		 cmd_ln_float32("-wip"),
		 cmd_ln_str("-mean"),
		 cmd_ln_str("-var"),
		 cmd_ln_float32("-varfloor"),
		 cmd_ln_str("-senmgau"),
		 cmd_ln_str("-mixw"),
		 cmd_ln_float32("-mixwfloor"),
		 cmd_ln_str("-tmat"),
		 cmd_ln_float32("-tmatfloor"));
}
#endif
