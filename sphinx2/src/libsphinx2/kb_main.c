/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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

/* KB.C - for compile_kb
 * 
 * $Log$
 * Revision 1.11  2004/12/10  16:48:56  rkm
 * Added continuous density acoustic model handling
 * 
 * 
 * 02-Dec-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added acoustic score weight (applied only to S3 continuous
 * 		acoustic models).
 * 
 * 22-Nov-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Incorporated continuous acoustic model handling.
 * 
 * 06-Aug-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added phonetp (phone transition probs matrix) for use in
 * 		allphone search.
 * 
 * 27-May-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		Included Bob Brennan's personaldic handling (similar to 
 *              oovdic).
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
#include <assert.h>

#include "s2types.h"
#include "CM_macros.h"
#include "basic_types.h"
#include "cont_mgau.h"
#include "search_const.h"
#include "list.h"
#include "hash.h"
#include "err.h"
#include "ckd_alloc.h"
#include "c.h"
#include "pconf.h"
#include "log.h"
#include "logs3.h"
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
#include "s3mdef_s2map.h"


static char *fsg_ctlfile_name = NULL;
static char *fsg_file_name = NULL;
/*
 * If (fsg_use_altpron) consider alternative pronunciations in addition to the
 * words explicitly mentioned in an FSG
 */
static int32 fsg_use_altpron = TRUE;
/*
 * If (fsg_use_filler) transparently insert silence and other filler words at
 * each state of the FSG.
 */
static int32 fsg_use_filler = TRUE;

static char *lm_file_name = NULL;
static char *lm_ctl_filename = NULL;	/* Multiple LM filenames and assoc. LM names */
static char *lmft_file_name = NULL;
static char *lm_tag_file_name = NULL;
static char *lm_word_tags_file_name = NULL;
static char const *lm_start_sym = "<s>";
static char const *lm_end_sym = "</s>";
static char *phone_file_name = NULL;
static char *mapFileName = NULL;
static char *mdefFileName = NULL;
static char *meanFileName = NULL;
static char *varFileName = NULL;
static char *mixwFileName = NULL;
static float32 s3varfloor = 0.0001f;
static float32 s3mixwfloor = 0.0000001f;
static char *dict_file_name = NULL;
static char *oov_dict_file_name = NULL;
static char *personal_dict_file_name = NULL;
static char *phrase_dict_file_name = NULL;
static char *noise_dict_file_name = NULL;
static char *phonetp_file_name = NULL;
static float32 ptplw = 5.0f;		/* Pulled out of thin air */
static float32 uptpwt = 0.001f;		/* Pulled out of thin air */
static char *hmm_dir = NULL;
static char *hmm_dir_list = NULL;
static char const *hmm_ext = "chmm";
static double  hmm_smooth_min = 0.0000001f;
static char const *code1_ext = "ccode";
static char const *code2_ext = "d2code";
static char const *code3_ext = "p3code";
static char const *code4_ext = "xcode";
static int32  num_cb = 4;		/* Number of code books */
static char  *sendumpfile = NULL;	/* Senone probs dump file */
static int32  Use8BitSenProb = FALSE;	/* TRUE=>use sen-probs compressed to 8bits */
static int32  UseDarpaStandardLM = TRUE;	/* FALSE => Use Roni Rosenfeld LM */
static int32  UseBigramOnly = FALSE;	/* Only use bigram even is trigram is avaiable */
static int32  UseWordPair = FALSE;	/* Use word pair probs only */
static int32 use_left_context = TRUE;
static float unigramWeight = 1.0f;	/* Unigram wieght */
static double transSmooth = 0.0001f;	/* Transition smoothing floor */
static double transWeight = 1.0f;	/* Transition Weight */
static int32 NoLangModel = TRUE;
static int32 useBigHmmFiles = FALSE;	/* use big hmms files for IO */
static int32 useCiTrans = TRUE;		/* only ci transitions in hmms */
static int32 useCiPhonesOnly = FALSE;	/* only ci phones */
static int32 useWDPhonesOnly = FALSE;	/* only with in word phones */

static float silence_word_penalty = 0.005f;
static float insertion_penalty = 0.65f;
static float filler_word_penalty = 1e-8f;

/* Per frame penalty for filler words; no lang wt applied to this */
static float filler_pfpen = 1.0f;	/* Default: no penalty (0 => infinite) */

static float language_weight = 9.5f;
static int32 ascr_scale = 0;		/* Acoustic score scaling: SCALED DOWN by so
					   many bits.  Applicable to S3 continuous
					   models only, not semi-continuous models */
static int32 max_new_oov = 0;		/* #new OOVs that can be added at run time */
static float oov_ugprob = -4.5f;	/* (Actually log10(ugprob)) of OOVs */

extern void unlimit(void);

/* LM dump directory */
static char *kb_dump_dir = NULL;

static char *startsym_file = NULL;	/* For LISTEN project */

config_t kb_param[] = {
	/*
	 * LongName, Documentation, Switch, TYPE, Address
	 */
	{ "FSGFile", "FSG language model file name", "-fsgfn",
		STRING, (caddr_t) &fsg_file_name },
	{ "FSGCtlFile", "FSG language model control file name", "-fsgctlfn",
		STRING, (caddr_t) &fsg_ctlfile_name },
	{ "FSGUseAltPron", "Use alternative pronunciations for FSG", "-fsgusealtpron",
		BOOL, (caddr_t) &fsg_use_altpron }, 
	{ "FSGUseFiller", "Insert filler words at each state", "-fsgusefiller",
		BOOL, (caddr_t) &fsg_use_filler }, 
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
	{ "LmStartSym", "Language model start symbol", "-lmstartsym",
		STRING, (caddr_t) &lm_start_sym }, 
	{ "LmEndSym", "Language model end symbol", "-lmendsym",
		STRING, (caddr_t) &lm_end_sym }, 
	{ "UseDarpaStandardLM", "Use DARPA standard LM", "-useDarpaLM",
		BOOL, (caddr_t) &UseDarpaStandardLM }, 
	{ "UseBigramOnly", "Only use a bigram model", "-useBigramOnly",
		BOOL, (caddr_t) &UseBigramOnly }, 
	{ "UseWordPair", "Use word pair probabilities", "-usewordpair",
		BOOL, (caddr_t) &UseWordPair }, 
	{ "S3MdefFile", "S3 Model Definition File", "-mdeffn",
		STRING, (caddr_t) &mdefFileName }, 
	{ "S3MeanFile", "S3 Gauden Means File", "-meanfn",
		STRING, (caddr_t) &meanFileName }, 
	{ "S3VarFile", "S3 Gauden Variances File", "-varfn",
		STRING, (caddr_t) &varFileName }, 
	{ "S3MixwFile", "S3 Mixture Weights File", "-mixwfn",
		STRING, (caddr_t) &mixwFileName }, 
	{ "S3VarFloor", "Variance Floor for S3 var file", "-varfloor",
		FLOAT, (caddr_t) &s3varfloor },
	{ "S3MixwFloor", "Mixture Weights Floor for S3 mixw file", "-mixwfloor",
		FLOAT, (caddr_t) &s3mixwfloor },
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
	{ "PhoneTransCountsFile", "Phone transition counts file", "-phonetpfn",
		STRING, (caddr_t) &phonetp_file_name }, 
	{ "PhoneTPLanguageWeight", "Weighting on Phone Transition Probabilities", "-ptplw",
		FLOAT, (caddr_t) &ptplw }, 
	{ "UniformPTPWeight", "Unigram phone transition prob weight", "-uptpwt",
		FLOAT, (caddr_t) &uptpwt },
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
	{ "FillerPerFramePenalty", "Per frame penalty for filler words", "-fpfpen",
		FLOAT, (caddr_t) &filler_pfpen }, 
	{ "LanguageWeight", "Weighting on Language Probabilities", "-langwt",
		FLOAT, (caddr_t) &language_weight }, 
	{ "AcousticWeight", "Weighting on Acoustic Scores", "-ascrwt",
		INT, (caddr_t) &ascr_scale }, 
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

/* Phone transition LOG(probability) matrix */
static int32 **phonetp;
static float phone_insertion_penalty;

/* S3 model definition to S2 mapping */
static s3mdef_s2map_t *s3mdef_s2map = NULL;

/* S3 (continuous) acoustic model */
static mgau_model_t *mgau = NULL;

/*
 * Array for storing s3 senone scores in any frame.
 * S3 senone order different from S2 order
 */
static int32 *s3senscr = NULL;

/*
 * Array for s3 format feature vector in any frame.
 * S3 feature vector = cep[12], dcep[12], pow[3], ddcep[12] concatenated
 * into a single vector.
 */
static float32 *s3feat = NULL;


int32 kb_get_silence_word_id ( void )
{
  return kb_get_word_id("SIL");
}


int32 kb_get_silence_ciphone_id ( void )
{
  return phone_to_id("SIL", TRUE);
}


float32 kb_get_silpen ( void )
{
  return silence_word_penalty;
}


float32 kb_get_fillpen ( void )
{
  return filler_word_penalty;
}


float32 kb_get_pip ( void )
{
  return phone_insertion_penalty;
}


float32 kb_get_wip ( void )
{
  return insertion_penalty;
}


float32 kb_get_lw ( void )
{
  return language_weight;
}


int32 kb_get_ascr_scale ( void )
{
  return ascr_scale;
}


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


static void phonetp_load_file (char *file, int32 **tp)
{
  FILE *fp;
  char line[16384], p1[4096], p2[4096];
  int32 i, j, k, n;
  
  E_INFO("Reading phone transition counts file '%s'\n", file);
  fp = CM_fopen(file, "r");
  
  while (fgets (line, sizeof(line), fp) != NULL) {
    if (line[0] == '#')
      continue;
    
    k = sscanf (line, "%s %s %d", p1, p2, &n);
    if ((k != 0) && (k != 3))
      E_FATAL("Expecting 'srcphone dstphone count'; found:\n%s\n", line);
    
    i = phone_to_id(p1, TRUE);
    j = phone_to_id(p2, TRUE);
    if ((i == NO_PHONE) || (j == NO_PHONE))
      E_FATAL("Unknown src or dst phone: %s or %s\n", p1, p2);
    if (n < 0)
      E_FATAL("Phone transition count cannot be < 0:\n%s\n", line);
    
    tp[i][j] = n;
  }
  
  fclose (fp);
}


static void phonetp_dump (int32 **tp, int32 np)
{
  int32 i, j;
  
  E_INFO("Phone transition prob LOGprobs:\n");
  for (i = 0; i < np; i++) {
    for (j = 0; j < np; j++) {
      E_INFOCONT ("\t%s\t%s\t%10d\n", phone_from_id(i), phone_from_id(j), tp[i][j]);
    }
  }
}


void kb (int argc, char *argv[],
	 float ip,	/* word insertion penalty */
	 float lw,	/* language weight */
	 float pip)	/* phone insertion penalty */
{
    char *pname = argv[0];
    char hmm_file_name[256];
    int32 num_phones, num_ci_phones;
    int32 i, j, n, use_darpa_lm;
    float32 p, uptp;
    int32 logp;
    char filename[4096];
    mdef_t *mdef;
    
    /* FIXME: This is evil.  But if we do it, let's prototype it
       somewhere, OK? */
    unlimit ();		/* Remove memory size limits */
    
    language_weight = lw;
    insertion_penalty = ip;
    phone_insertion_penalty = pip;

    pconf (argc, argv, kb_param, 0, 0, 0);
    
    /* This test can be A LOT more comprehensive! :) */
    if (((phone_file_name == NULL) && (mdefFileName == NULL))
	|| (dict_file_name == NULL)) {
      pusage (pname, (Config_t *)kb_param);
    }
    
    if (mdefFileName != NULL) {
      if (((phone_file_name != NULL) && (mapFileName == NULL))
	  || ((phone_file_name == NULL) && (mapFileName != NULL))) {
	E_FATAL("Must specify both map/phone files, or neither\n");
      }
    }
    
    if (mdefFileName != NULL) {
      /*
       * mdefFileName != NULL indicates use of S3 (continuous) instead of S2
       * (semi-continuous) models.  Still, unfortunately, the S2 equivalent
       * of the mdef file (map/phone files) is also needed.  These can be
       * specified via the usual -phnfn and -mapfn flags.  However, if they
       * are omitted, the map and phone files are automatically generated.
       * If the -kbdumpdir flag is specified, the generated files are written
       * to that directory, otherwise to /tmp.  The map/phone filenames are
       * __s3_s2__.{phone,map}.
       * The generated files can be saved and used in subsequent runs that
       * deploy the same model configuration.
       */
      logs3_init (1.0001);	/* Hack!! Hardwired constant */
      
      if ((meanFileName == NULL)
	  || (varFileName == NULL)
	  || (mixwFileName == NULL)) {
	E_FATAL("No S3 mean/var/mixw files specified\n");
      }
      
      /* Read S3 model definition (mdef) and build s2 mapping */
      s3mdef_s2map = s3mdef_s2map_init (mdefFileName);
      
      if (phone_file_name == NULL) {
	/* Write temporary phone file */
	if (kb_dump_dir != NULL)
	  sprintf (filename, "%s/__s3_s2__.phone", kb_dump_dir);
	else
	  strcpy (filename, "/tmp/__s3_s2__.phone");
	phone_file_name = ckd_salloc (filename);
	if (s2phonefile_write (s3mdef_s2map, phone_file_name) < 0)
	  E_FATAL("Cannot write phone-file... exiting\n");
      }
      
      if (mapFileName == NULL) {
	/* Write temporary map file */
	if (kb_dump_dir != NULL)
	  sprintf (filename, "%s/__s3_s2__.map", kb_dump_dir);
	else
	  strcpy (filename, "/tmp/__s3_s2__.map");
	mapFileName = ckd_salloc (filename);
	if (s2mapfile_write (s3mdef_s2map, mapFileName) < 0)
	  E_FATAL("Cannot write map-file... exiting\n");
      }
      
      /* Read S3 format acoustic model files */
      mgau = mgau_init (meanFileName, varFileName, s3varfloor,
			mixwFileName, s3mixwfloor,
			TRUE);
      
      /* Allocate senone scores array */
      s3senscr = (int32 *) CM_calloc (mgau_n_mgau(mgau), sizeof(int32));
      
      /* Allocate feature vector array (cep,dcep,pow,ddcep concatenated) */
      s3feat = (float32 *) CM_calloc (CEP_VECLEN * 3, sizeof(float32));
    }
    
    E_INFO("Reading phone file [%s]\n", phone_file_name);
    if (phone_read (phone_file_name))
	exit (-1);
    if (useWDPhonesOnly)
	phone_add_diphones();
    
    num_ci_phones = phoneCiCount();
    
    /* Read the distribution map file */
    E_INFO("Reading map file [%s]\n", mapFileName);
    read_map (mapFileName, TRUE /* useCiTrans compress */);
    
    if (mdefFileName != NULL) {
      E_INFO("Checking mdef and phone/map files for consistency\n");
      
      /* A few sanity checks between mdef and phone/map files */
      mdef = s3mdef_s2map->s3mdef;
      
      if (mdef_n_ciphone(mdef) != phoneCiCount())
	E_FATAL("#CIphones(%s) = %d(S3); #CIphones(%s) = %d(S2)\n",
		mdefFileName, mdef_n_ciphone(mdef),
		phone_file_name, phoneCiCount());
      
      j = 0;
      for (i = 0; i < mdef_n_ciphone(mdef); i++) {
	if (strcmp (mdef_ciphone_str(mdef, i), phone_from_id(i)) != 0) {
	  E_ERROR("Phone mismatch: '%s'(S3) vs '%s'(S2)\n",
		  mdef_ciphone_str(mdef, i), phone_from_id(i));
	  j++;
	}
      }
      if (j > 0)
	E_FATAL("%d CIphones mismatched between mdef and phone/map\n", j);
      
      if (mdef_n_phone(mdef) != phone_count())
	E_FATAL("#Phones(%s) = %d(S3); #Phones(%s) = %d(S2)\n",
		mdefFileName, mdef_n_phone(mdef),
		phone_file_name, phone_count());
      
      if (mdef_n_sseq(mdef) != hmm_num_sseq())
	E_FATAL("#SSeq(%s) = %d(S3); #SSeq(%s) = %d(S2)\n",
		mdefFileName, mdef_n_sseq(mdef),
		mapFileName, hmm_num_sseq());
      
      /* Other checks can be added here */
    }
    
    E_INFO("Reading dict file [%s]\n", dict_file_name);
    word_dict = dict_new ();
    if (dict_read (word_dict, dict_file_name, phrase_dict_file_name, 
		   noise_dict_file_name, !useWDPhonesOnly)) {
      exit (-1);
    }
    
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

    if (mdefFileName == NULL) {		/* Only if not using S3 models */
      /*
       * Read the distributions
       */
      read_dists (hmm_dir, code1_ext, code2_ext, code3_ext, code4_ext,
		  NUMOFCODEENTRIES, hmm_smooth_min, useCiPhonesOnly);
      if (Use8BitSenProb)
	SCVQSetSenoneCompression (8);
    }
    
    /*
     * Map the distributions to the correct locations
     */
    remap (smds);
    
    /*
     * Create phone transition logprobs matrix
     */
    phonetp = (int32 **) CM_2dcalloc(num_ci_phones, num_ci_phones, sizeof(int32));
    if (phonetp_file_name) {
      /* Load phone transition counts file */
      phonetp_load_file (phonetp_file_name, phonetp);
    } else {
      /* No transition probs file specified; use uniform probs */
      for (i = 0; i < num_ci_phones; i++) {
	for (j = 0; j < num_ci_phones; j++) {
	  phonetp[i][j] = 1;
	}
      }
    }
    /* Convert counts to probs; smooth; convert to LOG-probs; apply lw/pip */
    for (i = 0; i < num_ci_phones; i++) {
      n = 0;
      for (j = 0; j < num_ci_phones; j++)
	n += phonetp[i][j];
      assert (n >= 0);
      
      if (n == 0) {	/* No data here, use uniform probs */
	p = 1.0 / (float32)num_ci_phones;
	p *= pip;	/* Phone insertion penalty */
	logp = (int32)(LOG(p) * ptplw);
	
	for (j = 0; j < num_ci_phones; j++)
	  phonetp[i][j] = logp;
      } else {
	uptp = 1.0 / (float32)num_ci_phones;	/* Uniform prob trans prob*/
	
	for (j = 0; j < num_ci_phones; j++) {
	  p = ((float32)phonetp[i][j] / (float32)n);
	  p = ((1.0 - uptpwt) * p) + (uptpwt * uptp);	/* Smooth */
	  p *= pip;	/* Phone insertion penalty */
	  
	  phonetp[i][j] = (int32)(LOG(p) * ptplw);
	}
      }
    }
#if 0
    phonetp_dump(phonetp, num_ci_phones);
#endif
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

	    E_INFO ("Phrase log prob [%20s] = %8d\n",
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
	E_WARN("%s: Couldn't find word \"%s\"\n", rname, word);
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


char *kb_get_fsg_file_name ( void )
{
    return fsg_file_name;
}


char *kb_get_fsg_ctlfile_name ( void )
{
    return fsg_ctlfile_name;
}


int32 query_fsg_use_altpron ( void )
{
  return fsg_use_altpron;
}


int32 query_fsg_use_filler ( void )
{
  return fsg_use_filler;
}


int32 **kb_get_phonetp ( void )
{
  return phonetp;
}


float32 kb_get_filler_pfpen ( void )
{
  return filler_pfpen;
}


mgau_model_t *kb_s3model ( void )
{
  return mgau;
}


int32 *kb_s3senscr ( void )
{
  return s3senscr;
}


float32 *kb_s3feat ( void )
{
  return s3feat;
}


int32 *kb_s3_s2_senmap ( void )
{
  return s3mdef_s2map->s2map;
}


int32 *kb_s2_s3_senmap ( void )
{
  return s3mdef_s2map->s3map;
}
