/* ====================================================================
 * Copyright (c) 1993-2000 Carnegie Mellon University.  All rights 
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

/* KB.C - for compile_kb
 * 
 * 27-May-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		Included Bob Brennan's personaldic handling (similar to oovdic).
 * 
 * 09-Dec-94	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Cleaned up kb() interface; got rid of fwd-only, astar-mode
 * 		etc.
 * 
 * Revision 8.6  94/10/11  12:33:49  rkm
 * Minor changes.
 * 
 * Revision 8.5  94/07/29  11:52:10  rkm
 * Removed lmSetParameters() call; that is now part of lm_3g.c.
 * Added lm_init_oov() call to LM module.
 * Added ilm_init() call to ILM module.
 * 
 * Revision 8.4  94/05/19  14:19:12  rkm
 * Commented out computePhraseLMProbs().
 * 
 * Revision 8.3  94/04/14  14:38:01  rkm
 * Added OOV words sub-dictionary.
 * 
 * Revision 8.1  94/02/15  15:08:13  rkm
 * Derived from v7.  Includes multiple start symbols for the LISTEN
 * project.  Includes multiple LMs for grammar switching.
 * 
 * Revision 6.15  94/02/11  13:15:18  rkm
 * Initial revision (going into v7).
 * Added multiple start symbols for the LISTEN project.
 * 
 * Revision 6.14  94/01/31  16:35:17  rkm
 * Moved check for use of 8/16-bit senones on HPs to after pconf().
 * 
 * Revision 6.13  94/01/07  17:48:13  rkm
 * Added option to use trigrams in forward pass (simple implementation).
 * 
 * Revision 6.12  94/01/05  16:04:20  rkm
 * *** empty log message ***
 * 
 * Revision 6.11  94/01/05  16:02:17  rkm
 * Placed senone probs compression under conditional compilation.
 * 
 * Revision 6.10  93/12/05  17:25:46  rkm
 * Added -8bsen option and necessary datastructures.
 * 
 * Revision 6.9  93/12/04  16:24:11  rkm
 * Added check for use of -16bsen if compiled with _HPUX_SOURCE.
 * 
 * Revision 6.8  93/11/15  12:20:39  rkm
 * Added -ilmusesdarpalm flag.
 * 
 * Revision 6.7  93/11/03  12:42:35  rkm
 * Added -16bsen option to compress senone probs to 16 bits.
 * 
 * Revision 6.6  93/10/27  17:29:45  rkm
 * *** empty log message ***
 * 
 * Revision 6.5  93/10/15  14:59:13  rkm
 * Bug-fix in call to create_ilmwid_map.
 * 
 * Revision 6.4  93/10/13  16:48:16  rkm
 * Added ilm_init call to Roni's ILM and whatever else is needed.
 * Added option to process A* only (in which case phoneme-level
 * files are not loaded).  Added bigram_only argument to lm_read
 * call.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "s2types.h"
#include "CM_macros.h"
#include "basic_types.h"
#include "search_const.h"
#include "list.h"
#include "hash.h"
#include "err.h"
#include "logmsg.h"
#include "c.h"
#include "pconf.h"
#include "log.h"
#include "dict.h"
#include "msd.h"
#include "smmap4f.h"
#include "lm.h"
#include "lmclass.h"
#include "lm_3g.h"
#include "hmm_tied_r.h"
#include "scvq.h"
#include "kb.h"
#include "phone.h"
#include "fbs.h"

#define QUIT(x)		{fprintf x; exit(-1);}

static char *lm_file_name = 0;
static char *lm_ctl_filename = 0;	/* Multiple LM filenames and assoc. LM names */
static char *lmft_file_name = 0;
static char *lm_tag_file_name = 0;
static char *lm_word_tags_file_name = 0;
static char const *lm_start_sym = "<s>";
static char const *lm_end_sym = "</s>";
static char *phone_file_name = 0;
static char *mapFileName = 0;
static char *dict_file_name = 0;
static char *oov_dict_file_name = 0;
static char *personal_dict_file_name = 0;
static char *phrase_dict_file_name = 0;
static char *noise_dict_file_name = 0;
static char *hmm_dir = 0;
static char *hmm_dir_list = 0;
static char const *hmm_ext = "chmm";
static double  hmm_smooth_min = 0.0000001;
static char const *code1_ext = "ccode";
static char const *code2_ext = "d2code";
static char const *code3_ext = "p3code";
static char const *code4_ext = "xcode";
static int32  num_cb = 4;		/* Number of code books */
static char  *sendumpfile = 0;		/* Senone probs dump file */
static int32  Use8BitSenProb = FALSE;	/* TRUE=>use sen-probs compressed to 8bits */
static int32  UseDarpaStandardLM = TRUE;	/* FALSE => Use Roni Rosenfeld LM */
static int32  UseBigramOnly = FALSE;	/* Only use bigram even is trigram is avaiable */
static int32  UseWordPair = FALSE;	/* Use word pair probs only */
static int32 use_left_context = TRUE;
static float unigramWeight = 1.0;	/* Unigram wieght */
static double transSmooth = 0.0001;	/* Transition smoothing floor */
static double transWeight = 1.0;	/* Transition Weight */
static int32 NoLangModel = TRUE;
static int32 useBigHmmFiles = FALSE;	/* use big hmms files for IO */
static int32 useCiTrans = TRUE;		/* only ci transitions in hmms */
static int32 useCiPhonesOnly = FALSE;	/* only ci phones */
static int32 useWDPhonesOnly = FALSE;	/* only with in word phones */

static float silence_word_penalty;
static float insertion_penalty;
static float filler_word_penalty;
static float language_weight;
static int32 max_new_oov = 0;		/* #new OOVs that can be added at run time */
static float oov_ugprob = -4.5;		/* (Actually log10(ugprob)) of OOVs */

extern void unlimit(void);

/* LM dump directory */
static char *kb_dump_dir = NULL;

static char *startsym_file = NULL;	/* For LISTEN project */

config_t kb_param[] = {
	/*
	 * LongName, Documentation, Switch, TYPE, Address
	 */
	{ "LmFile", "Language model file name", "-lmfn",
		STRING, (caddr_t) &lm_file_name },
	{ "LmControlFile", "Language model control file name", "-lmctlfn",
		STRING, (caddr_t) &lm_ctl_filename },
	{ "LmFTFile", "Language model (forward trigram) file name", "-lmftfn",
		STRING, (caddr_t) &lmft_file_name },
	{ "LmTagFile", "Language model tag file name", "-lmtagfn",
		STRING, (caddr_t) &lm_tag_file_name },
	{ "LmWordTagFile", "Language model word tag file name", "-lmwtagfn",
		STRING, (caddr_t) &lm_word_tags_file_name },
	{ "LmStartSym", "Langauge model start symbol", "-lmstartsym",
		STRING, (caddr_t) &lm_start_sym }, 
	{ "LmEndSym", "Langauge model end symbol", "-lmendsym",
		STRING, (caddr_t) &lm_end_sym }, 
	{ "UseDarpaStandardLM", "Use DARPA standard LM", "-useDarpaLM",
		BOOL, (caddr_t) &UseDarpaStandardLM }, 
	{ "UseBigramOnly", "Only use a bigram model", "-useBigramOnly",
		BOOL, (caddr_t) &UseBigramOnly }, 
	{ "UseWordPair", "Use word pair probabilities", "-usewordpair",
		BOOL, (caddr_t) &UseWordPair }, 
	{ "MapFile", "Distribution map", "-mapfn",
		STRING, (caddr_t) &mapFileName }, 
	{ "DictFile", "Dictionary file name", "-dictfn",
		STRING, (caddr_t) &dict_file_name }, 
	{ "OOVDictFile", "OOV Dictionary file name", "-oovdictfn",
		STRING, (caddr_t) &oov_dict_file_name }, 
	{ "PersonalDictFile", "Personal Dictionary file name", "-perdictfn",
		STRING, (caddr_t) &personal_dict_file_name }, 
	{ "PhraseDictFile", "Phrase dictionary file name", "-pdictfn",
		STRING, (caddr_t) &phrase_dict_file_name }, 
	{ "NoiseDictFile", "Noise Dictionary file name", "-ndictfn",
		STRING, (caddr_t) &noise_dict_file_name }, 
	{ "StartSymbolsFile", "Start symbols file name", "-startsymfn",
		STRING, (caddr_t) &startsym_file }, 
	{ "UseLeftContext", "Use the left context models", "-useleftcontext",
		BOOL, (caddr_t) &use_left_context }, 
	{ "PhoneFile", "Phone file name", "-phnfn",
		STRING, (caddr_t) &phone_file_name }, 
	{ "UseBigHmmFiles", "Use big hmm file format", "-usebighmm",
		BOOL, (caddr_t) &useBigHmmFiles }, 
	{ "UseCITransOnly", "Only use trans probs from CI phones", "-usecitrans",
		BOOL, (caddr_t) &useCiTrans }, 
	{ "UseCIPhonesOnly", "Only use CI phones", "-useciphones",
		BOOL, (caddr_t) &useCiPhonesOnly }, 
	{ "WordPhonesOnly", "Only use with in word phones", "-usewdphones",
		BOOL, (caddr_t) &useWDPhonesOnly }, 
	{ "HmmDirList", "Hmm Directory List", "-hmmdirlist",
		STRING, (caddr_t) &hmm_dir_list }, 
	{ "HmmDir", "Hmm Directory", "-hmmdir",
		STRING, (caddr_t) &hmm_dir }, 
	{ "HmmExt", "Hmm Extention", "-hmmext",
		STRING, (caddr_t) &hmm_ext }, 
	{ "Code1Ext", "Code1 Extention", "-code1ext",
		STRING, (caddr_t) &code1_ext }, 
	{ "Code2Ext", "Code2 Extention", "-code2ext",
		STRING, (caddr_t) &code2_ext }, 
	{ "Code3Ext", "Code3 Extention", "-code3ext",
		STRING, (caddr_t) &code3_ext }, 
	{ "Code4Ext", "Code4 Extention", "-code4ext",
		STRING, (caddr_t) &code4_ext }, 
	{ "NumCb", "Number of Codebooks", "-numcb",
		INT, (caddr_t) &num_cb }, 
	{ "HmmSmoothMin", "Hmm minimum output probability", "-hmmsm",
	        DOUBLE, (caddr_t) &hmm_smooth_min }, 
	{ "TransWeight", "Arc transition weight", "-transwt",
	        DOUBLE,	(caddr_t) &transWeight }, 
	{ "TransSmooth", "Minimum arc transition probability", "-transsm,",
	        DOUBLE,	(caddr_t) &transSmooth }, 
	{ "Unigram Weight", "Unigram weight, 0.0 - 1.0", "-ugwt",
		FLOAT, (caddr_t) &unigramWeight }, 
	{ "Use8BitSenProb", "#bits to use for senone probs = 8", "-8bsen",
		BOOL, (caddr_t) &Use8BitSenProb }, 
	{ "InsertionPenalty", "Penalty for word transitions", "-inspen",
		FLOAT, (caddr_t) &insertion_penalty }, 
	{ "SilenceWordPenalty", "Penalty for silence word transitions", "-silpen",
		FLOAT, (caddr_t) &silence_word_penalty }, 
	{ "FillerWordPenalty", "Penalty for filler word transitions", "-fillpen",
		FLOAT, (caddr_t) &filler_word_penalty }, 
	{ "LanguageWeight", "Weighting on Language Probabilities", "-langwt",
		FLOAT, (caddr_t) &language_weight }, 
	{ "MaxNewOOV", "MAX New OOVs that can be added at run time", "-maxnewoov",
		INT, (caddr_t) &max_new_oov }, 
	{ "OOVUgProb", "OOV Unigram Log Prob", "-oovugprob",
		FLOAT, (caddr_t) &oov_ugprob }, 
	{ "KBDumpDirectory", "KB dump directory", "-kbdumpdir",
		STRING, (caddr_t) &kb_dump_dir }, 
	{ "SenoneProbDumpFile", "Senone Probs Dump File", "-sendumpfn",
		STRING, (caddr_t) &sendumpfile }, 
	
	{ 0,0,0,NOTYPE,0 }
};

int32  num_alphabet = NUMOFCODEENTRIES;
SMD   *smds;		  	/* Pointer to the smd's */
int32  numSmds;			/* Number of SMDs allocated */
dictT *word_dict;

static float phone_insertion_penalty;

void kbAddGrammar(char const *fileName, char const *grammarName)
{
    lmSetStartSym (lm_start_sym);
    lmSetEndSym (lm_end_sym);
    lm_read (fileName, grammarName, language_weight, unigramWeight, insertion_penalty);
}


static void kb_init_lmclass_dictwid (lmclass_t cl)
{
    lmclass_word_t w;
    int32 wid;
    
    for (w = lmclass_firstword(cl); lmclass_isword(w); w = lmclass_nextword(cl, w)) {
	wid = kb_get_word_id (lmclass_getword (w));
	lmclass_set_dictwid (w, wid);
    }
}


void kb (int argc, char *argv[],
	 float ip,	/* word insertion penalty */
	 float lw,	/* langauge weight */
	 float pip)	/* phone insertion penalty */
{
    char *pname = argv[0];
    char hmm_file_name[256];
    int32 num_phones, num_ci_phones;
    int32 i, use_darpa_lm;

    /* FIXME: This is evil.  But if we do it, let's prototype it
       somewhere, OK? */
    unlimit ();		/* Remove memory size limits */
    
    language_weight = lw;
    insertion_penalty = ip;
    phone_insertion_penalty = pip;

    pconf (argc, argv, kb_param, 0, 0, 0);

    if ((phone_file_name == 0) ||
	(dict_file_name == 0))
	pusage (pname, (Config_t *)kb_param);

    log_info("%s(%d): Reading phone file [%s]\n",
	     __FILE__, __LINE__, phone_file_name);
    if (phone_read (phone_file_name))
	exit (-1);
    if (useWDPhonesOnly)
	phone_add_diphones();

    num_ci_phones = phoneCiCount();

    /* Read the distribution map file */
    log_info("%s(%d): Reading map file [%s]\n", __FILE__, __LINE__, mapFileName);

    read_map (mapFileName, TRUE /* useCiTrans compress */);
    log_info("%s(%d): Reading dict file [%s]\n",
	     __FILE__, __LINE__, dict_file_name);

    word_dict = dict_new ();
    if (dict_read (word_dict, dict_file_name, phrase_dict_file_name, 
		   noise_dict_file_name, !useWDPhonesOnly))
	exit (-1);

    use_darpa_lm = TRUE;

    if (use_darpa_lm) {
	lmSetStartSym (lm_start_sym);
	lmSetEndSym (lm_end_sym);
	
	/*
	 * Read control file describing multiple LMs, if specified.
	 * File format (optional stuff is indicated by enclosing in []):
	 * 
	 *   [{ LMClassFileName LMClassFilename ... }]
	 *   TrigramLMFileName LMName [{ LMClassName LMClassName ... }]
	 *   TrigramLMFileName LMName [{ LMClassName LMClassName ... }]
	 *   ...
	 * (There should be whitespace around the { and } delimiters.)
	 * 
	 * This is an extension of the older format that had only TrigramLMFilenName
	 * and LMName pairs.  The new format allows a set of LMClass files to be read
	 * in and referred to by the trigram LMs.  (Incidentally, if one wants to use
	 * LM classes in a trigram LM, one MUST use the -lmctlfn flag.  It is not
	 * possible to read in a class-based trigram LM using the -lmfn flag.)
	 * 
	 * No "comments" allowed in this file.
	 */
	if (lm_ctl_filename) {
	    FILE *ctlfp;
	    char lmfile[4096], lmname[4096], str[4096];
	    lmclass_set_t lmclass_set;
	    lmclass_t *lmclass, cl;
	    int32 n_lmclass, n_lmclass_used;
	    
	    lmclass_set = lmclass_newset();
	    
	    E_INFO("Reading LM control file '%s'\n", lm_ctl_filename);
	    
	    ctlfp = CM_fopen (lm_ctl_filename, "r");
	    if (fscanf (ctlfp, "%s", str) == 1) {
		if (strcmp (str, "{") == 0) {
		    /* Load LMclass files */
		    while ((fscanf (ctlfp, "%s", str) == 1) && (strcmp (str, "}") != 0))
			lmclass_set = lmclass_loadfile (lmclass_set, str);
		    
		    if (strcmp (str, "}") != 0)
			E_FATAL("Unexpected EOF(%s)\n", lm_ctl_filename);
		    
		    if (fscanf (ctlfp, "%s", str) != 1)
			str[0] = '\0';
		}
	    } else
		str[0] = '\0';
	    
	    /* Fill in dictionary word id information for each LMclass word */
	    for (cl = lmclass_firstclass(lmclass_set);
		 lmclass_isclass(cl);
		 cl = lmclass_nextclass(lmclass_set, cl)) {
		kb_init_lmclass_dictwid (cl);
	    }

	    /* At this point if str[0] != '\0', we have an LM filename */
	    n_lmclass = lmclass_get_nclass(lmclass_set);
	    lmclass = (lmclass_t *) CM_calloc (n_lmclass, sizeof(lmclass_t));
	    
	    /* Read in one LM at a time */
	    while (str[0] != '\0') {
		strcpy (lmfile, str);
		if (fscanf (ctlfp, "%s", lmname) != 1)
		    E_FATAL("LMname missing after LMFileName '%s'\n", lmfile);
		
		n_lmclass_used = 0;
		
		if (fscanf (ctlfp, "%s", str) == 1) {
		    if (strcmp (str, "{") == 0) {
			/* LM uses classes; read their names */
			while ((fscanf (ctlfp, "%s", str) == 1) &&
			       (strcmp (str, "}") != 0)) {
			    if (n_lmclass_used >= n_lmclass)
				E_FATAL("Too many LM classes specified for '%s'\n",
					lmfile);
			    lmclass[n_lmclass_used] = lmclass_get_lmclass (lmclass_set,
									   str);
			    if (! (lmclass_isclass(lmclass[n_lmclass_used])))
				E_FATAL("LM class '%s' not found\n", str);
			    n_lmclass_used++;
			}
			if (strcmp (str, "}") != 0)
			    E_FATAL("Unexpected EOF(%s)\n", lm_ctl_filename);
			
			if (fscanf (ctlfp, "%s", str) != 1)
			    str[0] = '\0';
		    }
		} else
		    str[0] = '\0';
		
		if (n_lmclass_used > 0)
		    lm_read_clm (lmfile, lmname,
				 language_weight, unigramWeight, insertion_penalty,
				 lmclass, n_lmclass_used);
		else
		    lm_read (lmfile, lmname,
			     language_weight, unigramWeight, insertion_penalty);
	    }
	    
	    fclose (ctlfp);
	    NoLangModel = FALSE;
	}
	
	/* Read "base" LM file, if specified */
	if (lm_file_name) {
	    lmSetStartSym (lm_start_sym);
	    lmSetEndSym (lm_end_sym);
	    lm_read (lm_file_name, "", language_weight, unigramWeight, insertion_penalty);

	    /* Make initial OOV list known to this base LM */
	    lm_init_oov ();

	    NoLangModel = FALSE;
	}

#ifdef USE_ILM
	/* Init ILM module (non-std-Darpa LM, eg ug/bg cache LM) */
	ilm_init ();
#endif
    }

#if 0
    /* Compute the phrase lm probabilities */
    computePhraseLMProbs ();
#endif

    num_phones = phone_count ();
    numSmds = hmm_num_sseq();
    smds = (SMD *) CM_calloc (numSmds, sizeof (SMD));
    
    /*
     * Read the hmm's into the SMD structures
     */
    if (useBigHmmFiles) {
	for (i = 0; i < num_ci_phones; i++) {
	    sprintf (hmm_file_name, "%s.%s", phone_from_id (i),
		     hmm_ext);
	    
	    hmm_tied_read_big_bin (hmm_dir_list, hmm_file_name, smds,
				   transSmooth, NUMOFCODEENTRIES, TRUE,
				   transWeight);
	}
    } else {
	for (i = 0; i < num_phones; i++) {
	    if ((!useCiTrans) || (phone_id_to_base_id(i) == i)) {
		sprintf (hmm_file_name, "%s.%s", phone_from_id (i), hmm_ext);
		hmm_tied_read_bin (hmm_dir_list, hmm_file_name,
				   &smds[hmm_pid2sid(i)], transSmooth,
				   NUMOFCODEENTRIES, TRUE, transWeight);
	    }
	}
    }
    
    /*
     *  Use Ci transitions ?
     */
    if (useCiTrans) {
	for (i = 0; i < num_phones; i++) {
	    if (hmm_pid2sid(phone_id_to_base_id(i)) != hmm_pid2sid(i)) {
		/*
		 * Just make a copy of the CI phone transitions
		 */
		memcpy (&smds[hmm_pid2sid(i)], &smds[hmm_pid2sid(phone_id_to_base_id(i))],
			sizeof (SMD));
	    }
	}
    }
    /*
     * Read the distributions
     */
    read_dists (hmm_dir, code1_ext, code2_ext, code3_ext, code4_ext,
		NUMOFCODEENTRIES, hmm_smooth_min, useCiPhonesOnly);
    if (Use8BitSenProb)
	SCVQSetSenoneCompression (8);
    
    /*
     * Map the distributions to the correct locations
     */
    remap (smds);
}

#if 0
computePhraseLMProbs ()
{
    int32	wcnt = dict_count (word_dict);
    int32	i;
    char	stmp[256];
    char	*p, *q, *r;

    for (i = 0; i < wcnt; i++) {
	int32	prob = 0;
	/*
 	 * Is this a phrase word ?
	 */
	if (word_dict->dict_list[i]->wid != word_dict->dict_list[i]->fwid) {
	    strcpy (stmp, word_dict->dict_list[i]->word);
	    
	    q = stmp;
	    p = index (q, '_');
	    if (p) {
		*p = '\0';
		p++;
	    }
	    while (q && p) {
		r = index (p, '_');
		if (r) {
		    *r = '\0';
		    r++;
		}
		/*
	         * Look out for alternate pronuciations in phrases and strip the
	         * modifier
	         */
		{
	            char *lp = rindex(p, '(');
	            char *rp = rindex(p, ')');
		    if (lp && rp) {
			*lp = '\0';
		    }
		}
		prob += lmLogProbability (langModel, q, p);
		q = p;
		p = r;
	    }

	    log_info ("Phrase log prob [%20s] = %8d\n",
		    word_dict->dict_list[i]->word, prob);
	    word_dict->dict_list[i]->lm_pprob = prob;
	}
    }
}
#endif

extern int32 totalDists;
int32 kb_get_total_dists (void)
{
    return totalDists;  
}
 
int32 kb_get_aw_tprob (void)
/*------------------------------------------------------------*
 * Return the All_Word Transition Probability
 */
{
    return  (language_weight *
		(LOG(1.0 / word_dict->dict_entry_count) -
			LOG(insertion_penalty)));
}

int32 kb_get_num_models (void)
/*------------------------------------------------------------*
 * Return the number of unique hmm models.
 */
{
    return hmm_num_sseq();
}

int32 kb_get_num_dist (void)
/*------------------------------------------------------------*
 * Return the number of distributions.
 */
{
    return 5;
}

int32 kb_get_num_model_instances (void)
/*------------------------------------------------------------*
 * Return the number of hmm model instances.
 */
{
    register int32 i, tot = 0;

    for (i = 0; i < word_dict->dict_entry_count; i++) {
	tot += word_dict->dict_list[i]->len + 10;
    }
    return tot;
}

int32 kb_get_num_words (void)
/*------------------------------------------------------------*
 * Return the number of words.
 */
{
    return word_dict->dict_entry_count;
}

SMD *kb_get_models (void)
/*------------------------------------------------------------*
 * Return the a pointer to the hmm models
 */
{
    return smds;
}

char **kb_get_phone_list (void)
/*------------------------------------------------------------*
 * Return the phone list.
 */
{
    return ((char **) phoneList()->list);
}

extern int32 *Out_Prob1;
extern int32 *Out_Prob2;
extern int32 *Out_Prob3;
extern int32 *Out_Prob4;

typedef struct {
    int32 **prob;           /* 2-D array, #codewords x 256 */
    unsigned char **id;     /* 2-D array, #codewords x #senones */
} OPDF_8BIT_T;
extern OPDF_8BIT_T out_prob_8b[];

int32 *kb_get_codebook_0_dist (void)
/*------------------------------------------------------------*
 * Return the permutted codebook 0 distributions.
 */
{
    /* UGLY coercion of various pointer types to (int32 *) */
    if (Use8BitSenProb)
	return ((int32 *) &(out_prob_8b[0]));
    return (Out_Prob1);
}

int32 *kb_get_codebook_1_dist (void)
/*------------------------------------------------------------*
 * Return the permutted codebook 1 distributions.
 */
{
    /* UGLY coercion of various pointer types to (int32 *) */
    if (Use8BitSenProb)
	return ((int32 *) &(out_prob_8b[1]));
    return (Out_Prob2);
}

int32 *kb_get_codebook_2_dist (void)
/*------------------------------------------------------------*
 * Return the permutted codebook 2 distributions.
 */
{
    /* UGLY coercion of various pointer types to (int32 *) */
    if (Use8BitSenProb)
	return ((int32 *) &(out_prob_8b[2]));
    return (Out_Prob3);
}

int32 *kb_get_codebook_3_dist (void)
/*------------------------------------------------------------*
 * Return the permutted codebook 3 distributions.
 */
{
    /* UGLY coercion of various pointer types to (int32 *) */
    if (Use8BitSenProb)
	return ((int32 *) &(out_prob_8b[3]));
    return (Out_Prob4);
}

int32 kb_get_dist_prob_bytes (void)
/*------------------------------------------------------------*
 * Return kbh.dist_prob_bytes
 */
{
     return 4;
}

int32 kb_get_word_id (char const *word)
{
#if 0
    static char const *rname = "kb_get_word_id";
    register int32 i, id = -1;

    for (i = 0; i < word_dict->dict_entry_count; i++) {
	if (mystrcasecmp (word, word_dict->dict_list[i]->word) == 0) {
	   id = i;
	   break;
	}
    }
    if (id == -1) {
	log_warn(stdout, "%s: Couldn't find word \"%s\"\n", rname, word);
    }

    return id;
#else
    return (dict_to_id (word_dict, word));
#endif
}

char *kb_get_word_str (int32 wid)
{
    return (word_dict->dict_list[wid]->word);
}

dictT *kb_get_word_dict (void)
{
    return word_dict;
}

int32 kb_get_num_codebooks (void)
{
    return  4;
}

int kb_get_darpa_lm_flag (void)
{
    return UseDarpaStandardLM;
}

int kb_get_no_lm_flag (void)
{
    return NoLangModel;
}

char const *kb_get_lm_start_sym(void)
{
   return lm_start_sym;
}

char const *kb_get_lm_end_sym(void)
{
   return lm_end_sym;
}

char *kb_get_dump_dir(void)
{
    return kb_dump_dir;
}

char *kb_get_senprob_dump_file(void)
{
    return sendumpfile;
}

int32 kb_get_senprob_size(void)
{
    if (Use8BitSenProb)
	return 8;
    return 32;
}

/* For LISTEN project */
char *kb_get_startsym_file (void)
{
    return startsym_file;
}

char *kb_get_oovdic(void)
{
    return oov_dict_file_name;
}

char *kb_get_personaldic(void)
{
    return personal_dict_file_name;
}

double kb_get_oov_ugprob (void)
{
    return (oov_ugprob);
}

int32 kb_get_max_new_oov (void)
{
    return (max_new_oov);
}

int32 dict_maxsize (void)
{
    return (word_dict->dict.size_hint);
}
