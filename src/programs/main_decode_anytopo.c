/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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
 * main.c -- Main S3 decoder driver.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 26-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporated from sphinx 3.0 code base to 3.X codebase. 
 *
 * $Log$
 * Revision 1.6  2004/12/14  00:50:33  arthchan2003
 * 1, Change the code to accept extension, 2, add timer to livepretend, 3, fixing the s3_astar to separate the bypass variable to bypass and is_filler_bypass.  4, Add some doxygen comments. 5, Don't care about changes in main_decode_anytopo.c. It is still under work, 6, remove option -help and -example from 3.5 releases.
 * 
 * Revision 1.5  2004/12/06 11:31:47  arthchan2003
 * Fix brief comments for programs.
 *
 * Revision 1.4  2004/12/06 11:15:11  arthchan2003
 * Enable doxygen in the program directory.
 *
 * Revision 1.3  2004/12/05 12:01:32  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.2  2004/11/16 05:13:19  arthchan2003
 * 1, s3cipid_t is upgraded to int16 because we need that, I already check that there are no magic code using 8-bit s3cipid_t
 * 2, Refactor the ep code and put a lot of stuffs into fe.c (should be renamed to something else.
 * 3, Check-in codes of wave2feat and cepview. (cepview will not dump core but Evandro will kill me)
 * 4, Make the same command line frontends for decode, align, dag, astar, allphone, decode_anytopo and ep . Allow the use a file to configure the application.
 * 5, Make changes in test such that test-allphone becomes a repeatability test.
 * 6, cepview, wave2feat and decode_anytopo will not be installed in 3.5 RCIII
 * (Known bugs after this commit)
 * 1, decode_anytopo has strange bugs in some situations that it cannot find the end of the lattice. This is urgent.
 * 2, default argument file's mechanism is not yet supported, we need to fix it.
 * 3, the bug discovered by SonicFoundry is still not fixed.
 *
 * Revision 1.1  2004/11/14 07:00:08  arthchan2003
 * 1, Finally, a version of working flat decoder is completed. It is not compiled in the standard compilation yet because there are two many warnings. 2, eliminate the statics variables in  fe_sigproc.c
 *
 * Revision 1.2  2002/12/03 23:02:40  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 *
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 * 
 * 02-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Restricted MLLR transformation to CD mixture densities only.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the meaning of -matchsegfn and, correspondingly, log_hypseg().
 * 
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -min_endfr and -dagfudge arguments.
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added BSTXCT: reporting since that became available from dag_search.
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ,NODES suffix to -outlatdir argument for dumping only words to
 * 		lattice output files.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added fillpen_init() and removed explicit addition of SILENCE_WORD,
 * 		START_WORD and FINISH_WORD to the dictionary.
 * 
 * 04-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added unlimit() call to remove malloc restrictions.
 *  
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added separate language weight (-bestpathlw) for bestpath DAG search.
 * 		Added -mllrctlfn flag and handling.
 * 
 * 21-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -bptblsize argument.
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added optional start/end frame specification in control file, for
 * 		processing selected segments (utterances) from a large cepfile.
 * 		Control spec: cepfile [startframe endframe [uttid]].
 * 
 * 13-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: added senscale to bestscr before writing best score file.
 * 		(Otherwise, the scaled scores are meaningless.)
 * 		Added ,EXACT suffix option to -matchfn argument, and correspondingly
 * 		added "exact" argument to log_hypstr().  (But running bestpath search
 * 		will still cause <sil> and filler words to be removed from matchfile.)
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed structure of gauden/senone computation:
 * 		    from: foreach (gauden) {eval gauden}; foreach (senone) {senone}
 * 		    to:   foreach (gauden) {eval gauden;  foreach (senone in gauden) {...}}
 * 		reducing memory space for results of gauden, specially in block mode.
 * 		Normalized senone scores (subtracting the best) rather than density scores.
 * 		Changed active senone list to flags.
 * 
 * 09-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Block-mode gauden computation for improving cache performance: changed
 *		    from: foreach(frame) {foreach(gauden)...}
 * 		    to:   foreach(gauden) {foreach(frame)...}
 * 		within a block of frames.  Must evaluate all gauden, not just active ones.
 * 		But even so the resulting caching performance is better.
 * 
 * 29-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed argument -inlatext to -latext.
 * 		Added check to ensure input and output lattice directories are different.
 * 		Added reporting of hostname.
 * 
 * 23-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed profiling to use timing_ functions, available on all platforms.
 * 		Added write_bestscore() for writing best statescore in each frame,
 * 		for helping determine desirable beamwidth.
 * 
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added DAG search option and consolidated logging and reporting.
 * 		Added backtrace option.
 * 
 * 22-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added absolute (unnormalized) acoustic scores in log file.
 * 		Added uttid in log file with each word segmentation.
 * 		Compute only active codebooks and senones if multiple codebooks present.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added specification of input word lattices to limit search (-inlatdir
 * 		argument...).  Added computation of active senone and gauden codebook
 * 		lists when such a lattice is provided, to minimize computation.
 * 		Added -cmn and -agc flags.
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added call to feat_init(); added cepsize variable and initialization;
 * 		Changed argument to norm_mean from featlen[0]+1 to cepsize.
 * 
 * 13-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed call to senone_eval to senone_eval_all optimized for the
 * 		semi-continuous case.
 * 		Completed handling multiple mixture-gaussian codebooks.
 * 
 * 01-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

/** \file main_decode_anytopo.c
 * \brief Main driver for sphinx 3.0 decoding (or the slow decoder)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <s3types.h>
#include "logs3.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "feat.h"
#include "bio.h"
#include <wid.h>
#include "search.h"

#include "cmn.h"
#include "agc.h"

#include "flat_fwd.h"
#include "ms_mllr.h"
#include "ms_gauden.h"
#include "ms_senone.h"
#include "interp.h"
#include "s3_dag.h"


static gauden_t *g;		/* Gaussian density codebooks */
static senone_t *sen;		/* Senones */
static interp_t *interp;	/* CD/CI interpolation */
static tmat_t *tmat;		/* HMM transition matrices */

static feat_t *fcb;           /* Feature type descriptor (Feature Control Block) */
static float32 ***feat = NULL;        /* Speech feature data */


static mdef_t *mdef;		/* Model definition */

extern lm_t* lm;
extern dict_t* dict;
extern fillpen_t* fpen;
extern s3lmwid_t *dict2lmwid;   /* Mapping from decoding dictionary wid's to lm ones.  
				   They may not be the same! */

static s3wid_t startwid, finishwid, silwid;
static int32 *senscale;		/* ALL senone scores scaled by senscale[i] in frame i */
static int32 *bestscr;		/* Best statescore in each frame */

ptmr_t tmr_utt;
ptmr_t tmr_fwdvit;
ptmr_t tmr_bstpth;
ptmr_t tmr_gausen;
ptmr_t tmr_fwdsrch;

pctr_t ctr_nfrm;
pctr_t ctr_nsen;


static int32 tot_nfr;

static char *inlatdir;
static char *outlatdir;
static int32 outlat_onlynodes;
static FILE *matchfp, *matchsegfp;
static int32 matchexact;


/*
 * Command line arguments.
 */
static arg_t defn[] = {
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-mdef", 
      ARG_STRING,
      NULL,
      "Model definition input file: triphone -> senones/tmat tying" },
    { "-tmat",
      ARG_STRING,
      NULL,
      "Transition matrix input file" },
    { "-mean",
      ARG_STRING,
      NULL,
      "Mixture gaussian codebooks mean parameters input file" },
    { "-var",
      ARG_STRING,
      NULL,
      "Mixture gaussian codebooks variance parameters input file" },
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixw",
      ARG_STRING,
      NULL,
      "Senone mixture weights parameters input file" },
    { "-lambda",
      ARG_STRING,
      NULL,
      "Interpolation weights (CD/CI senone) parameters input file" },
    { "-tpfloor",
      ARG_FLOAT32,
      "0.0001",
      "Triphone state transition probability floor applied to -tmat file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Codebook variance floor applied to -var file" },
    { "-mwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Codebook mixture weight floor applied to -mixw file" },
    { "-agc",
      ARG_STRING,
      "max",
      "AGC.  max: C0 -= max(C0) in current utt; none: no AGC" },
    { "-cmn",
      ARG_STRING,
      "current",
      "Cepstral mean norm.  current: C[1..n-1] -= mean(C[1..n-1]) in current utt; none: no CMN" },
    { "-varnorm",
      ARG_STRING,
      "no",
      "Cepstral var norm. yes: C[0..n-1] /= stddev(C[0..n-1]); no = no norm */"},
    { "-feat",
      ARG_STRING,
      "s2_4x",
      "Feature stream:\n\t\t\t\ts2_4x: Sphinx-II type 4 streams, 12cep, 24dcep, 3pow, 12ddcep\n\t\t\t\ts3_1x39: Single stream, 12cep+12dcep+3pow+12ddcep\n\t\t\t\t1s_12c_12d_3p_12dd: Single stream, 12cep+12dcep+3pow+12ddcep\n\t\t\t\t1s_c: Single stream, given input vector only\n\t\t\t\t1s_c_d: Feature + Deltas only\n\t\t\t\t1s_c_dd: Feature + Double deltas only\n\t\t\t\t1s_c_d_dd: Feature + Deltas + Double deltas\n\t\t\t\t1s_c_wd_dd: Feature cep+windowed delcep+deldel \n\t\t\t1s_c_d_ld_dd: Feature + delta + longter delta + doubledelta" },
/* ADDED BY BHIKSHA: 6 JAN 98 */
    { "-lminmemory",
      ARG_INT32,
      "0",
      "Load language model into memory (default: use disk cache for lm"},
    { "-ceplen",
      ARG_INT32,
      "13",
      "Length of input feature vector" },
    { "-dict",
      ARG_STRING,
      NULL,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdict",
      ARG_STRING,
      NULL,
      "Silence and filler (noise) word pronunciation dictionary input file" },
    { "-lm",
      ARG_STRING,
      NULL,
      "Language model input file (precompiled .DMP file)" },
    { "-lw",
      ARG_FLOAT32,
      "9.5",
      "Language weight: empirical exponent applied to LM probabilty" },
    { "-ugwt",
      ARG_FLOAT32,
      "0.7",
      "LM unigram weight: unigram probs interpolated with uniform distribution with this weight" },
    { "-bestpath",
      ARG_INT32,
      "0",
      "Whether to run bestpath DAG search after forward Viterbi pass" },
    { "-min_endfr",
      ARG_INT32,
      "3",
      "Nodes ignored during search if they persist for fewer than so many end frames" },
    { "-dagfudge",
      ARG_INT32,
      "2",
      "(0..2); 1 or 2: add edge if endframe == startframe; 2: if start == end-1" },
    { "-bestpathlw",
      ARG_FLOAT32,
      NULL,
      "Language weight for bestpath DAG search (default: same as -lw)" },
    { "-inspen",
      ARG_FLOAT32,
      "0.65",
      "Word insertion penalty" },
    { "-silpen",
      ARG_FLOAT32,
      "0.1",
      "Language model 'probability' of silence word" },
    { "-noisepen",
      ARG_FLOAT32,
      "0.05",
      "Language model 'probability' of each non-silence filler word" },
    { "-fillpen",
      ARG_STRING,
      NULL,
      "Filler word probabilities input file (used in place of -silpen and -noisepen)" },
    { "-ctl",
      ARG_STRING,
      NULL,
      "Input control file listing utterances to be decoded" },
    { "-ctloffset",
      ARG_INT32,
      "0",
      "No. of utterances at the beginning of -ctl file to be skipped" },
    { "-ctlcount",
      ARG_INT32,
      NULL,
      "No. of utterances in -ctl file to be processed (after -ctloffset).  Default: Until EOF" },
    { "-cepdir",
      ARG_STRING,
      ".",
      "Directory for utterances in -ctl file (if relative paths specified)." },
    { "-cepext",
      ARG_STRING,
      ".mfc",
      "File extension appended to utterances listed in -ctl file" },
    { "-mllrctl",
      ARG_STRING,
      NULL,
      "Input control file listing MLLR input data; parallel to -ctl argument file" },
    { "-topn",
      ARG_INT32,
      "4",
      "No. of top scoring densities computed in each mixture gaussian codebook" },
    { "-beam",
      ARG_FLOAT64,
      "1e-64",
      "Main pruning beam applied to triphones in forward search" },
    { "-nwbeam",
      ARG_FLOAT64,
      "1e-27",
      "Pruning beam applied in forward search upon word exit" },
    { "-phonepen",
      ARG_FLOAT32,
      "1.0",
      "Penalty applied for each phone transition" },
    { "-tracewhmm",
      ARG_STRING,
      NULL,
      "Word whose active HMMs are to be traced (for debugging/diagnosis/analysis)" },
    { "-hmmdumpsf",
      ARG_INT32,
      NULL,
      "Starting frame for dumping all active HMMs (for debugging/diagnosis/analysis)" },
    { "-worddumpsf",
      ARG_INT32,
      NULL,
      "Starting frame for dumping all active words (for debugging/diagnosis/analysis)" },
    { "-inlatdir",
      ARG_STRING,
      NULL,
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-inlatwin",
      ARG_INT32,
      "50",
      "Input word-lattice words starting within +/- <this argument> of current frame considered during search" },
    { "-outlatdir",
      ARG_STRING,
      NULL,
      "Directory for writing word lattices (one file/utterance); optional ,NODES suffix to write only the nodes" },
    { "-latext",
      ARG_STRING,
      "lat.gz",
      "Word-lattice filename extension (.gz or .Z extension for compression)" },
    { "-bestscoredir",
      ARG_STRING,
      NULL,
      "Directory for writing best score/frame (used to set beamwidth; one file/utterance)" },
    { "-match",
      ARG_STRING,
      NULL,
      "Recognition result output file (pre-1995 NIST format) (optional ,EXACT suffix)" },
    { "-matchseg",
      ARG_STRING,
      NULL,
      "Exact recognition result file with word segmentations and scores" },
    { "-logfn",
      ARG_STRING,
      NULL,
      "Log file (default stdout/stderr)" },
    { "-backtrace",
      ARG_INT32,
      "1",
      "Whether detailed backtrace information (word segmentation/scores) shown in log" },
    { "-bptblsize",
      ARG_INT32,
      "32767",
      "Number of BPtable entries to allocate initially (grown as necessary)" },
    { "-bptbldump",
      ARG_INT32,
      "0",
      "Whether BPTable should be dumped to log output (for debugging)" },
    
    { NULL, ARG_INT32,  NULL, NULL }
};

/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    float32 varfloor, mixwfloor, tpfloor;
    int32 i;
    char *arg;
    
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdef"));

    /* Dictionary */
    dict = dict_init (mdef,
		      (char *) cmd_ln_access("-dict"),
		      (char *) cmd_ln_access("-fdict"),
		      0);


    /* HACK!! Make sure SILENCE_WORD, START_WORD and FINISH_WORD are in dictionary */
    silwid = dict_wordid (dict, S3_SILENCE_WORD);
    startwid = dict_wordid (dict,S3_START_WORD);
    finishwid = dict_wordid (dict, S3_FINISH_WORD);
    if (NOT_S3WID(silwid) || NOT_S3WID(startwid) || NOT_S3WID(finishwid)) {
	E_FATAL("%s, %s, or %s missing from dictionary\n",
		S3_SILENCE_WORD, S3_START_WORD, S3_FINISH_WORD);
    }
    if ((dict->filler_start > dict->filler_end) || (! dict_filler_word (dict,silwid)))
	E_FATAL("%s must occur (only) in filler dictionary\n", S3_SILENCE_WORD);
    /* No check that alternative pronunciations for filler words are in filler range!! */

    /* Codebooks */
    varfloor = *((float32 *) cmd_ln_access("-varfloor"));
    g = gauden_init ((char *) cmd_ln_access("-mean"),
		     (char *) cmd_ln_access("-var"),
		     varfloor);

    /* Verify codebook feature dimensions against libfeat */
     if (feat_n_stream(fcb) != g->n_feat) {
       E_FATAL("#feature mismatch: feat= %d, mean/var= %d\n",
               feat_n_stream(fcb), g->n_feat);
     }
     for (i = 0; i < feat_n_stream(fcb); i++) {
       if (feat_stream_len(fcb,i) != g->featlen[i]) {
           E_FATAL("featlen[%d] mismatch: feat= %d, mean/var= %d\n", i,
                   feat_stream_len(fcb, i), g->featlen[i]);
       }
     }
     


    /* Senone mixture weights */
    mixwfloor = *((float32 *) cmd_ln_access("-mwfloor"));
    sen = senone_init ((char *) cmd_ln_access("-mixw"),
		       (char *) cmd_ln_access("-senmgau"),
		       mixwfloor);
    
    /* Verify senone parameters against gauden parameters */
    if (sen->n_feat != g->n_feat)
	E_FATAL("#Feature mismatch: gauden= %d, senone= %d\n", g->n_feat, sen->n_feat);
    if (sen->n_cw != g->n_density)
	E_FATAL("#Densities mismatch: gauden= %d, senone= %d\n", g->n_density, sen->n_cw);
    if (sen->n_gauden > g->n_mgau)
	E_FATAL("Senones need more codebooks (%d) than present (%d)\n",
		sen->n_gauden, g->n_mgau);
    if (sen->n_gauden < g->n_mgau)
	E_ERROR("Senones use fewer codebooks (%d) than present (%d)\n",
		sen->n_gauden, g->n_mgau);

    /* Verify senone parameters against model definition parameters */
    if (mdef->n_sen != sen->n_sen)
	E_FATAL("Model definition has %d senones; but #senone= %d\n",
		mdef->n_sen, sen->n_sen);

    /* CD/CI senone interpolation weights file, if present */
    if ((arg = (char *) cmd_ln_access ("-lambda")) != NULL) {
	interp = interp_init (arg);

	/* Verify interpolation weights size with senones */
	if (interp->n_sen != sen->n_sen)
	    E_FATAL("Interpolation file has %d weights; but #senone= %d\n",
		    interp->n_sen, sen->n_sen);
    } else
	interp = NULL;

    /* Transition matrices */
    tpfloor = *((float32 *) cmd_ln_access("-tpfloor"));
    tmat = tmat_init ((char *) cmd_ln_access("-tmat"), tpfloor);

    /* Verify transition matrices parameters against model definition parameters */
    if (mdef->n_tmat != tmat->n_tmat)
	E_FATAL("Model definition has %d tmat; but #tmat= %d\n",
		mdef->n_tmat, tmat->n_tmat);
    if (mdef->n_emit_state != tmat->n_state)
	E_FATAL("#Emitting states in model definition = %d, #states in tmat = %d\n",
		mdef->n_emit_state, tmat->n_state);

    /* LM */

    {
      char *lmfile;
      lmfile = (char *) cmd_ln_access("-lm");
      if (! lmfile)
	E_FATAL("-lm argument missing\n");


      lm = lm_read (lmfile, 
		    *(float32 *)cmd_ln_access("-lw"),
		    *(float32 *)cmd_ln_access("-inspen"),
		    *(float32 *)cmd_ln_access("-ugwt"));
      

      /* Filler penalties */
      
      fpen = fillpen_init (dict, 
			   (char *) cmd_ln_access("-fillpen"),
			   *(float32 *)cmd_ln_access("-silpen"),
			   *(float32 *)cmd_ln_access("-noisepen"),
			   *(float32 *)cmd_ln_access("-lw"),
			   *(float32 *)cmd_ln_access("-inspen"));
    }

    dict2lmwid = wid_dict_lm_map(dict, lm, *(float32*) cmd_ln_access("-lw"));


}


/*
 * Write exact hypothesis.  Format
 *   <id> S <scl> T <scr> A <ascr> L <lscr> {<sf> <wascr> <wlscr> <word>}... <ef>
 * where:
 *   scl = acoustic score scaling for entire utterance
 *   scr = ascr + (lscr*lw+N*wip), where N = #words excluding <s>
 *   ascr = scaled acoustic score for entire utterance
 *   lscr = LM score (without lw or wip) for entire utterance
 *   sf = start frame for word
 *   wascr = scaled acoustic score for word
 *   wlscr = LM score (without lw or wip) for word
 *   ef = end frame for utterance.
 */
static void log_hypseg (char *uttid,
			FILE *fp,	/* Out: output file */
			srch_hyp_t *hypptr,	/* In: Hypothesis */
			int32 nfrm,	/* In: #frames in utterance */
			int32 scl,	/* In: Acoustic scaling for entire utt */
			float64 lwf)	/* In: LM score scale-factor (in dagsearch) */
{
    srch_hyp_t *h;
    int32 ascr, lscr, tscr;
    
    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
	ascr += h->ascr;
	if (dict_basewid(dict,h->wid) != startwid) {
	    lscr += lm_rawscore (lm,h->lscr, lwf);
	} else {
	    assert (h->lscr == 0);
	}
	tscr += h->ascr + h->lscr;
    }

    fprintf (fp, "%s S %d T %d A %d L %d", uttid, scl, tscr, ascr, lscr);
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, " (null)\n");
    else {
	for (h = hypptr; h; h = h->next) {
	    lscr = (dict_basewid(dict,h->wid) != startwid) ? lm_rawscore (lm,h->lscr, lwf) : 0;
	    fprintf (fp, " %d %d %d %s", h->sf, h->ascr, lscr, dict_wordstr (dict,h->wid));
	}
	fprintf (fp, " %d\n", nfrm);
    }
    
    fflush (fp);
}


/* Write hypothesis in old (pre-Nov95) NIST format */
static void log_hypstr (FILE *fp, srch_hyp_t *hypptr, char *uttid, int32 exact, int32 scr)
{
    srch_hyp_t *h;
    s3wid_t w;
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, "(null)");
    
    for (h = hypptr; h; h = h->next) {
	w = h->wid;
	if (! exact) {
	    w = dict_basewid (dict,w);
	    if ((w != startwid) && (w != finishwid) && (! dict_filler_word (dict,w)))
		fprintf (fp, "%s ", dict_wordstr(dict,w));
	} else
	    fprintf (fp, "%s ", dict_wordstr(dict,w));
    }
    if (scr != 0)
	fprintf (fp, " (%s %d)\n", uttid, scr);
    else
	fprintf (fp, " (%s)\n", uttid);
    fflush (fp);
}


/* Log hypothesis in detail with word segmentations, acoustic and LM scores  */
static void log_hyp_detailed (FILE *fp, srch_hyp_t *hypptr, char *uttid, char *LBL, char *lbl)
{
    srch_hyp_t *h;
    int32 f, scale, ascr, lscr;

    ascr = 0;
    lscr = 0;
    
    fprintf (fp, "%s:%s> %20s %5s %5s %11s %10s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr", "LMScore");
    
    for (h = hypptr; h; h = h->next) {
	scale = 0;
	for (f = h->sf; f <= h->ef; f++)
	    scale += senscale[f];
	
	fprintf (fp, "%s:%s> %20s %5d %5d %11d %10d\n", lbl, uttid,
		 h->word, h->sf, h->ef, h->ascr + scale, h->lscr);

	ascr += h->ascr + scale;
	lscr += h->lscr;
    }

    fprintf (fp, "%s:%s> %20s %5s %5s %11d %10d\n", LBL, uttid,
	     "TOTAL", "", "", ascr, lscr);
}



static void write_bestscore (char *dir, char *uttid, int32 *score, int32 nfr)
{
    char filename[1024];
    FILE *fp;
    int32 k;
    
    sprintf (filename, "%s/%s.bscr", dir, uttid);
    E_INFO("Writing bestscore file: %s\n", filename);
    if ((fp = fopen (filename, "wb")) == NULL) {
	E_ERROR("fopen(%s,wb) failed\n", filename);
	return;
    }
    
    /* Write version no. */
    if (fwrite ("0.1\n", sizeof(char), 4, fp) != 4)
	goto write_error;

    /* Write binary comment string */
    if (fwrite ("*end_comment*\n", sizeof(char), 14, fp) != 14)
	goto write_error;

    /* Write byte-ordering magic number */
    k = BYTE_ORDER_MAGIC;
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write #frames */
    k = nfr;
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write bestscore/frame */
    if (fwrite (score, sizeof(int32), nfr, fp) != nfr)
	goto write_error;

    fclose (fp);
    return;
    
write_error:
    E_ERROR("fwrite(%s) failed\n", filename);
    fclose (fp);
}


#define GAUDEN_EVAL_WINDOW	8

/* Lists of senones sharing each mixture Gaussian codebook */
typedef struct mgau2sen_s {
    s3senid_t sen;		/* Senone shared by this mixture Gaussian */
    struct mgau2sen_s *next;	/* Next entry in list for this mixture Gaussian */
} mgau2sen_t;

/*
 * Forward Viterbi decode.
 * Return value: recognition hypothesis with detailed segmentation and score info.
 */
static srch_hyp_t *fwdvit (	/* In: MFC cepstra for input utterance */
		      int32 nfr,	/* In: #frames of input */
		      char *uttid)	/* In: Utterance id, for logging and other use */
{
    static int32 w;
    static int32 topn;
    static int32 **senscr;		/* Senone scores for window of frames */
    static gauden_dist_t **dist;	/* Density values for one mgau in one frame */
    static int8 *sen_active;		/* [s] TRUE iff s active in current frame */
    static int8 *mgau_active;		/* [m] TRUE iff m active in current frame */
    static mgau2sen_t **mgau2sen;	/* Senones sharing mixture Gaussian codebooks */

    int32 i, j, k, s, gid, n_sen_active, best;
    srch_hyp_t *hyp;
    mgau2sen_t *m2s;
    float32 **fv;


    i=0;
    
    if (! senscr) {

	w = feat_window_size (fcb);	/* #MFC vectors needed on either side of current
					   frame to compute one feature vector */
	topn = *((int32 *) cmd_ln_access("-topn"));
	E_INFO("The value of topn: %d\n",topn);
	if (topn > g->n_density) {
	    E_WARN("-topn argument (%d) > #density codewords (%d); set to latter\n",
		   topn, g->n_density);
	    topn = g->n_density;
	}
	dist = (gauden_dist_t **) ckd_calloc_2d (g->n_feat, topn, sizeof(gauden_dist_t));

	/*
	 * If search limited to given word lattice, or if many codebooks, only active
	 * senones computed in each frame.   Allocate space for list of active senones,
	 * and active codebook flags.
	 */
	if (inlatdir) {
	    E_INFO("Computing only active codebooks and senones each frame\n");
	    sen_active = (int8 *) ckd_calloc (sen->n_sen, sizeof(int8));
	    mgau_active = (int8 *) ckd_calloc (g->n_mgau, sizeof(int8));
	
	    /* Space for senone scores (one frame) */
	    senscr = (int32 **) ckd_calloc_2d (1, sen->n_sen, sizeof(int32));
	} else {
	    E_INFO("Computing all codebooks and senones each frame\n");
	    sen_active = NULL;
	    mgau_active = NULL;
	
	    /* Space for senone scores (window of frames) */
	    senscr = (int32 **) ckd_calloc_2d (GAUDEN_EVAL_WINDOW, sen->n_sen,
					       sizeof(int32));
	}
	
	/* Initialize mapping from mixture Gaussian to senones */
	mgau2sen = (mgau2sen_t **) ckd_calloc (g->n_mgau, sizeof(mgau2sen_t *));
	for (s = 0; s < sen->n_sen; s++) {
	    m2s = (mgau2sen_t *) listelem_alloc (sizeof(mgau2sen_t));
	    m2s->sen = s;
	    m2s->next = mgau2sen[sen->mgau[s]];
	    mgau2sen[sen->mgau[s]] = m2s;
	}
    }
    
    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return NULL;
    }
    
    ptmr_reset (&tmr_gausen);
    ptmr_reset (&tmr_fwdsrch);

    
    fwd_start_utt (uttid);

    /*
     * A feature vector for frame f depends on input MFC vectors [f-w..f+w].  Hence
     * the feature vector corresponding to the first w and last w input frames is
     * undefined.  We define them by simply replicating the first and last true
     * feature vectors (presumably silence regions).
     */
    if (sen_active) {
	for (i = 0; i < nfr; i++) {
	    ptmr_start (&tmr_gausen);
	    fv=feat[i];
	    /* Compute feature vector for current frame from input speech cepstra */

	    /* Obtain list of active senones */
	    fwd_sen_active (sen_active, sen->n_sen);
	    
	    /* Flag all active mixture-gaussian codebooks */
	    for (gid = 0; gid < g->n_mgau; gid++)
		mgau_active[gid] = 0;
	    n_sen_active = 0;
	    for (s = 0; s < sen->n_sen; s++) {
		if (sen_active[s]) {
		    mgau_active[sen->mgau[s]] = 1;
		    n_sen_active++;
		}
	    }

	    /* Add in CI senones and codebooks if interpolating with CI */
	    if (interp) {
		for (s = 0; s < mdef->n_ci_sen; s++) {
		    mgau_active[s] = 1;
		    if (! sen_active[s]) {
			sen_active[s] = 1;
			n_sen_active++;
		    }
		}
	    }

	    pctr_increment (ctr_nsen, n_sen_active);
	    
	    /* Compute topn gaussian density and senones values (for active codebooks) */
	    best = (int32) 0x80000000;
	    for (gid = 0; gid < g->n_mgau; gid++) {
		if (mgau_active[gid]) {
		    gauden_dist (g, gid, topn, fv, dist);
		    for (m2s = mgau2sen[gid]; m2s; m2s = m2s->next) {
			s = m2s->sen;
			if (sen_active[s]) {
			    senscr[0][s] = senone_eval (sen, s, dist, topn);
			    if (best < senscr[0][s])
				best = senscr[0][s];
			}
		    }
		}
	    }

	    /* Interpolate CI and CD senones if indicated */
	    if (interp) {
		for (s = mdef->n_ci_sen; s < sen->n_sen; s++) {
		    if (sen_active[s])
			interp_cd_ci (interp, senscr[0], s, mdef->cd2cisen[s]);
		}
	    }

	    /* Normalize senone scores (interpolation above can only lower best score) */
	    for (s = 0; s < sen->n_sen; s++) {
		if (sen_active[s])
		    senscr[0][s] -= best;

		E_INFO("The senone scores %d\n",senscr[0][s]);
	    }


	    senscale[i] = best;
	    ptmr_stop (&tmr_gausen);

	    /* Step HMMs one frame forward */
	    ptmr_start (&tmr_fwdsrch);
	    bestscr[i] = fwd_frame (senscr[0]);
	    ptmr_stop (&tmr_fwdsrch);
	    
	    if ((i%10) == 9) {
		printf ("."); fflush (stdout);
	    }
	}
    } else {
	/* Work in groups of GAUDEN_EVAL_WINDOW frames (blocking to improve cache perf) */
      assert(feat);
	for (j = 0; j < nfr; j += GAUDEN_EVAL_WINDOW) {
	    /* Compute Gaussian densities and senone scores for window of frames */
	    ptmr_start (&tmr_gausen);

	    for (gid = 0; gid < g->n_mgau; gid++) {
	      for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		    /* Compute feature vector for current frame from input speech cepstra */
		assert(feat[i]);
		fv=feat[i];

		    /* Evaluate mixture Gaussian densities */
		    gauden_dist (g, gid, topn, fv, dist);

		    /* Compute senone scores */
		    if (g->n_mgau > 1) {
			for (m2s = mgau2sen[gid]; m2s; m2s = m2s->next) {
			    s = m2s->sen;
			    senscr[k][s] = senone_eval (sen, s, dist, topn);
			}
		    } else {
			/* Semi-continuous special case; single shared codebook */
			senone_eval_all (sen, dist, topn, senscr[k]);
		    }
		}
	    }

	    /* Interpolate senones and normalize */
	    for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		pctr_increment (ctr_nsen, sen->n_sen);

		if (interp)
		    interp_all (interp, senscr[k], mdef->cd2cisen, mdef->n_ci_sen);

		/* Normalize senone scores */
		best = (int32)0x80000000;
		for (s = 0; s < sen->n_sen; s++){
		    if (best < senscr[k][s])
			best = senscr[k][s];

		}
		for (s = 0; s < sen->n_sen; s++)
		    senscr[k][s] -= best;
		senscale[i] = best;
	    }

	    ptmr_stop (&tmr_gausen);
		
	    /* Step HMMs one frame forward */
	    ptmr_start (&tmr_fwdsrch);
	    for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		bestscr[i] = fwd_frame (senscr[k]);
		if ((i%10) == 9) {
		    printf ("."); fflush (stdout);
		}
	    }


	    ptmr_stop (&tmr_fwdsrch);

	}
    }
    printf ("\n");

    hyp = fwd_end_utt ();

    /* Add in senscale into bestscr, turning them into absolute scores */
    k = 0;
    for (i = 0; i < nfr; i++) {
	k += senscale[i];
	bestscr[i] += k;
    }

    pctr_increment (ctr_nfrm, nfr);

    return hyp;
}


/* Decode the given mfc file and write result to matchfp and matchsegfp */
static void decode_utt (int32 nfr, char *uttid)
{
    char *bscrdir;
    srch_hyp_t *hyp, *h;
    int32 i, bp, ascr, lscr, scl;
    float32 *f32arg;
    float64 lwf;


    ptmr_reset (&tmr_utt);
    ptmr_reset (&tmr_fwdvit);
    ptmr_reset (&tmr_bstpth);
    ptmr_start (&tmr_utt);
    ptmr_start (&tmr_fwdvit);

    pctr_reset(ctr_nfrm);
    pctr_reset(ctr_nsen);

    hyp = fwdvit (nfr, uttid);
    ptmr_stop (&tmr_fwdvit);
    bp = *((int32 *) cmd_ln_access("-bestpath"));
    scl = 0;
    lwf = 1.0;
    if (hyp != NULL) {
	if ( *((int32 *) cmd_ln_access("-backtrace")) )
	    log_hyp_detailed (stdout, hyp, uttid, "FV", "fv");

	/* Total acoustic score scaling */
	for (i = 0; i < nfr; i++)
	    scl += senscale[i];

	/* Total scaled acoustic score and LM score */
	ascr = lscr = 0;
	for (h = hyp; h; h = h->next) {
	    ascr += h->ascr;
	    lscr += h->lscr;
	}

	/* Print sanitized recognition */
	printf ("FWDVIT: ");
	log_hypstr (stdout, hyp, uttid, matchexact, ascr + lscr);

	printf ("FWDXCT: ");
	log_hypseg (uttid, stdout, hyp, nfr, scl, lwf);
	lm_cache_stats_dump (lm);

	/* Check if need to dump bestscore/frame */
	if ((bscrdir = (char *) cmd_ln_access ("-bestscoredir")) != NULL)
	    write_bestscore (bscrdir, uttid, bestscr, nfr);

	/* Check if need to dump or search DAG */
	if ((outlatdir || bp) && (dag_build () == 0)) {
	    if (outlatdir)
		dag_dump (outlatdir, outlat_onlynodes, uttid);
	    
	    /* Perform bestpath DAG search if specified */
	    if (bp) {
		ptmr_start (&tmr_bstpth);
		h = s3flat_fwd_dag_search (uttid);
		ptmr_stop (&tmr_bstpth);
		
		if (h) {
		    hyp = h;

		    f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
		    lwf = f32arg ?
			((*f32arg) / *((float32 *) cmd_ln_access ("-lw"))) :
			1.0;
		} else
		    E_ERROR("%s: Bestpath search failed; using Viterbi result\n", uttid);
		
		if ( *((int32 *) cmd_ln_access("-backtrace")) )
		    log_hyp_detailed (stdout, hyp, uttid, "BP", "bp");
		
		/* Total scaled acoustic score and LM score */
		ascr = lscr = 0;
		for (h = hyp; h; h = h->next) {
		    ascr += h->ascr;
		    lscr += h->lscr;
		}
		
		printf ("BSTPTH: ");
		log_hypstr (stdout, hyp, uttid, matchexact, ascr + lscr);
		
		printf ("BSTXCT: ");
		log_hypseg (uttid, stdout, hyp, nfr, scl, lwf);
	    }
	    
	    dag_destroy ();
	}
	
	lm_cache_stats_dump (lm);
	lm_cache_reset (lm);
    } else {
	E_ERROR ("%s: Viterbi search failed\n", uttid);
	hyp = NULL;
    }
    
    /* Log recognition output to the standard match and matchseg files */
    if (matchfp)
	log_hypstr (matchfp, hyp, uttid, matchexact, 0);
    if (matchsegfp)
	log_hypseg (uttid, matchsegfp, hyp, nfr, scl, lwf);

    ptmr_stop (&tmr_utt);
    
    printf ("%s: ", uttid);

    pctr_print(stderr,ctr_nfrm);
    pctr_print(stderr,ctr_nsen);

    printf ("%s: TMR: %5d Frm", uttid, nfr);
    if (nfr > 0) {
	printf (" %6.2f xEl", tmr_utt.t_elapsed * 100.0 / nfr);
	printf (" %6.2f xCPU", tmr_utt.t_cpu * 100.0 / nfr);

	if (tmr_utt.t_cpu > 0.0) {
	    printf (" [fwd %6.2fx %3d%%]", tmr_fwdvit.t_cpu * 100.0 / nfr,
		    (int32) ((tmr_fwdvit.t_cpu * 100.0) / tmr_utt.t_cpu));
	    printf ("[gau+sen %6.2fx %2d%%]", tmr_gausen.t_cpu * 100.0 / nfr,
		    (int32) ((tmr_gausen.t_cpu * 100.0) / tmr_utt.t_cpu));
	    printf ("[srch %6.2fx %2d%%]", tmr_fwdsrch.t_cpu * 100.0 / nfr,
		    (int32) ((tmr_fwdsrch.t_cpu * 100.0) / tmr_utt.t_cpu));
	    
	    fwd_timing_dump (tmr_utt.t_cpu);
	    
	    if (bp)
		printf ("[bp %6.2fx %2d%%]", tmr_bstpth.t_cpu * 100.0 / nfr,
			(int32) ((tmr_bstpth.t_cpu * 100.0) / tmr_utt.t_cpu));
	}
    }
    printf ("\n");
    fflush (stdout);
    
    tot_nfr += nfr;
}


/* Process utterances in the control file (-ctl argument) */
static int32 process_ctlfile ( void )
{
    FILE *ctlfp, *mllrctlfp;
    char *ctlfile, *cepdir, *cepext, *mllrctlfile;
    char *matchfile, *matchsegfile;
    char line[1024], ctlspec[1024], cepfile[1024], uttid[1024];
    char mllrfile[1024], prevmllr[1024];
/* CHANGE BY BHIKSHA: ADDED veclen AS A VARIABLE, 6 JAN 98 */
    int32 ctloffset, ctlcount, veclen, sf, ef, nfr;
/* END OF CHANGES BY BHIKSHA */
    int32 err_status;
    int32 i, k;
    
    err_status = 0;
    
    if ((ctlfile = (char *) cmd_ln_access("-ctl")) == NULL)
	E_FATAL("No -ctl argument\n");
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    if ((mllrctlfile = (char *) cmd_ln_access("-mllrctl")) != NULL) {
	if ((mllrctlfp = fopen (mllrctlfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", mllrctlfile);
    } else
	mllrctlfp = NULL;
    prevmllr[0] = '\0';
    
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    if ((matchfile = (char *) cmd_ln_access("-match")) == NULL) {
	matchfp = NULL;
    } else {
	/* Look for ,EXACT suffix, for retaining fillers/pronunciation specs in output */
	k = strlen (matchfile);
	if ((k > 6) && (strcmp (matchfile+(k-6), ",EXACT") == 0)) {
	    matchexact = 1;
	    matchfile[k-6] = '\0';
	} else
	    matchexact = 0;

	if ((matchfp = fopen (matchfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchfile);
    }
    
    if ((matchsegfile = (char *) cmd_ln_access("-matchseg")) == NULL) {
	E_WARN("No -matchseg argument\n");
	matchsegfp = NULL;
    } else {
	if ((matchsegfp = fopen (matchsegfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchsegfile);
    }
    
    cepdir = (char *) cmd_ln_access("-cepdir");
    cepext = (char *) cmd_ln_access("-cepext");
    assert ((cepdir != NULL) && (cepext != NULL));

/* BHIKSHA: ADDING VECLEN TO ALLOW VECTORS OF DIFFERENT SIZES */
    veclen = *((int32 *) cmd_ln_access("-ceplen"));
/* END CHANGES, 6 JAN 1998, BHIKSHA */
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    if (! cmd_ln_access("-ctlcount"))
	ctlcount = 0x7fffffff;	/* All entries processed if no count specified */
    else
	ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));
    if (ctlcount == 0) {
	E_INFO("-ctlcount argument = 0!!\n");
	fclose (ctlfp);
	return err_status;
    }
    
    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    if (mllrctlfp) {
		if (fscanf (mllrctlfp, "%s", mllrfile) != 1)
		    E_FATAL ("Unexpected EOF(%s)\n", mllrctlfile);
	    }
	    --ctloffset;
	}
    }
    
    while ((ctlcount > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	printf ("\n");
	E_INFO("Utterance: %s", line);

	sf = 0;
	ef = (int32)0x7ffffff0;
	if ((k = sscanf (line, "%s %d %d %s", ctlspec, &sf, &ef, uttid)) <= 0)
	    continue;	    /* Empty line */

	if ((k == 2) || ( (k >= 3) && ((sf >= ef) || (sf < 0))) ) {
	    E_ERROR("Error in ctlfile spec; skipped\n");
	    /* What happens to ctlcount??? */
	    continue;
	}
	if (k < 4) {
	    /* Create utt-id from mfc-filename (and sf/ef if specified) */
	    for (i = strlen(ctlspec)-1; (i >= 0) && (ctlspec[i] != '/'); --i);
	    if (k == 3)
		sprintf (uttid, "%s_%d_%d", ctlspec+i+1, sf, ef);
	    else
		strcpy (uttid, ctlspec+i+1);
	}

	if (mllrctlfp) {
	    if (fscanf (mllrctlfp, "%s", mllrfile) != 1)
		E_FATAL ("Unexpected EOF(%s)\n", mllrctlfile);
	    
	    if (strcmp (prevmllr, mllrfile) != 0) {
		float32 ***A, **B;
		int32 gid, sid;
		uint8 *mgau_xform;
		
		gauden_mean_reload (g, (char *) cmd_ln_access("-mean"));
		
		if (ms_mllr_read_regmat (mllrfile, &A, &B, fcb->stream_len,feat_n_stream(fcb)) < 0)
		    E_FATAL("ms_mllr_read_regmat failed\n");
		
		mgau_xform = (uint8 *) ckd_calloc (g->n_mgau, sizeof(uint8));

		/* Transform each non-CI mixture Gaussian */
		for (sid = 0; sid < sen->n_sen; sid++) {
		    if (mdef->cd2cisen[sid] != sid) {	/* Otherwise it's a CI senone */
			gid = sen->mgau[sid];
			if (! mgau_xform[gid]) {
			    ms_mllr_norm_mgau (g->mean[gid], g->n_density, A, B,
					    fcb->stream_len,feat_n_stream(fcb));
			    mgau_xform[gid] = 1;
			}
		    }
		}

		ckd_free (mgau_xform);
		
		ms_mllr_free_regmat (A, B, fcb->stream_len,feat_n_stream(fcb));

		strcpy (prevmllr, mllrfile);
	    }
	}

	if (! feat)
	  feat = feat_array_alloc (fcb, S3_MAX_FRAMES);
	
	/* Read and process mfc/feature speech input file */
	nfr = feat_s2mfc2feat(fcb, ctlspec, cepdir, cepext,sf, ef, feat, S3_MAX_FRAMES);
	assert(feat);

	if (nfr <= 0)
	    E_ERROR("Utt %s: Input file read (%s) failed\n", uttid, cepfile);
	else {
	  /* Form full cepfile name */
	  if (ctlspec[0] != '/')
	    sprintf (cepfile, "%s/%s.%s", cepdir, ctlspec, cepext);
	  else
	    sprintf (cepfile, "%s.%s", ctlspec, cepext);

	    E_INFO ("%s: %d mfc frames\n", uttid, nfr);
	    assert(feat);
	    decode_utt (nfr, uttid);
	}
#if 0
	linklist_stats ();
#endif
	--ctlcount;
    }
    printf ("\n");

    while (fgets(line, sizeof(line), ctlfp) != NULL) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    E_INFO("Skipping rest of control file beginning with:\n\t%s", line);
	    break;
	}
    }
    
    if (matchfp)
	fclose (matchfp);
    if (matchsegfp)
	fclose (matchsegfp);

    fclose (ctlfp);
    if (mllrctlfp)
	fclose (mllrctlfp);
    
    return (err_status);
}


int main (int32 argc, char *argv[])
{
    int32 err_status;

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc,argv,"default.arg",defn);
    unlimit ();
    
    if ((cmd_ln_access("-mdef") == NULL) ||
	(cmd_ln_access("-mean") == NULL) ||
	(cmd_ln_access("-var") == NULL)  ||
	(cmd_ln_access("-mixw") == NULL)  ||
	(cmd_ln_access("-tmat") == NULL))
	E_FATAL("Missing -mdef, -mean, -var, -mixw, or -tmat argument\n");

    if ((cmd_ln_access("-dict") == NULL) ||
	(cmd_ln_access("-lm") == NULL))
	E_FATAL("Missing -dict or -lm argument\n");
    
    inlatdir = (char *) cmd_ln_access ("-inlatdir");
    outlatdir = (char *) cmd_ln_access ("-outlatdir");
    if (outlatdir) {
	int32 k;
	
	k = strlen(outlatdir);
	if ((k > 6) && (strcmp (outlatdir+(k-6), ",NODES") == 0)) {
	    outlat_onlynodes = 1;
	    outlatdir[k-6] = '\0';
	} else
	    outlat_onlynodes = 0;
    }

    if (inlatdir && outlatdir && (strcmp (inlatdir, outlatdir) == 0))
	E_FATAL("Input and output lattice directories are the same\n");

    /*
     * Initialize log(S3-base).  All scores (probs...) computed in log domain to avoid
     * underflow.  At the same time, log base = 1.0001 (1+epsilon) to allow log values
     * to be maintained in int32 variables without significant loss of precision.
     */
    {
	float32 logbase;
    
	logbase = *((float32 *) cmd_ln_access("-logbase"));
	if (logbase <= 1.0)
	    E_FATAL("Illegal log-base: %e; must be > 1.0\n", logbase);
	if (logbase > 1.01)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init ((float64) logbase);
    }

    /* Initialize feature stream type */
    /*feat_init ((char *) cmd_ln_access ("-feat"));*/

    fcb = feat_init ( (char *) cmd_ln_access ("-feat"),
		      (char *) cmd_ln_access ("-cmn"),
		      (char *) cmd_ln_access ("-varnorm"),
		      (char *) cmd_ln_access ("-agc"));
    
    /* Read in input databases */
    models_init ();
    
    /* Senone scaling factor in each frame */
    senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));

    /* Best statescore in each frame */
    bestscr = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    
    /* Allocate profiling timers and counters */
    ptmr_init(&tmr_utt);
    ptmr_init(&tmr_fwdvit);
    ptmr_init(&tmr_bstpth);
    ptmr_init(&tmr_gausen);
    ptmr_init(&tmr_fwdsrch);
    
    pctr_new(ctr_nfrm,"frm");
    pctr_new(ctr_nsen,"sen");

    /* Initialize forward Viterbi search module */
    fwd_init (mdef,tmat,dict,lm);
    printf ("\n");
    
    tot_nfr = 0;
    
    err_status = process_ctlfile ();

    if (tot_nfr > 0) {
	printf ("\n");
	printf("TOTAL FRAMES:       %8d\n", tot_nfr);
	printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	       tmr_utt.t_tot_cpu, tmr_utt.t_tot_cpu/(tot_nfr*0.01));
	printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	       tmr_utt.t_tot_elapsed, tmr_utt.t_tot_elapsed/(tot_nfr*0.01));
	fflush (stdout);
    }

#if (! WIN32)
#if defined(_SUN4)
    system("ps -el | grep s3decode");
#else
    system ("ps aguxwww | grep s3decode");
#endif
#endif


    cmd_ln_appl_exit();

    exit(err_status);
    return 0 ;
}
