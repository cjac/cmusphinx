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
 * $Log: main_dag.c,v $
 * 2008/06/20  N. Coetmeur, supervised by Y. Esteve
 * Adapt with new LM Ng model :
 * - Adjust parameters of lm_rawscore function
 * - Replace lm_read function by lm_read_advance2
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
 * 27-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporate it from s3 code base
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include "libutil.h"

#include "s3types.h"
#include "s3_dag.h"
#include "logs3.h"

#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "wid.h"


extern dict_t *dict;            /* The dictionary */
extern lm_t *lm ;               /* The lm */
extern lm_t *lm3g; /* 3g lm to compute word posteriors */
extern fillpen_t *fpen;         /* The filler penalty structure */

static s3wid_t startwid, finishwid, silwid;

static ptmr_t tm_utt;
static int32 tot_nfr;


/*
 * Command line arguments.
 */
static arg_t defn[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-lminmemory",
      ARG_INT32,
      "0",
      "Load language model into memory (default: use disk cache for lm"},
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    { "-dictfn",
      ARG_STRING,
      NULL,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdictfn",
      ARG_STRING,
      NULL,
      "Optional filler word (noise word) pronunciation dictionary input file" },
    { "-lmfn",
      ARG_STRING,
      NULL,
      "Language model input file (precompiled .DMP file)" },
    { "-lm3gPAP",
      ARG_STRING,
      NULL,
      "Trigram language model input file (precompiled .DMP file): to compute correctly word posteriors when the 4g LM uses Kneser-Ney smoothing (however, word posteriors are computed with trigrams). It's better to use non-Kneser-Ney smoothing to estimate this LM (bigrams will be used to prune the dag)" },
    { "-langwt",
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
    { "-ctlfn",
      ARG_STRING,
      NULL,
      "Input control file listing utterances to be decoded" },
    { "-ctloffset",
      ARG_INT32,
      "0",
      "No. of utterances at the beginning of -ctlfn file to be skipped" },
    { "-ctlcount",
      ARG_INT32,
      NULL,
      "No. of utterances in -ctlfn file to be processed (after -ctloffset).  Default: Until EOF" },
    { "-silpen",
      ARG_FLOAT32,
      "0.1",
      "Language model 'probability' of silence word" },
    { "-noisepen",
      ARG_FLOAT32,
      "0.05",
      "Language model 'probability' of each non-silence filler word" },
    { "-fillpenfn",
      ARG_STRING,
      NULL,
      "Filler word probabilities input file (used in place of -silpen and -noisepen)" },
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
{ "-gram",
      ARG_INT32,
      "3",
      "n-gram" },       

    { "-dumplattice",
      ARG_INT32,
      "0",
      "comme son nom l indique"},
{ "-maxedge",
      ARG_INT32,
      "2000000",
      "Max DAG edges allowed in utterance; aborted if exceeded; controls memory usage" },
    { "-maxlink",
      ARG_INT32,
      "2000000",
      "Max DAG edges after pruning by map" },
    { "-inlatdir",
      ARG_STRING,
      NULL,
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-latext",
      ARG_STRING,
      "lat.gz",
      "Word-lattice filename extension (.gz or .Z extension for compression)" },
    { "-matchfn",
      ARG_STRING,
      NULL,
      "Recognition result output file (old NIST format; pre Nov95)" },
    { "-matchsegfn",
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


/* Hacks to avoid hanging problem under Linux */
static FILE orig_stdout, orig_stderr;
static FILE *logfp;

static int32 cmdline_parse (int argc, char *argv[])
{
    int32 i;
    char *logfile;
    
    E_INFO("Parsing command line:\n");
    for (i = 0; i < argc; i++) {
	if (argv[i][0] == '-')
	    printf ("\\\n\t");
	printf ("%s ", argv[i]);
    }
    printf ("\n\n");
    fflush (stdout);

    cmd_ln_parse (defn,argc, argv, 0);

    logfp = NULL;
    if ((logfile = (char *)cmd_ln_str("-logfn")) != NULL) {
	if ((logfp = fopen(logfile, "w")) == NULL) {
	    E_ERROR("fopen(%%s,w) failed; logging to stdout/stderr\n");
	} else {
	    orig_stdout = *stdout;	/* Hack!! To avoid hanging problem under Linux */
	    orig_stderr = *stderr;	/* Hack!! To avoid hanging problem under Linux */
	    *stdout = *logfp;
	    *stderr = *logfp;
	    
	    E_INFO("Command line:\n");
	    for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-')
		    printf ("\\\n\t");
		printf ("%s ", argv[i]);
	    }
	    printf ("\n\n");
	    fflush (stdout);
	}
    }
    
    E_INFO("Configuration in effect:\n");
    /*cmd_ln_print_configuration();*/
    cmd_ln_print_help(stderr,defn);
    printf ("\n");
    
    return 0;
}


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    /* HMM model definition */


    /* Dictionary */
    dict = dict_init (NULL,
		      cmd_ln_str("-dictfn"),
		      cmd_ln_str("-fdictfn"),
		      0, 0, 0);

    /* HACK!! Make sure S3_SILENCE_WORD, S3_START_WORD and S3_FINISH_WORD are in dictionary */
    silwid = dict_wordid (dict,S3_SILENCE_WORD);
    startwid = dict_wordid (dict, S3_START_WORD);
    finishwid = dict_wordid (dict, S3_FINISH_WORD);
    if (NOT_S3WID(silwid) || NOT_S3WID(startwid) || NOT_S3WID(finishwid)) {
	E_FATAL("%s, %s, or %s missing from dictionary\n",
		S3_SILENCE_WORD, S3_START_WORD, S3_FINISH_WORD);
    }
    if ((dict->filler_start > dict->filler_end) || (! dict_filler_word (dict,silwid)))
	E_FATAL("%s must occur (only) in filler dictionary\n", S3_SILENCE_WORD);
    /* No check that alternative pronunciations for filler words are in filler range!! */

    /* LM */

    {
      char *lmfile;

      lmfile = (char *)cmd_ln_str("-lmfn");
      if (! lmfile)
	E_FATAL("-lmfn argument missing\n");

      lm = lm_read_advance2 (lmfile, "default",
		    *(float64 *)cmd_ln_access("-langwt"),
		    *(float64 *)cmd_ln_access("-inspen"),
		    *(float64 *)cmd_ln_access("-ugwt"),
			0, NULL, 1,
			cmd_ln_int32 ("-lminmemory"));



      /* Filler penalties */
      fpen = fillpen_init (dict,cmd_ln_str("-fillpenfn"),
		    *(float64 *)cmd_ln_access("-silpen"),
		    *(float64 *)cmd_ln_access("-noisepen"),
		    *(float64 *)cmd_ln_access("-langwt"),
		    *(float64 *)cmd_ln_access("-inspen"));
			
			lm->dict2lmwid = wid_dict_lm_map(dict, lm, *(float32 *)cmd_ln_access("-langwt"));
    }

		
    /* LM 3G*/
		{
			char *lmfile=NULL;

			lmfile = (char *)cmd_ln_str("-lm3gPAP");
			if (!lmfile) {
				lm3g = NULL;
				if ( *(int32 *)cmd_ln_access("-gram") == 4)
					E_WARN("-lm3gPAP argument missing: BE SURE THAT 4G LM DOESN'T USE KNESER-NEY DISCOUNTING METHOD\n");
			}
			else {

				lm3g = lm_read_advance2 (lmfile, "default",
					*(float64 *)cmd_ln_access("-langwt"),
					*(float64 *)cmd_ln_access("-inspen"),
					*(float64 *)cmd_ln_access("-ugwt"),
					0, NULL, 1,
					cmd_ln_int32 ("-lminmemory"));



				/* Filler penalties */
				fpen = fillpen_init (dict,cmd_ln_str("-fillpenfn"),
					*(float64 *)cmd_ln_access("-silpen"),
					*(float64 *)cmd_ln_access("-noisepen"),
					*(float64 *)cmd_ln_access("-langwt"),
					*(float64 *)cmd_ln_access("-inspen"));
				lm3g->dict2lmwid = wid_dict_lm_map(dict, lm3g, *(float32 *)cmd_ln_access("-langwt"));
			}
		}
			
		
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
			s3_dag_srch_hyp_t *hypptr,	/* In: Hypothesis */
			int32 nfrm)	/* In: #frames in utterance */
{
    s3_dag_srch_hyp_t *h;
    int32 ascr, lscr, tscr;
    char rep[1024], loc[1024];
    float32 debut, longu;
      sscanf(uttid,"%[^-]-%f-%f-%s",rep,&debut,&longu,loc); 
    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
	ascr += h->ascr;
	if (dict_basewid(dict,h->wid) != startwid) {
	    lscr += lm_rawscore (lm,h->lscr);
	} else {
	    assert (h->lscr == 0);
	}
	tscr += h->ascr + h->lscr;
    }

    //   fprintf (fp, "%s T %d A %d L %d", uttid, tscr, ascr, lscr);
    
    if (! hypptr)	/* HACK!! */
      //	fprintf (fp, " (null)\n")
;
    else {
      for (h = hypptr; h; h = h->next) {float duree,sf = h->sf-1; sf /= 100.0; sf += debut;
      if (h->next) duree = (h->next->sf-h->sf)/100.0 ;
      else duree = longu-sf ;
	    lscr = (dict_basewid(dict,h->wid) != startwid) ? lm_rawscore (lm,h->lscr) : 0;
	    fprintf (fp,"%s 1 %0.2f %0.2f  %7d %7d %.5f %s %s\n",rep,sf,duree, h->ascr, lscr, logs3_to_p(h->pap), dict_wordstr (dict,h->wid),uttid);
	}
	
      //	fprintf (fp, " %d\n", nfrm);
    }
    
    fflush (fp);
}


/* Write hypothesis in old (pre-Nov95) NIST format */
static void log_hypstr (FILE *fp, s3_dag_srch_hyp_t *hypptr, char *uttid, int32 scr)
{
    s3_dag_srch_hyp_t *h;
    s3wid_t w;

    if (! hypptr)	/* HACK!! */
	fprintf (fp, "(null)");
    
    for (h = hypptr; h; h = h->next) {
	w = dict_basewid (dict,h->wid);
	if ((w != startwid) && (w != finishwid) && (! dict_filler_word (dict,w)))
	    fprintf (fp, "%s ", dict_wordstr(dict,w));
    }

    if (scr != 0)
	fprintf (fp, " (%s %d)\n", uttid, scr);
    else
	fprintf (fp, " (%s)\n", uttid);

    fflush (fp);
}


/* Log hypothesis in detail with word segmentations, acoustic and LM scores  */
static void log_hyp_detailed (FILE *fp, s3_dag_srch_hyp_t *hypptr, char *uttid, char *LBL, char *lbl)
{
    s3_dag_srch_hyp_t *h;
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
    char rep[1024],loc[1024], dagfile[1024];
    s3_dag_srch_hyp_t *h, *hyp;
    char *latdir, *latext;
    int32 nfrm, ascr, lscr;


    ptmr_reset (&tm_utt);
    ptmr_start (&tm_utt);

    
    latdir = (char *)cmd_ln_str ("-inlatdir");
    latext = (char *)cmd_ln_str ("-latext");
    if (latdir){
      sscanf(uttid,"%[^-]-%s",rep,loc);
      sprintf (dagfile, "%s/%s/%s.%s", latdir,rep, uttid, latext);
    }
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
	  
	  lm_cache_stats_dump (lm);
	  lm_cache_reset (lm);
		
		if (lm3g) {
			lm_cache_stats_dump (lm3g);
			lm_cache_reset (lm3g);
		}
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
    
    dag_destroy ();

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


/* Process utterances in the control file (-ctlfn argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp, *matchfp, *matchsegfp;
    char *ctlfile;
    char *matchfile, *matchsegfile;
    char line[1024], ctlspec[1024], uttid[1024];
    int32 ctloffset, ctlcount;
    int32 i, k, sf, ef;
    
    if ((ctlfile = (char *)cmd_ln_str("-ctlfn")) == NULL)
	E_FATAL("No -ctlfn argument\n");
    
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    if ((matchfile = (char *)cmd_ln_str("-matchfn")) == NULL) {
	E_WARN("No -matchfn argument\n");
	matchfp = NULL;
    } else {
	if ((matchfp = fopen (matchfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchfile);
    }
    
    if ((matchsegfile = (char *)cmd_ln_str("-matchsegfn")) == NULL) {
	E_WARN("No -matchsegfn argument\n");
	matchsegfp = NULL;
    } else {
	if ((matchsegfp = fopen (matchsegfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchsegfile);
    }
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
	if (! cmd_ln_access("-ctlcount")->ptr)
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

    fclose (ctlfp);
}


static int32 load_argfile (char *file, char *pgm, char ***argvout)
{
    FILE *fp;
    char line[1024], word[1024], *lp, **argv;
    int32 len, n;

    E_INFO("Reading arguments from %s\n", file);

    if ((fp = fopen (file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return -1;
    }

    /* Count #arguments */
    n = 1;	/* Including pgm */
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    n++;
	}
    }
    
    /* Allocate space for arguments */
    argv = (char **) ckd_calloc (n+1, sizeof(char *));
    
    /* Create argv list */
    rewind (fp);
    argv[0] = pgm;
    n = 1;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    argv[n] = ckd_salloc (word);
	    n++;
	}
    }
    argv[n] = NULL;
    *argvout = argv;

    fclose (fp);

    return n;
}


int main (int32 argc, char *argv[])
{
    char *str;
    
#if 0
    ckd_debug(100000);
#endif
    
    /* Digest command line argument definitions */
    /*    cmd_ln_define (defn);*/

    if ((argc == 2) && (strcmp (argv[1], "help") == 0)) {
      /*cmd_ln_print_definitions();*/
      cmd_ln_print_help(stderr,defn);
	exit(1); 
    }

    /* Look for default or specified arguments file */
    str = NULL;
    if ((argc == 2) && (argv[1][0] != '-'))
	str = argv[1];
    else if (argc == 1) {
	str = "s3decode.arg";
	E_INFO("Looking for default argument file: %s\n", str);
    }
    if (str) {
	/* Build command line argument list from file */
	if ((argc = load_argfile (str, argv[0], &argv)) < 0) {
	    fprintf (stderr, "Usage:\n");
	    fprintf (stderr, "\t%s argument-list, or\n", argv[0]);
	    fprintf (stderr, "\t%s [argument-file] (default file: s3decode.arg)\n\n",
		     argv[0]);
	    /*	    cmd_ln_print_definitions();*/
	    cmd_ln_print_help(stderr,defn);
	    exit(1);

	}
    }
    
    cmdline_parse (argc, argv);

    /* Remove memory allocation restrictions */
    unlimit ();
    
#if (! WIN32)
    {
	char buf[1024];
	
	gethostname (buf, 1024);
	buf[1023] = '\0';
	E_INFO ("Executing on: %s\n", buf);
    }
#endif

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);
    
    if (
	(cmd_ln_access("-dictfn") == NULL))
	E_FATAL("Missing -mdeffn, -dictfn, or -lmfn argument\n");
    
    /*
     * Initialize log(S3-base).  All scores (probs...) computed in log domain to avoid
     * underflow.  At the same time, log base = 1.0001 (1+epsilon) to allow log values
     * to be maintained in int32 variables without significant loss of precision.
     */
    if (cmd_ln_access("-logbase") == NULL)
	logs3_init (1.0001, 0, cmd_ln_int32 ("-log3table"));
    else {
	float64 logbase;
    
	logbase = *((float64 *) cmd_ln_access("-logbase"));
	if (logbase <= 1.0)
	    E_FATAL("Illegal log-base: %e; must be > 1.0\n", logbase);
	if (logbase > 1.1)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init (logbase, 0, cmd_ln_int32 ("-log3table"));
    }
    
    /* Read in input databases */
    models_init ();

    /* Allocate timing object */
    ptmr_init(&tm_utt);
    tot_nfr = 0;
    
    /* Initialize forward Viterbi search module */
    dag_init (dict);
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
    // system ("ps auxwww | grep s3dag");
#endif
   
    /* Hack!! To avoid hanging problem under Linux */
    if (logfp) {
	fclose (logfp);
	/*	*stdout = orig_stdout;
	 *stderr = orig_stderr;*/
    }

    exit(0);
	return EXIT_SUCCESS; /* compiler is happy */
}
