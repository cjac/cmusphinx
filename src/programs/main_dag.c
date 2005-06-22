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
 * dag-main.c -- Driver for DAG search.
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
 * 
 * 
 * 27-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporate it from s3 code base
 *
 * 
 * $Log$
 * Revision 1.10  2005/06/22  05:39:13  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 * 
 * Revision 1.11  2005/06/19 03:58:17  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.10  2005/06/18 20:05:24  archan
 * Sphinx3 to s3.generic: Set lm correctly in dag.c and astar.c.  Same changes should also be applied to decode_anytopo.
 *
 * Revision 1.9  2005/06/18 03:23:13  archan
 * Change to lmset interface.
 *
 * Revision 1.8  2005/06/17 23:46:06  archan
 * Sphinx3 to s3.generic 1, Remove bogus log messages in align and allphone, 2, Unified the logbase value from 1.0001 to 1.0003
 *
 * Revision 1.7  2005/06/15 05:32:25  archan
 * (Tested) fixed compilation problem of dag_destroy.
 *
 * Revision 1.6  2005/06/03 06:45:30  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.5  2005/06/03 06:12:57  archan
 * 1, Simplify and unify all call of logs3_init, move warning when logbase > 1.1 into logs3.h.  2, Change arguments to require arguments in align and astar.
 *
 * Revision 1.4  2005/05/27 01:15:45  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.3  2005/04/21 23:50:27  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.2  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:01  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.8  2004/12/06 11:31:47  arthchan2003
 * Fix brief comments for programs.
 *
 * Revision 1.7  2004/12/06 11:15:11  arthchan2003
 * Enable doxygen in the program directory.
 *
 * Revision 1.6  2004/12/05 12:01:32  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.5  2004/11/16 05:13:19  arthchan2003
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
 * Revision 1.4  2004/10/07 22:46:26  dhdfu
 * Fix compiler warnings that are also real bugs (but why does this
 * function take an int32 when -lw is a float parameter?)
 *
 * Revision 1.3  2004/09/13 08:13:28  arthchan2003
 * update copyright notice from 200x to 2004
 *
 * Revision 1.2  2004/09/09 20:29:08  arthchan2003
 * Added test for astar and dag.  Put a hack in s3_dag.c that allows 0 as acoustice score.
 *
 * Revision 1.1  2004/08/20 08:25:19  arthchan2003
 * Sorry, I forget to add the main of dag.c
 *
 * Revision 1.2  2002/12/03 23:02:37  egouvea
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
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxedge argument to control memory usage.
 * 
 * 21-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxlmop and -maxlpf options to control execution time.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the meaning of -matchsegfn and, correspondingly, log_hypseg().
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added BSTXCT: reporting since that became available from dag_search.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added fillpen_init() and removed explicit addition of SILENCE_WORD,
 * 		START_WORD and FINISH_WORD to the dictionary.
 * 
 * 03-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Creating.
 */

/** \file main_dag.c
 * \brief main driver for DAG and find the best path. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <s3types.h>
#include "s3_dag.h"
#include "dag.h"
#include "logs3.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "wid.h"


static mdef_t *mdef;		/* Model definition */
extern dict_t *dict;            /* The dictionary */


#if 0
extern lm_t *lm ;               /* The lm */
extern s3lmwid_t *dict2lmwid;   /* Mapping from decoding dictionary wid's to lm  */
#endif

extern fillpen_t *fpen;         /* The filler penalty structure */
extern dag_t dag;
extern lmset_t *lmset;          /* The lmset. Replace lm */

static ptmr_t tm_utt;
static int32 tot_nfr;


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
      REQARG_STRING,
      NULL,
      "Model definition input file: triphone -> senones/tmat tying" },
    { "-dict",
      REQARG_STRING,
      NULL,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdict",
      ARG_STRING,
      NULL,
      "Optional filler word (noise word) pronunciation dictionary input file" },
    { "-lm",
      ARG_STRING,
      NULL,
      "Language model input file (precompiled .DMP file)" },
    { "-lmctlfn",
      ARG_STRING,
      NULL,
      "Language model control file (for class-based language model)" },
    { "-lmname",
      ARG_STRING,
      NULL,
      "Name of language model in -lmctlfn to use for all utterances" },
    { "-ctl_lm",
      ARG_STRING,
      NULL,
      "List of language model to use for each utterance (one line per utt)" },
    { "-lmdumpdir",
      ARG_STRING,
      NULL,
      "The directory for dumping the DMP file. "},
    { "-lw",
      ARG_FLOAT32,
      "9.5",
      "Language weight: empirical exponent applied to LM probabilty" },
    { "-uw",
      ARG_FLOAT32,
      "0.7",
      "LM unigram weight: unigram probs interpolated with uniform distribution with this weight" },
    { "-wip",
      ARG_FLOAT32,
      "0.65",
      "Word insertion penalty" },
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
    { "-silprob",
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
      "Filler word probabilities input file (used in place of -silprob and -noisepen)" },
    { "-min_endfr",
      ARG_INT32,
      "3",
      "Nodes ignored during search if they persist for fewer than so many end frames" },
    { "-dagfudge",
      ARG_INT32,
      "2",
      "(0..2); 1 or 2: add edge if endframe == startframe; 2: if start == end-1" },
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
    { "-inlatdir",
      ARG_STRING,
      NULL,
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-latext",
      ARG_STRING,
      "lat.gz",
      "Word-lattice filename extension (.gz or .Z extension for compression)" },
    { "-match",
      ARG_STRING,
      NULL,
      "Recognition result output file (old NIST format; pre Nov95)" },
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
    
    { NULL, ARG_INT32,  NULL, NULL }
};

/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdef"),1);

    /* Dictionary */
    dict = dict_init (mdef,
		      (char *) cmd_ln_access("-dict"),
		      (char *) cmd_ln_access("-fdict"),
		      0,
		      1);

    /* LM Set */
    lmset=lmset_init(cmd_ln_str("-lm"),
		     cmd_ln_str("-lmctlfn"),
		     cmd_ln_str("-ctl_lm"),
		     cmd_ln_str("-lmname"),
		     cmd_ln_str("-lmdumpdir"),
		     cmd_ln_float32("-lw"),
		     cmd_ln_float32("-wip"),
		     cmd_ln_float32("-uw"),
		     dict);

    /* Filler penalties */
    fpen = fillpen_init (dict,(char *) cmd_ln_access("-fillpen"),
			 *(float32 *)cmd_ln_access("-silprob"),
			 *(float32 *)cmd_ln_access("-noisepen"),
			 *(float32 *)cmd_ln_access("-lw"),
			 *(float32 *)cmd_ln_access("-wip"));
    
}


/*
 * Write exact hypothesis.  Format:
 *   <id> T <scr> A <ascr> L <lscr> {<sf> <wascr> <wlscr> <word>}... <ef>
 * where:
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
			int32 nfrm)	/* In: #frames in utterance */
{
    srch_hyp_t *h;
    int32 ascr, lscr, tscr;
    
    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
	ascr += h->ascr;
	if (dict_basewid(dict,h->wid) != dict->startwid) {
	    lscr += lm_rawscore (lmset->cur_lm,h->lscr, 1.0);
	} else {
	    assert (h->lscr == 0);
	}
	tscr += h->ascr + h->lscr;
    }

    fprintf (fp, "%s T %d A %d L %d", uttid, tscr, ascr, lscr);
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, " (null)\n");
    else {
	for (h = hypptr; h; h = h->next) {
	    lscr = (dict_basewid(dict,h->wid) != dict->startwid) ? lm_rawscore (lmset->cur_lm,h->lscr, 1.0) : 0;
	    fprintf (fp, " %d %d %d %s", h->sf, h->ascr, lscr, dict_wordstr (dict,h->wid));
	}
	fprintf (fp, " %d\n", nfrm);
    }
    
    fflush (fp);
}


/* Write hypothesis in old (pre-Nov95) NIST format */
static void log_hypstr (FILE *fp, srch_hyp_t *hypptr, char *uttid, int32 scr)
{
    srch_hyp_t *h;
    s3wid_t w;

    if (! hypptr)	/* HACK!! */
	fprintf (fp, "(null)");
    
    for (h = hypptr; h; h = h->next) {
	w = dict_basewid (dict,h->wid);
	if ((w != dict->startwid) && (w != dict->finishwid) && (! dict_filler_word (dict,w)))
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
    int32 ascr_norm, lscr;

    ascr_norm = 0;
    lscr = 0;
    
    fprintf (fp, "%s:%s> %20s %5s %5s %11s %10s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr(Norm)", "LMScore");
    
    for (h = hypptr; h; h = h->next) {
	fprintf (fp, "%s:%s> %20s %5d %5d %11d %10d\n", lbl, uttid,
		 h->word, h->sf, h->ef, h->ascr, h->lscr);

	ascr_norm += h->ascr;
	lscr += h->lscr;
    }

    fprintf (fp, "%s:%s> %20s %5s %5s %11d %10d\n", LBL, uttid,
	     "TOTAL", "", "", ascr_norm, lscr);
}


/* Decode the given mfc file and write result to matchfp and matchsegfp */
static void decode_utt (char *uttid, FILE *matchfp, FILE *matchsegfp)
{
    char dagfile[1024];
    srch_hyp_t *h, *hyp;
    char *latdir, *latext;
    int32 nfrm, ascr, lscr;


    ptmr_reset (&tm_utt);
    ptmr_start (&tm_utt);

    
    latdir = (char *) cmd_ln_access ("-inlatdir");
    latext = (char *) cmd_ln_access ("-latext");
    if (latdir)
	sprintf (dagfile, "%s/%s.%s", latdir, uttid, latext);
    else
	sprintf (dagfile, "%s.%s", uttid, latext);

    
    if ((nfrm = s3dag_dag_load (dagfile)) >= 0) {
	hyp = s3dag_dag_search (uttid);
	if(hyp!=NULL){
	  if ( *((int32 *) cmd_ln_access("-backtrace")) )
	    log_hyp_detailed (stdout, hyp, uttid, "BP", "bp");
	  
	  /* Total scaled acoustic score and LM score */
	  ascr = lscr = 0;
	  for (h = hyp; h; h = h->next) {
	    ascr += h->ascr;
	    lscr += h->lscr;
	  }
	  
	  printf ("BSTPTH: ");
	  log_hypstr (stdout, hyp, uttid, ascr+lscr);
	  
	  printf ("BSTXCT: ");
	  log_hypseg (uttid, stdout, hyp, nfrm);
	  
	  lm_cache_stats_dump (lmset->cur_lm);
	  lm_cache_reset (lmset->cur_lm);
	}else{
	  E_ERROR("DAG search (%s) failed\n", uttid);
	  hyp = NULL;
	}
    } else {
	E_ERROR("DAG search (%s) failed\n", uttid);
	hyp = NULL;
    }

    
    /* Log recognition output to the standard match and matchseg files */
    if (matchfp)
	log_hypstr (matchfp, hyp, uttid, 0);
    if (matchsegfp)
	log_hypseg (uttid, matchsegfp, hyp, nfrm);
    
    dag_destroy (&dag);

    ptmr_stop (&tm_utt);
    
    printf ("%s: TMR: %5d Frm", uttid, nfrm);
    if (nfrm > 0) {
	printf (" %6.2f xEl", tm_utt.t_elapsed * 100.0 / nfrm);
	printf (" %6.2f xCPU", tm_utt.t_cpu * 100.0 / nfrm);
    }
    printf ("\n");
    fflush (stdout);

    tot_nfr += nfrm;
}


/* Process utterances in the control file (-ctl argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp, *ctllmfp, *matchfp, *matchsegfp;
    char *ctlfile, *ctllmfile;
    char *matchfile, *matchsegfile;
    char line[1024], ctlspec[1024], uttid[1024], lmname[1024];
    int32 ctloffset, ctlcount;
    int32 i, k, sf, ef;
    
    if ((ctlfile = (char *) cmd_ln_access("-ctl")) == NULL)
	E_FATAL("No -ctl argument\n");
    
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    ctllmfile = (char *) cmd_ln_access("-ctl_lm");
    if (ctllmfile) {
	if ((ctllmfp = fopen(ctllmfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", ctllmfile);
    }
    else
	ctllmfp = NULL;

    if ((matchfile = (char *) cmd_ln_access("-match")) == NULL) {
	E_WARN("No -match argument\n");
	matchfp = NULL;
    } else {
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

    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0)
	    --ctloffset;
	if (fgets(line, sizeof(line), ctllmfp) == NULL)
	    E_FATAL("File size mismatch between %s and %s\n",
		    ctlfile, ctllmfile);
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

	if (ctllmfp) {
	    fgets(line, sizeof(line), ctllmfp);
	    if (sscanf(line, "%s", lmname) > 0)
	      lmset_set_curlm_wname(lmset,lmname);
	    /*		s3dag_set_lm(lmname);*/
	}

	decode_utt (uttid, matchfp, matchsegfp);

	--ctlcount;
    }
    printf ("\n");

    if (fscanf (ctlfp, "%s", line) == 1)
	E_INFO("Skipping rest of control file beginning with:\n\t%s\n", line);

    if (matchfp)
	fclose (matchfp);
    if (matchsegfp)
	fclose (matchsegfp);
    if (ctllmfp)
	fclose (ctllmfp);
    fclose (ctlfp);
}

int main (int32 argc, char *argv[])
{
  /*  kb_t kb;
      ptmr_t tm;*/

  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);

  unlimit ();

  logs3_init ((float64) cmd_ln_float32("-logbase"),1,cmd_ln_int32("-log3table"));
    
  /* Read in input databases */
  models_init ();
  
  /* Allocate timing object */
  ptmr_init(&tm_utt);
  tot_nfr = 0;
    
  /* Initialize forward Viterbi search module */
  s3_dag_init (dict);
  printf ("\n");
  
  process_ctlfile ();

  printf ("\n");
  printf("TOTAL FRAMES:       %8d\n", tot_nfr);
  if (tot_nfr > 0) {
    printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	   tm_utt.t_tot_cpu, tm_utt.t_tot_cpu/(tot_nfr*0.01));
    printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	   tm_utt.t_tot_elapsed, tm_utt.t_tot_elapsed/(tot_nfr*0.01));
  }
  fflush (stdout);

#if (! WIN32)
  system ("ps auxwww | grep s3dag");
#endif

    /* Hack!! To avoid hanging problem under Linux */
  
  cmd_ln_appl_exit();
    
  return 0;
}
