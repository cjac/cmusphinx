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
 * astar-main.c -- Driver for N-best list creation from DAGs using A* algorithm.
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
 * $Log$
 * Revision 1.4  2004/11/16  05:13:19  arthchan2003
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
 * Revision 1.3  2004/10/07 22:46:26  dhdfu
 * Fix compiler warnings that are also real bugs (but why does this
 * function take an int32 when -lw is a float parameter?)
 *
 * Revision 1.2  2004/09/13 08:13:28  arthchan2003
 * update copyright notice from 200x to 2004
 *
 * Revision 1.1  2004/08/09 05:15:23  arthchan2003
 * incorporate s3.0 astar
 *
 *
 * 26-Jun-2004  ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporate it from CarlQ's patch.
 * 
 * Revision 1.1  2003/02/12 16:30:42  cbq
 * A simple n-best program for sphinx.
 *
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * 
 * 31-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added compound words handling option.  Compound words are broken up
 * 		into component words for computing LM probabilities.
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice and nbest files.
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxedge argument to control memory usage.
 * 		Added -maxlmop and -maxlpf options to control execution time.
 * 		Added -maxppath option to control CPU/memory usage.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 * 
 * 29-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from nbest-main.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <libutil/libutil.h>

#include <s3types.h>
#include <logs3.h>
#include <tmat.h>
#include <mdef.h>
#include <dict.h>
#include <lm.h>
#include <fillpen.h>
#include <search.h>
#include <hyp.h>
#include <wid.h>

static mdef_t *mdef;		/* Model definition */

static s3wid_t startwid, finishwid, silwid;

static ptmr_t tm_utt;		/* Entire utterance */

extern dict_t *dict;		/* The dictionary	*/
extern lm_t *lm;		/* The lm.		*/
extern fillpen_t *fpen;		/* The filler penalty structure. */
extern s3lmwid_t *dict2lmwid;	/* Mapping from decoding dictionary wid's to lm ones.  They may not be the same! */

void nbest_search (char *filename, char *uttid);
int32 dag_load (char *file);
int32 dag_destroy ( void );
void nbest_init ( void );

/*
 * Command line arguments.
 */
static arg_t defn[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0003",
      "Base in which all log values calculated" },
    { "-lminmemory",
      ARG_INT32,
      "0",
      "Load language model into memory (default: use disk cache for lm"},
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    { "-mdef",
      ARG_STRING,
      NULL,
      "Model definition input file: triphone -> senones/tmat tying" },
    { "-dict",
      ARG_STRING,
      NULL,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdict",
      ARG_STRING,
      NULL,
      "Optional filler word (noise word) pronunciation dictionary input file" },
    { "-compwd",
      ARG_INT32,
      "0",
      "Whether compound words should be broken up internally into component words" },
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
    { "-inspen",
      ARG_FLOAT32,
      "0.65",
      "Word insertion penalty" },
    { "-beam",
      ARG_FLOAT64,
      "1e-64",
      "Partial path pruned if below beam * score of best partial ppath so far" },
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
    { "-maxlpf",
      ARG_INT32,
      "40000",
      "Max LMops/frame after which utterance aborted; controls CPU use (see maxlmop)" },
    { "-maxlmop",
      ARG_INT32,
      "100000000",
      "Max LMops in utterance after which it is aborted; controls CPU use (see maxlpf)" },
    { "-maxedge",
      ARG_INT32,
      "2000000",
      "Max DAG edges allowed in utterance; aborted if exceeded; controls memory usage" },
    { "-maxppath",
      ARG_INT32,
      "1000000",
      "Max partial paths created after which utterance aborted; controls CPU/memory use" },
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
      0,
      "No. of utterances in -ctl file to be processed (after -ctloffset).  Default: Until EOF" },
    { "-inlatdir",
      ARG_STRING,
      NULL,
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-latext",
      ARG_STRING,
      "lat.gz",
      "Word-lattice filename extension (.gz or .Z extension for compression)" },
    { "-nbestdir",
      ARG_STRING,
      ".",
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-nbestext",
      ARG_STRING,
      "nbest.gz",
      "N-best filename extension (.gz or .Z extension for compression)" },
    { "-nbest",
      ARG_INT32,
      "200",
      "Max. n-best hypotheses to generate per utterance" },
    { "-min_endfr",
      ARG_INT32,
      "3",
      "Nodes ignored during search if they persist for fewer than so many end frames" },
    { "-dagfudge",
      ARG_INT32,
      "2",
      "Adjacency fudge (#frames) between nodes in DAG (0..2)" },
    { "-ppathdebug",
      ARG_INT32,
      "0",
      "Adjacency fudge (#frames) between nodes in DAG (0..2)" },
    { "-logfn",
      ARG_STRING,
      NULL,
      "Log file (default stdout/stderr)" },
    
    { NULL, ARG_INT32, NULL, NULL }
};

/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdef"));

    /* Dictionary */
    dict = dict_init (mdef, (char *) cmd_ln_access("-dict"),
		      (char *) cmd_ln_access("-fdict"), 0);

    /* HACK!! Make sure SILENCE_WORD, START_WORD and FINISH_WORD are in dictionary */
    silwid = dict_wordid (dict, S3_SILENCE_WORD);
    startwid = dict_wordid (dict, S3_START_WORD);
    finishwid = dict_wordid (dict, S3_FINISH_WORD);
    if (NOT_S3WID(silwid) || NOT_S3WID(startwid) || NOT_S3WID(finishwid)) {
	E_FATAL("%s, %s, or %s missing from dictionary\n",
		S3_SILENCE_WORD, S3_START_WORD, S3_FINISH_WORD);
    }
    if ((dict->filler_start > dict->filler_end) || (! dict_filler_word (dict, silwid)))
	E_FATAL("%s must occur (only) in filler dictionary\n", S3_SILENCE_WORD);
    /* No check that alternative pronunciations for filler words are in filler range!! */

    /* LM */
    {
	char *lmfile;
	
	lmfile = (char *) cmd_ln_access("-lm");
	if (! lmfile)
	    E_FATAL("-lm argument missing\n");
	lm = lm_read (lmfile, *(float32 *)cmd_ln_access("-lw"),
	    		      *(float32 *)cmd_ln_access("-inspen"),
			      *(float32 *)cmd_ln_access("-ugwt"));

	fpen = fillpen_init (dict, (char *) cmd_ln_access("-fillpen"),
		      *(float32 *)cmd_ln_access("-silpen"),
		      *(float32 *)cmd_ln_access("-noisepen"),
		      *(float32 *)cmd_ln_access("-lw"),
		      *(float32 *)cmd_ln_access("-inspen"));
    }

    dict2lmwid = wid_dict_lm_map(dict, lm, *(float32 *)cmd_ln_access("-lw"));
}


/* Decode the given mfc file and write result to given directory */
static void decode_utt (char *uttid, char *nbestdir)
{
    char dagfile[1024], nbestfile[1024];
    char *latdir, *latext, *nbestext;
    int32 nfrm;
    
    latdir = (char *) cmd_ln_access ("-inlatdir");
    latext = (char *) cmd_ln_access ("-latext");
    nbestext = (char *) cmd_ln_access ("-nbestext");
    if (latdir)
	sprintf (dagfile, "%s/%s.%s", latdir, uttid, latext);
    else
	sprintf (dagfile, "%s.%s", uttid, latext);
    
    ptmr_reset (&tm_utt);
    ptmr_start (&tm_utt);

    if ((nfrm = s3astar_dag_load (dagfile)) > 0) {
	sprintf (nbestfile, "%s/%s.%s", nbestdir, uttid, nbestext);
	nbest_search (nbestfile, uttid);
	
	lm_cache_stats_dump (lm);
	lm_cache_reset (lm);
    } else
	E_ERROR("Dag load (%s) failed\n", uttid);
    dag_destroy ();

    ptmr_stop (&tm_utt);
    
    printf ("%s: TMR: %5d Frm", uttid, nfrm);
    if (nfrm > 0) {
	printf (" %6.2f xEl", tm_utt.t_elapsed * 100.0 / nfrm);
	printf (" %6.2f xCPU", tm_utt.t_cpu * 100.0 / nfrm);
    }
    printf ("\n");
    fflush (stdout);
}


/* Process utterances in the control file (-ctl argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp;
    char *ctlfile, *nbestdir;
    char line[1024], ctlspec[1024], uttid[1024];
    int32 ctloffset, ctlcount;
    int32 i, k, sf, ef;
    
    if ((ctlfile = (char *) cmd_ln_access("-ctl")) == NULL)
	E_FATAL("No -ctl argument\n");
    
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    if (! cmd_ln_access("-ctlcount"))
	ctlcount = 0x7fffffff;	/* All entries processed if no count specified */
    else
	ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));
    if (ctlcount == 0) {
	E_INFO("-ctlcount argument = 0!!\n");
	fclose (ctlfp);
	return;
    }
    
    nbestdir = (char *) cmd_ln_access ("-nbestdir");

    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0)
	    --ctloffset;
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

	decode_utt (uttid, nbestdir);
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

    fclose (ctlfp);
}

int
main (int32 argc, char *argv[])
{
  /*  kb_t kb;
      ptmr_t tm;*/

  char *str;
  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);
  unlimit ();
    
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
	if (logbase > 1.1)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init ((float64) logbase);
    }
    
    /* Read in input databases */
    models_init ();

    /* Initialize forward Viterbi search module */
    nbest_init ();
    printf ("\n");

    ptmr_init (&tm_utt);
    
    process_ctlfile ();

#if (! WIN32)
    system ("ps aguxwww | grep s3astar");
#endif

    cmd_ln_appl_exit();
    return 0;
}

