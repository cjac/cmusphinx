/* MAIN.C - for FBS (Fast Beam Search)
 *-----------------------------------------------------------------------*
 * USAGE
 *	fbs -help
 *
 * DESCRIPTION
 *	This program allows you to run the beam search in either single
 * sentence or batch mode.
 *
 *-----------------------------------------------------------------------*
 * HISTORY
 *
 * $Log$
 * Revision 1.5  2001/12/07  04:27:35  lenzo
 * License cleanup.  Remove conditions on the names.  Rationale: These
 * conditions don't belong in the license itself, but in other fora that
 * offer protection for recognizeable names such as "Carnegie Mellon
 * University" and "Sphinx."  These changes also reduce interoperability
 * issues with other licenses such as the Mozilla Public License and the
 * GPL.  This update changes the top-level license files and removes the
 * old license conditions from each of the files that contained it.
 * All files in this collection fall under the copyright of the top-level
 * LICENSE file.
 * 
 * Revision 1.4  2001/10/23 22:20:30  lenzo
 * Change error logging and reporting to the E_* macros that call common
 * functions.  This will obsolete logmsg.[ch] and they will be removed
 * or changed in future versions.
 *
 * Revision 1.3  2001/05/12 15:20:14  dhdfu
 * Crap!  Yes, it *is* supposed to persist between control file commands.
 * Sorry, folks :(
 *
 * Revision 1.2  2001/02/13 19:51:38  lenzo
 * *** empty log message ***
 *
 * Revision 1.1  2000/12/12 23:30:00  lenzo
 * *** empty log message ***
 *
 * Revision 1.4  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.3  2000/03/29 14:30:28  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/02/08 21:06:46  lenzo
 * More to get the sphinx2-phone example working. Also
 * removed a couple of vestigial files from the demo lm.
 *
 * Revision 1.1.1.1  2000/01/28 22:08:52  lenzo
 * Initial import of sphinx2
 *
 *
 * 
 * 06-Jan-99	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added set_adc_input().
 * 		Fixed call to utt_file2feat to use mfcfile instead of utt.
 * 		Changed build_uttid to return the built id string.
 * 
 * 05-Jan-99	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added uttproc_parse_ctlfile_entry().
 * 
 * 21-Oct-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Made file extension on ctlfn entries optional.
 * 
 * 19-Oct-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added uttproc_set_logfile().
 * 
 * 10-Sep-98	M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 		Wrote Nbest list to stdout if failed to open .hyp file.
 * 		Added "-" (use stdin) special case to -ctlfn option.
 * 
 * 10-Sep-98	M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 		Moved run_allphone_utt to uttproc.c as uttproc_allphone_cepfile, with
 * 		minor modifications to allow calls from outside the libraries.
 * 
 * 19-Nov-97	M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 		Added return-value check from SCVQInitFeat().
 * 
 * 22-Jul-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -samp argument for sampling rate.
 * 
 * 22-May-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Included Bob Brennan's code for quoted strings in argument list.
 * 
 * 10-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -nbest option in batch mode.
 * 
 * 02-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added time_align_phone and time_align_state flags.
 * 
 * 12-Jul-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Changed use_3g_in_fwd_pass default to TRUE.
 * 
 * 02-Jul-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added allphone handling.
 * 
 * 16-Jun-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added handling of #comment lines in argument files.
 * 
 * 14-Jun-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added backslash option in building filenames (for PC compatibility).
 * 
 * 13-Jun-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Modified to conform to the new, simplified uttproc interface.
 * 
 * 09-Dec-94	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 *		Added code to handle flat forward pass after tree forward pass.
 * 		Added code to handle raw speech input in batch mode.
 * 		Moved the early cep preprocessing code to cep2feat.c.
 * 
 * Revision 8.8  94/10/11  12:38:45  rkm
 * Added backtrace argument.
 * 
 * Revision 8.7  94/07/29  11:56:59  rkm
 * Added arguments for ILM ug and bg cache parameters.
 * 
 * Revision 8.6  94/05/19  14:21:11  rkm
 * Minor changes to statistics format.
 * 
 * Revision 8.5  94/04/22  13:56:04  rkm
 * Cosmetic changes to various global variables and run-time arguments.
 * 
 * Revision 8.4  94/04/14  14:43:03  rkm
 * Added second pass option for lattice-rescoring.
 * 
 * Revision 8.1  94/02/15  15:09:47  rkm
 * Derived from v7.  Includes multiple start symbols for the LISTEN
 * project.  Includes multiple LMs for grammar switching.
 * 
 * Revision 6.15  94/02/11  13:13:29  rkm
 * Initial revision (going into v7).
 * Added multiple start symbols for the LISTEN project.
 * 
 * Revision 6.14  94/01/07  17:47:12  rkm
 * Added option to use trigrams in forward pass (simple implementation).
 * 
 * Revision 6.13  94/01/05  16:14:02  rkm
 * Added option to report alternative pronunciations in match file.
 * Output just the marker in match file if speech file could not be found.
 * 
 * Revision 6.12  93/12/04  16:37:57  rkm
 * Added ifndef _HPUX_SOURCE around getrusage.
 * 
 * Revision 6.11  93/11/22  11:39:55  rkm
 * *** empty log message ***
 * 
 * Revision 6.10  93/11/15  12:20:56  rkm
 * Added Mei-Yuh's handling of wsj1Sent organization.
 * 
 * Revision 6.9  93/11/03  12:42:52  rkm
 * Added -latsize option to specify BP and FP table sizes to allocate.
 * 
 * Revision 6.8  93/10/29  11:40:42  rkm
 * Added QUIT definition.
 * 
 * Revision 6.7  93/10/29  10:21:40  rkm
 * *** empty log message ***
 * 
 * Revision 6.6  93/10/28  18:06:04  rkm
 * Added -fbdumpdir flag.
 * 
 * Revision 6.5  93/10/27  17:41:00  rkm
 * *** empty log message ***
 * 
 * Revision 6.4  93/10/13  16:59:34  rkm
 * Added -ctlcount, -astaronly, and -svadapt options.
 * Added --END-OF-DOCUMENT-- option within .ctl files for Roni's ILM.
 * 
 * Revision 6.3  93/10/09  17:03:17  rkm
 * 
 * Revision 6.2  93/10/06  11:09:43  rkm
 * M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * Darpa Trigram LM module added.
 * 
 *
 *	Spring, 89 - Fil Alleva (faa) at Carnegie Mellon
 *		Created
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#ifdef WIN32
#include <fcntl.h>
#else
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _SUN4
#include <unistd.h>
#endif

#include <stdlib.h>

#include "c.h"
#include "s2types.h"
#include "CM_macros.h"
#include "basic_types.h"
#include "strfuncs.h"
#include "list.h"
#include "hash.h"
#include "search_const.h"
#include "msd.h"
#include "pconf.h"
#include "scvq.h"
#include "dict.h"
#include "err.h"
#include "lmclass.h"
#include "lm_3g.h"
#include "lm.h"
#include "kb.h"
#include "time_align.h"
#include "fbs.h"
#include "search.h"
#include "cepio.h"

#include "s2params.h"


#define QUIT(x)		{fprintf x; exit(-1);}

/* Default parameter initialization
 *----------------------------------*/
static char *ctl_file_name = 0;
static char *match_file_name = NULL;
static char *matchseg_file_name = NULL;
static char *logfn_arg = NULL;
static char *correct_file_name = 0;
static char *data_directory = 0;
static char *cepdir = 0;
static char *seg_data_directory = 0;
static char const *sent_directory = ".";
static char *utterance = 0;
static int32 phone_conf = 0;
static int32 pscr2lat = 0;

static int32 nbest = 0;			/* #N-best hypotheses to generate/utterance */
static char const *nbest_dir = ".";
static char const *nbest_ext = "hyp";

static char const *cbdir = "./";	/* Code book dir */
static char const *ccbfn = "cep.256";	/* Cepstrum Codebook file name */
static char const *dcbfn = "d2cep.256";	/* Diff Cepstrum Codebook file name */
static char const *pcbfn = "p3cep.256";	/* Power Codebook file name */
static char const *xcbfn = "xcep.256";	/* Xcode Codebook file name */

static float Cep_Floor  = 0.0001;
static float Dcep_Floor = 0.0001;
static float Pow_Floor  = 0.0001;

static int32 scVqTopN = 4;	/* Number of semi-contnuous entries to use */
static int32 ctl_offset = 0;	/* No. of lines to skip at start of ctlfile */
static int32 ctl_incr = 1;	/* Do every nth line in the ctl file */
static int32 ctl_count = 0x7fffffff;	/* #lines to be processed */

static char *force_str = 0;
static char ref_sentence[2048];

static char const *exts[4];
static char const *cext = "CCODE";
static char const *dext = "DCODE";
static char const *pext = "PCODE";
static char const *xext = NULL;
static char const *cep_ext = "mfc";
static char const *sent_ext = "sent";
static float beam_width = 1e-6;
static float new_phone_beam_width = 1e-6;
static float last_phone_beam_width = 1e-5;
static float lastphone_alone_beam_width  = 3e-4;
static float new_word_beam_width  = 3e-4;
static float fwdflat_beam_width = 1e-8;
static float fwdflat_new_word_beam_width  = 3e-4;
static float filler_word_penalty = 1e-8;
static float silence_word_penalty = 0.005;
static float phone_insertion_penalty = 1.0;
static float insertion_penalty = 0.65;
static float fwdtree_lw = 6.5;
static float fwdflat_lw = 8.5;
static float bestpath_lw = 9.5;
static float nw_pen = 1.0;

static int32 fwdtree_flag = TRUE;
static int32 fwdflat_flag = TRUE;
static int32 bestpath_flag = TRUE;
static int32 forward_only = FALSE;

static int32 live = FALSE;

static int32 forceRec = FALSE;
static int32 agcNoise = FALSE;
static int32 agcBeta = FALSE;
static int32 agcMax = FALSE;
static int32 agcEMax = FALSE;
static int32 normalizeMean = TRUE;
static int32 normalizeMeanPrior = FALSE;
static int32 compress = FALSE;
static int32 compress_prior = FALSE;
static float agcThresh = 0.2;
static int32 wsj1Sent = FALSE;
static int32 use20msDiffPow = FALSE;
static double dcep80msWeight = 1.0;

static int32 writeScoreInMatchFile = TRUE;

/* if (skip_alt_frm) skip alternate frames in cross-phone transitions */
static int32 skip_alt_frm = 0;

/* BPTable size to allocate, use some default if max-int */
static int32 lattice_size = 50000;

/* # LM cache lines to allocate */
static int32 lm_cache_lines = 100;

extern int32 use_3g_in_fwd_pass;
int32 use_3g_in_fwd_pass = TRUE;

/* Phone Eval state */
static char *time_align_ctl_file_name = NULL;
int32 time_align_word = TRUE;
int32 time_align_phone = TRUE;
int32 time_align_state = FALSE;

/* State segmentation file (seg file) extension */
static char *seg_file_ext = NULL;

/* State-by-state score */
static char *score_file_ext = NULL;

/* For saving phone labels in alignment */
char const *phonelabdirname = NULL;
char const *phonelabextname = "lab";
char const *phonelabtype = "xlabel";

/* For saving word labels in alignment */
char const *wordlabdirname = NULL;
char const *wordlabextname = "wrd";
char const *wordlabtype = "xlabel";

/* "Best" alternative word sent output file */
static char *out_sent_filename = NULL;

static int32 allphone_mode = FALSE;

/*
 * Top senones window (#frames) for predicting phone transitions.
 * If 1, do not use top senones to predict phones, transition to all.
 */
static int32 topsen_window = 1;
static int32 topsen_thresh = -60000;

/* Local Storage
 *---------------*/
static float TotalElapsedTime;
static float TotalCPUTime;
static float TotalSpeechTime;


/* Report actual pronunciation in output; default = report base pronunciation */
static int32 report_altpron = FALSE;

/* If (! compute_all_senones) compute only those needed by active channels */
static int32 compute_all_senones = FALSE;

/* Directory/Filenames for raw A/D and mfc data (written in live mode) */
static char *rawlogdir = NULL;
static char *mfclogdir = NULL;

static int32 sampling_rate = 16000;
static int32 adc_input = FALSE;	/* TRUE <=> input utterances are raw A/D data */
static char const *adc_ext = "raw";	/* Default format: raw */
static int32 adc_endian = 1;	/* Default endian: little */
static int32 adc_hdr = 0;	/* Default adc file header size */
static int32 blocking_ad_read_p = FALSE;

/* For LISTEN project: LM and startword names for different utts */
char const *utt_lmname_dir = ".";
char const *lmname_ext = "lmname";
static char const *startWord_directory = ".";
static char const *startWord_ext = "start";
static char startWord[1024] = "";

static char *dumplat_dir = NULL;

static char *cdcn_file = NULL;

static char utt_name[1024] = "";

#ifdef USE_ILM
/* Default weighting for ILM ug and bg cache probs (%) */
static int32 ilm_ugcache_wt = 4;	/* ie 0.04 */
static int32 ilm_bgcache_wt = 9;	/* ie 0.09 */
#endif

static float *cep, *dcep, *dcep_80ms, *pcep, *ddcep;

int32 print_back_trace = FALSE;
static char *arg_file = NULL;
int32 verbosity_level = 0;

#ifndef WIN32
extern double MakeSeconds(struct timeval *, struct timeval *);
#endif

extern int32 uttproc_set_cmn (scvq_norm_t n);
extern int32 uttproc_set_agc (scvq_agc_t a);
extern int32 uttproc_set_silcmp (scvq_compress_t c);

/* FIXME: These misc functions need a header file. */
extern void unlimit(void);
int awriteshort (char const *file, short *data, int length);

extern void allphone_init(double, double, double); /* dubya, dubya, dubya? */

/* Parameters for the search
 *---------------------------*/
extern config_t kb_param[];
config_t param[] = {
	/*
	 * LongName, Documentation, Switch, TYPE, Address
	 */
	{ "Force", "Force", "-force",
		STRING, (caddr_t) &force_str }, 

	{ "ArgFile", "Cmd line argument file", "-argfile",
		STRING, (caddr_t) &arg_file }, 

	{ "AllPhoneMode", "All Phone Mode", "-allphone",
		BOOL, (caddr_t) &allphone_mode }, 

	{ "ForceRec", "ForceRec", "-forceRec",
		BOOL, (caddr_t) &forceRec }, 

	{ "AgcBeta", "Use beta based AGC", "-agcbeta",
		BOOL, (caddr_t) &agcBeta }, 

	{ "AgcMax", "Use max based AGC", "-agcmax",
		BOOL, (caddr_t) &agcMax }, 

	{ "AgcEMax", "Use another max based AGC", "-agcemax",
		BOOL, (caddr_t) &agcEMax }, 

	{ "AgcNoise", "Use Noise based AGC", "-agcnoise",
		BOOL, (caddr_t) &agcNoise }, 

	{ "AgcThreshold", "Threshold for Noise based AGC", "-agcthresh",
		FLOAT, (caddr_t) &agcThresh }, 

	{ "NormalizeMean", "Normalize the feature means to 0.0", "-normmean",
		BOOL, (caddr_t) &normalizeMean }, 

	{ "NormalizeMeanPrior", "Normalize feature means with prior mean", "-nmprior",
		BOOL, (caddr_t) &normalizeMeanPrior }, 

	{ "CompressBackground", "Compress excess background frames", "-compress",
		BOOL, (caddr_t) &compress }, 

	{ "CompressPrior", "Compress excess background frames based on prior utt", "-compressprior",
		BOOL, (caddr_t) &compress_prior }, 

	{ "Dcep80msWeight", "Weight for dcep80ms", "-dcep80msweight",
		DOUBLE, (caddr_t) &dcep80msWeight }, 

	{ "LiveData", "Get input from A/D hardware", "-live",
		BOOL, (caddr_t) &live }, 

	{ "A/D blocks on read", "A/D blocks on read", "-blockingad",
		BOOL, (caddr_t) &blocking_ad_read_p }, 

	{ "CtlFileName", "Control file name", "-ctlfn",
		STRING, (caddr_t) &ctl_file_name }, 

	{ "CtlLineOffset", "Number of Lines to skip in ctl file", "-ctloffset",
		INT,	(caddr_t) &ctl_offset }, 

	{ "CtlCount", "Number of lines to process in ctl file", "-ctlcount",
		INT,	(caddr_t) &ctl_count }, 

	{ "CtlLineIncr", "Do every nth line in the ctl file", "-ctlincr",
		INT,	(caddr_t) &ctl_incr }, 

	{ "ComputeAllSenones", "Compute all senone scores every frame", "-compallsen",
		BOOL,  (caddr_t) &compute_all_senones }, 

	{ "TopSenonesFrames", "#frames top senones for predicting phones", "-topsenfrm",
		INT,  (caddr_t) &topsen_window }, 

	{ "TopSenonesThresh", "Top senones threshold for predicting phones", "-topsenthresh",
		INT,  (caddr_t) &topsen_thresh }, 

	{ "wsj1Sent", "Sent_Dir using wsj1 format", "-wsj1Sent",
		BOOL, (caddr_t) &wsj1Sent }, 

	{ "ReportAltPron", "Report actual pronunciation in match file", "-reportpron",
		BOOL, (caddr_t) &report_altpron }, 

	{ "MatchFileName", "Recognition output file name", "-matchfn",
		STRING, (caddr_t) &match_file_name }, 

	{ "MatchSegFileName", "Recognition output with segmentation", "-matchsegfn",
		STRING, (caddr_t) &matchseg_file_name }, 

	{ "PhoneConfidence", "Phone confidence", "-phoneconf",
		INT, (caddr_t) &phone_conf }, 

	{ "PhoneLat", "Phone lattice based on best senone scores", "-pscr2lat",
		BOOL, (caddr_t) &pscr2lat }, 

	{ "LogFileName", "Recognition ouput file name", "-logfn",
		STRING, (caddr_t) &logfn_arg }, 

	{ "CorrectFileName", "Reference ouput file name", "-correctfn",
		STRING, (caddr_t) &correct_file_name }, 

	{ "Utterance", "Utterance name", "-utt",
		STRING, (caddr_t) &utterance }, 

	{ "DataDirectory", "Data directory", "-datadir",
		STRING, (caddr_t) &data_directory }, 

	{ "DataDirectory", "Data directory", "-cepdir",
		STRING, (caddr_t) &cepdir }, 

	{ "DataDirectory", "Data directory", "-vqdir",
		STRING, (caddr_t) &data_directory }, 

	{ "SegDataDirectory", "Data directory", "-segdir",
		STRING, (caddr_t) &seg_data_directory }, 

	{ "SentDir", "Sentence directory", "-sentdir",
		STRING, (caddr_t) &sent_directory }, 

	{ "SentExt", "Sentence File Extension", "-sentext",
  	        STRING, (caddr_t) &sent_ext }, 

	{ "PhoneLabDir", "Phone Label Directory", "-phonelabdir",
  	        STRING, (caddr_t) &phonelabdirname }, 

	{ "PhoneLabExt", "Phone Label Extension (default lab)", "-phonelabext",
  	        STRING, (caddr_t) &phonelabextname }, 

	{ "PhoneLabType", "Phone Label Type (default xlabel)", "-phonelabtype",
  	        STRING, (caddr_t) &phonelabtype }, 

	{ "WordLabDir", "Word Label Directory", "-wordlabdir",
  	        STRING, (caddr_t) &wordlabdirname }, 

	{ "WordLabExt", "Word Label Extension (default wrd)", "-wordlabext",
  	        STRING, (caddr_t) &wordlabextname }, 

	{ "WordLabType", "Word Label Type (default xlabel)", "-wordlabtype",
  	        STRING, (caddr_t) &wordlabtype }, 
	
	{ "LMNamesDir", "Directory for LM-name file for each utt", "-lmnamedir",
		STRING, (caddr_t) &utt_lmname_dir }, 
	
	{ "LMNamesExt", "Filename extension for LM-name files", "-lmnameext",
		STRING, (caddr_t) &lmname_ext }, 
	
	{ "StartWordDir", "Startword directory", "-startworddir",
		STRING, (caddr_t) &startWord_directory }, 

	{ "StartWordExt", "StartWord File Extension", "-startwordext",
  	        STRING, (caddr_t) &startWord_ext }, 

	{ "NbestDir", "N-best Hypotheses Directory", "-nbestdir",
		STRING, (caddr_t) &nbest_dir }, 

	{ "NbestCount", "No. N-best Hypotheses", "-nbest",
		INT, (caddr_t) &nbest }, 

	{ "NbestExt", "N-best Hypothesis File Extension", "-nbestext",
  	        STRING, (caddr_t) &nbest_ext }, 

	{ "CepExt", "Cepstrum File Extension", "-cepext",
  	        STRING, (caddr_t) &cep_ext }, 

	{ "CCodeExt", "CCode File Extension", "-cext",
  	        STRING, (caddr_t) &cext }, 

	{ "DCodeExt", "DCode File Extension", "-dext",
  	        STRING, (caddr_t) &dext }, 

	{ "PCodeExt", "PCode File Extension", "-pext",
  	        STRING, (caddr_t) &pext }, 

	{ "XCodeExt", "XCode File Extension (4 codebook only)", "-xext",
	        STRING, (caddr_t) &xext }, 

	{ "BeamWidth", "Beam Width", "-beam",
		FLOAT, (caddr_t) &beam_width }, 

	{ "NewWordBeamWidth", "New Word Beam Width", "-nwbeam",
	        FLOAT, (caddr_t) &new_word_beam_width }, 

	{ "FwdFlatBeamWidth", "FwdFlat Beam Width", "-fwdflatbeam",
		FLOAT, (caddr_t) &fwdflat_beam_width }, 

	{ "FwdFlatNewWordBeamWidth", "FwdFlat New Word Beam Width", "-fwdflatnwbeam",
	        FLOAT, (caddr_t) &fwdflat_new_word_beam_width }, 

	{ "LastPhoneAloneBeamWidth", "Beam Width for Last Phones Only", "-lponlybw",
	        FLOAT, (caddr_t) &lastphone_alone_beam_width }, 

	{ "LastPhoneAloneBeamWidth", "Beam Width for Last Phones Only", "-lponlybeam",
	        FLOAT, (caddr_t) &lastphone_alone_beam_width }, 

	{ "NewPhoneBeamWidth", "New Phone Beam Width", "-npbeam",
	        FLOAT, (caddr_t) &new_phone_beam_width }, 

	{ "LastPhoneBeamWidth", "Last Phone Beam Width", "-lpbeam",
	        FLOAT, (caddr_t) &last_phone_beam_width }, 

	{ "PhoneInsertionPenalty", "Penalty for each phone used", "-phnpen",
		FLOAT, (caddr_t) &phone_insertion_penalty }, 

	{ "InsertionPenalty", "Penalty for word transitions", "-inspen",
		FLOAT, (caddr_t) &insertion_penalty }, 

	{ "NewWordPenalty", "Penalty for new word transitions", "-nwpen",
		FLOAT, (caddr_t) &nw_pen }, 

	{ "SilenceWordPenalty", "Penalty for silence word transitions", "-silpen",
		FLOAT, (caddr_t) &silence_word_penalty }, 

	{ "FillerWordPenalty", "Penalty for filler word transitions", "-fillpen",
		FLOAT, (caddr_t) &filler_word_penalty }, 

	{ "LanguageWeight", "Weighting on Language Probabilities", "-langwt",
		FLOAT, (caddr_t) &fwdtree_lw }, 

	{ "RescoreLanguageWeight", "LM prob weight for rescoring pass", "-rescorelw",
		FLOAT, (caddr_t) &bestpath_lw }, 

	{ "FwdFlatLanguageWeight", "FwdFlat Weighting on Language Probabilities", "-fwdflatlw",
		FLOAT, (caddr_t) &fwdflat_lw }, 

	{ "FwdTree", "Fwd tree search (1st pass)", "-fwdtree",
		BOOL, (caddr_t) &fwdtree_flag }, 

	{ "FwdFlat", "Flat fwd search over fwdtree lattice", "-fwdflat",
		BOOL, (caddr_t) &fwdflat_flag }, 

	{ "ForwardOnly", "Run only the forward pass", "-forwardonly",
		BOOL, (caddr_t) &forward_only }, 

	{ "Bestpath", "Shortest path search over lattice", "-bestpath",
		BOOL, (caddr_t) &bestpath_flag }, 
	
	{ "TrigramInFwdPass", "Use trigram (if available) in forward pass", "-fwd3g",
		BOOL, (caddr_t) &use_3g_in_fwd_pass }, 

	{ "CodeBookDirectory", "Code book directory", "-cbdir",
	    	STRING, (caddr_t) &cbdir }, 

	{ "CCodeBookFileName", "CCode Book File Name", "-ccbfn",
		STRING, (caddr_t) &ccbfn }, 

	{ "DCodeBookFileName", "DCode Book File Name", "-dcbfn",
	       	STRING, (caddr_t) &dcbfn }, 

	{ "PCodeBookFileName", "PCode Book File Name", "-pcbfn",
		STRING, (caddr_t) &pcbfn }, 

	{ "XCodeBookFileName", "XCode Book File Name", "-xcbfn",
		STRING, (caddr_t) &xcbfn }, 

	{ "Use20msDiffPow", "Use 20 ms diff power instead of c0", "-use20msdp",
		BOOL, (caddr_t) &use20msDiffPow }, 

	{ "CepFloor", "Floor of Cepstrum Variance", "-cepfloor",
		FLOAT, (caddr_t) &Cep_Floor }, 

	{ "DCepFloor", "Floor of Delta Cepstrum Variance", "-dcepfloor",
		FLOAT, (caddr_t) &Dcep_Floor }, 

	{ "XCepFloor", "Floor of XCepstrum Variance", "-xcepfloor",
		FLOAT, (caddr_t) &Pow_Floor }, 

	{ "TopNCodeWords", "Number of code words to use", "-top",
		INT, (caddr_t) &scVqTopN }, 

	{ "SkipAltFrames", "Skip alternate frames in exiting phones", "-skipalt",
		INT, (caddr_t) &skip_alt_frm }, 

	{ "WriteScoreInMatchFile", "write score in the match file", "-matchscore",
		BOOL, (caddr_t) &writeScoreInMatchFile }, 

	{ "LatticeSizes", "BP and FP Tables Sizes", "-latsize",
		INT, (caddr_t) &lattice_size }, 

	{ "LMCacheNumLines", "No. lines in LM cache", "-lmcachelines",
		INT, (caddr_t) &lm_cache_lines }, 
#ifdef USE_ILM
	{ "ILMUGCacheWeight", "Weight(%) for ILM UG cache prob", "-ilmugwt",
		INT, (caddr_t) &ilm_ugcache_wt }, 

	{ "ILMBGCacheWeight", "Weight(%) for ILM BG cache prob", "-ilmbgwt",
		INT, (caddr_t) &ilm_bgcache_wt }, 
#endif
	{ "DumpLattice", "Dump Lattice", "-dumplatdir",
		STRING, (caddr_t) &dumplat_dir }, 

	{ "SamplingRate", "Sampling rate", "-samp",
		INT, (caddr_t) &sampling_rate }, 

	{ "UseADCInput", "Use raw ADC input", "-adcin",
		BOOL, (caddr_t) &adc_input }, 

	{ "ADCFileExt", "ADC file extension", "-adcext",
		STRING, (caddr_t) &adc_ext }, 

	{ "ADCByteOrder", "ADC file byte order (0:BIG/1:LITTLE)", "-adcendian",
		INT, (caddr_t) &adc_endian }, 

	{ "ADCHdrSize", "ADC file header size", "-adchdr",
		INT, (caddr_t) &adc_hdr }, 

	{ "RawLogDir", "Log directory for raw output files)", "-rawlogdir",
		STRING, (caddr_t) &rawlogdir }, 

	{ "MFCLogDir", "Log directory for MFC output files)", "-mfclogdir",
		STRING, (caddr_t) &mfclogdir }, 

	{ "TimeAlignCtlFile", "Time align control file", "-tactlfn",
		STRING, (caddr_t) &time_align_ctl_file_name }, 

	{ "TimeAlignWord", "Time Align Phone", "-taword",
		BOOL, (caddr_t) &time_align_word }, 

	{ "TimeAlignPhone", "Time Align Phone", "-taphone",
		BOOL, (caddr_t) &time_align_phone }, 

	{ "TimeAlignState", "Time Align State", "-tastate",
		BOOL, (caddr_t) &time_align_state }, 

	{ "SegFileExt", "Seg file extension", "-segext",
		STRING, (caddr_t) &seg_file_ext }, 

	{ "ScoreFileExt", "Seg file extension", "-scoreext",
		STRING, (caddr_t) &score_file_ext }, 

	{ "OutSentFile", "output sentence file name", "-osentfn",
		STRING, (caddr_t) &out_sent_filename }, 

	{ "PrintBackTrace", "Print Back Trace", "-backtrace",
		BOOL, (caddr_t) &print_back_trace }, 

	{ "CDCNinitFile", "CDCN Initialization File", "-cdcn",
		STRING, (caddr_t) &cdcn_file }, 

	{ "VerbosityLevel", "Verbosity Level", "-verbose",
	        INT, (caddr_t) &verbosity_level },

	{ 0,0,0,NOTYPE,0 }
};

search_hyp_t *run_sc_utterance (char *mfcfile, int32 sf, int32 ef, char *idspec);


static int32 nextarg(char *line, int32 *start, int32 *len, int32 *next)
{
    int32 i, lineLen;
    
    lineLen = strlen(line);

    /* Find first non-space character */
    for (i = 0; isspace(line[i]) && i < lineLen; i++);

    if (i == lineLen)
	return 1;

    if (line[i] == '"') {
	/* NOTE: No escape characters mechanism within quoted string */
	i++;
	for (*start = i; line[i] != '"' && i < lineLen; i++);
	if (line[i] != '"')
	    return 1;
	*len = i - *start;
	*next = i+1;
	return 0;
    } else {
	for (*start = i; !isspace(line[i]) && i < lineLen; i++);
	*len = i - *start;
	*next = i;
	return 0;
    }
}


/*
 * Read arguments from file and append to command line arguments.
 * NOTE: Spaces inside arguments, and quote marks around arguments not handled correctly.
 */
static int32 argfile_read (const int32 argc, char ***argv, const char *argfile)
{
    FILE *fp;
    int32 i, narg, len;
    char argstr[1024];
    char argline[4096];
    char **newargv;
    char *lp;
    int32 start, next;
    
    if ((fp = fopen(argfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", argfile);
    
    /* Count arguments */
    narg = 0;
    while (fgets (argline, sizeof(argline), fp) != NULL) {
	if (argline[0] == '#')
	    continue;

	lp = argline;

	while (! nextarg (lp, &start, &len, &next)) {
	    lp = lp + next;
	    narg++;
	}
    }
    rewind (fp);
    
    /* Allocate space for arguments */
    narg += argc;
    newargv = (char **) malloc (narg * sizeof(char *));
    if (newargv == NULL)
	E_FATAL("malloc failed\n");

    /* Read arguments */
    newargv[0] = (*argv)[0];
    i = 1;
    while (fgets (argline, sizeof(argline), fp) != NULL) {
	if (argline[0] == '#')
	    continue;

	lp = argline;
	while (! nextarg (lp, &start, &len, &next)) {
	    assert(i < narg);
	    strncpy(argstr, lp + start, len);
	    argstr[len] = '\0';
	    lp = lp + next;
	    newargv[i] = salloc(argstr);
	    
	    i++;
	}
    }
    fclose (fp);
    assert (i == narg-argc+1);
    
    /* Copy initial argument list at the end */
    for (i = 1, narg -= (argc-1); i < argc; i++, narg++)
	newargv[narg] = (*argv)[i];
    
    *argv = newargv;
    return (narg);
}


/* Set SCVQ parameters mean normalization, AGC, and silence compression */
static void init_norm_agc_cmp ( void )
{
    scvq_agc_t agc;
    scvq_norm_t norm;
    scvq_compress_t cmp;
    
    agc = AGC_NONE;
    if (agcNoise) agc = AGC_NOISE;
    else if (agcMax) agc = AGC_MAX;
    else if (agcEMax) agc = AGC_EMAX;
    if ((! ctl_file_name) && live && (agc != AGC_NONE) && (agc != AGC_EMAX)) {
	agc = AGC_EMAX;
	E_INFO("%s(%d): Live mode; AGC set to AGC_EMAX\n", __FILE__, __LINE__);
    }
    
    norm = NORM_NONE;
    if (normalizeMean)
	norm = normalizeMeanPrior ? NORM_PRIOR : NORM_UTT;
    if ((! ctl_file_name) && live && (norm == NORM_UTT)) {
	norm = NORM_PRIOR;
	E_INFO("%s(%d): Live mode; MeanNorm set to NORM_PRIOR\n", __FILE__, __LINE__);
    }
    
    cmp = COMPRESS_NONE;
    if (compress)
	cmp = compress_prior ? COMPRESS_PRIOR : COMPRESS_UTT;
    if ((! ctl_file_name) && live && (cmp == COMPRESS_UTT)) {
	cmp = COMPRESS_PRIOR;
	E_INFO("%s(%d): Live mode; Silence compression set to COMPRESS_PRIOR\n",
		__FILE__, __LINE__);
    }
    
    uttproc_set_cmn (norm);
    uttproc_set_agc (agc);
    uttproc_set_silcmp (cmp);
}


static FILE *uttfp = NULL;
static float *coeff;
static int32 ncoeff;
static int32 ncoeff_read;

#define SWAP_UINT16(x)	x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )

/*
 * Code for reading utterance data (A/D data) from a file, as per interface in ad.h.
 * This function is passed to uttproc for batch-mode processing of raw A/D data from
 * files.
 */
int32 adc_file_read (int16 *buf, int32 max)
{
    int32 i, n;
    
    if (uttfp == NULL)
	return -1;

    if ((n = fread (buf, sizeof(int16), max, uttfp)) <= 0)
	return -1;

    /* Byte swap if necessary */
#if (__BIG_ENDIAN__)
    if (adc_endian == 1) {	/* Little endian adc file */
	for (i = 0; i < n; i++)
	    SWAP_UINT16(buf[i]);
    }
#else
    if (adc_endian == 0) {	/* Big endian adc file */
	for (i = 0; i < n; i++)
	    SWAP_UINT16(buf[i]);
    }
#endif

    return n;
}


/*
 * Code for reading passing cep data from a buffer (previously filled from a cep file)
 * to the decoder in batch mode.
 * This function is passed to uttproc for batch-mode processing of cep data from files.
 */
static int32 cep_buf_read (float *cepbuf)
{
    if (ncoeff_read >= ncoeff)
	return -1;

    memcpy (cepbuf, coeff+ncoeff_read, CEP_VECLEN*sizeof(float));
    ncoeff_read += CEP_VECLEN;

    return 1;
}


static int32 final_argc;
static char **final_argv;
static FILE *logfp = NULL;
static char logfile[4096];	/* Hack!! Hardwired constant 4096 */


static void log_arglist (FILE *fp, int32 argc, char *argv[])
{
    int32 i;

    /* Log the arguments */
    for (i = 0; i < argc; i++) {
	if (argv[i][0] == '-')
	    fprintf (fp, "\\\n ");
	fprintf (fp, "%s ", argv[i]);
    }
    fprintf (fp, "\n\n");
    fflush (fp);
}


/* Should be in uttproc.c, but ... */
int32 uttproc_set_logfile (char const *file)
{
    FILE *fp;
    
    E_INFO("uttproc_set_logfile(%s)\n", file);
    
    if ((fp = fopen(file, "w")) == NULL) {
	E_ERROR ("fopen(%s,w) failed\n", file);
	return -1;
    } else {
	if (logfp)
	    fclose (logfp);

	logfp = fp;
	*stdout = *logfp;
	*stderr = *logfp;
	
	E_INFO("Previous logfile: '%s'\n", logfile);
	strcpy (logfile, file);
	
	log_arglist (logfp, final_argc, final_argv);
    }

    return 0;
}

int
fbs_init (int32 argc, char **argv)
{
    unlimit ();		/* Remove memory size limits */

    /* Parse command line arguments */
    pconf (argc, argv, param, 0, 0, 0);
    if (arg_file) {
	/* Read arguments from argfile */
	argc = argfile_read (argc, &argv, arg_file);
	pconf (argc, argv, param, 0, 0, 0);
    }
    final_argc = argc;
    final_argv = argv;
    
    /* Open logfile if specified; else log to stdout/stderr */
    logfile[0] = '\0';
    if (logfn_arg) {
	if ((logfp = fopen(logfn_arg, "w")) == NULL) {
	    E_ERROR ("fopen(%s,w) failed\n", logfn_arg);
	} else {
	    strcpy (logfile, logfn_arg);
	    *stdout = *logfp;
	    *stderr = *logfp;
	}
    }

    if (verbosity_level >= 2)
	log_arglist (stdout, argc, argv);
    
#ifndef WIN32
    if (verbosity_level >= 2) {
	system ("hostname");
	system ("date");
	printf ("\n\n");
    }
#endif

    E_INFO("libfbs/main COMPILED ON: %s, AT: %s\n\n", __DATE__, __TIME__);
    
#if defined(PROF_TIME)
    if (timer_open() < 0) {
	E_FATAL(stderr, "Could not open timer module\n");
    }
#endif
#if defined(PROF_MEM)
    _mtr_init();	/*  Turn on space metering a la Ravi */
#endif

    /* Compatibility with old forwardonly flag */
    if (forward_only)
	bestpath_flag = FALSE;
    if ((! fwdtree_flag) && (! fwdflat_flag))
	QUIT((stderr, "%s(%d): At least one of -fwdtree and -fwdflat flags must be TRUE\n",
	      __FILE__, __LINE__));
    
    /* Load the KB */
    kb (argc, argv, insertion_penalty, fwdtree_lw, phone_insertion_penalty);

    exts[0] = cext;
    exts[1] = dext;
    exts[2] = pext;
    exts[3] = xext;

    /*
     * Read the code books. These probably should be put into the KB
     * at a later date.
     */
    if ((ccbfn == NULL) || (dcbfn == NULL) || (pcbfn == NULL) || (xcbfn == NULL))
	QUIT ((stderr, "%s(%d): One or more codebooks not specified\n",
	       __FILE__, __LINE__));

    /* initialize semi-continuous acoustic and model scoring subsystem */
    SCVQInit(scVqTopN, kb_get_total_dists(), 1,
	     (double) Cep_Floor, use20msDiffPow);
    
    SCVQSetdcep80msWeight (dcep80msWeight);
    if (agcNoise || agcMax) {
	agc_set_threshold (agcThresh);
#if 0
	SCVQAgcSet(AGC_NONE);
#endif
    }
    else {
	if (agcBeta) {
#if 0
	    SCVQAgcSet(AGC_BETA);
	    SCVQAgcInit(TRUE, 25);
#else
	    QUIT((stdout, "%s(%d): agc beta not supported\n", __FILE__, __LINE__));
#endif
	} else {
#if 0
	    if (agcEMax) 
		SCVQAgcSet(AGC_EMAX);
	    else
		SCVQAgcSet(AGC_NONE);
#endif
	}
    }
    
    {
	char mpath[MAXPATHLEN+1], vpath[MAXPATHLEN+1];
	
	sprintf(mpath, "%s/%s.vec", cbdir, ccbfn);
	sprintf(vpath, "%s/%s.var", cbdir, ccbfn);
	if (SCVQInitFeat(CEP_FEAT, mpath, vpath, kb_get_codebook_0_dist()) < 0)
	    E_FATAL("SCVQInitFeat(%s,%s) failed\n", mpath, vpath);
	
	sprintf(mpath, "%s/%s.vec", cbdir, dcbfn);
	sprintf(vpath, "%s/%s.var", cbdir, dcbfn);
	if (SCVQInitFeat(DCEP_FEAT, mpath, vpath, kb_get_codebook_1_dist()) < 0)
	    E_FATAL("SCVQInitFeat(%s,%s) failed\n", mpath, vpath);
	
	sprintf(mpath, "%s/%s.vec", cbdir, pcbfn);
	sprintf(vpath, "%s/%s.var", cbdir, pcbfn);
	if (SCVQInitFeat(POW_FEAT, mpath, vpath, kb_get_codebook_2_dist()) < 0)
	    E_FATAL("SCVQInitFeat(%s,%s) failed\n", mpath, vpath);
	
	sprintf(mpath, "%s/%s.vec", cbdir, xcbfn);
	sprintf(vpath, "%s/%s.var", cbdir, xcbfn);
	if (SCVQInitFeat(DDCEP_FEAT, mpath, vpath, kb_get_codebook_3_dist()) < 0)
	    E_FATAL("SCVQInitFeat(%s,%s) failed\n", mpath, vpath);
    }
    
    search_initialize ();
    
    search_set_beam_width (beam_width);
    search_set_new_word_beam_width (new_word_beam_width);
    search_set_new_phone_beam_width (new_phone_beam_width);
    search_set_last_phone_beam_width (last_phone_beam_width);
    search_set_lastphone_alone_beam_width (lastphone_alone_beam_width);
    search_set_silence_word_penalty (silence_word_penalty, phone_insertion_penalty);
    search_set_filler_word_penalty (filler_word_penalty, phone_insertion_penalty);
    search_set_newword_penalty (nw_pen);
    search_set_lw (fwdtree_lw, fwdflat_lw, bestpath_lw);
    search_set_ip (insertion_penalty);
    search_set_skip_alt_frm (skip_alt_frm);
    search_set_fwdflat_bw (fwdflat_beam_width, fwdflat_new_word_beam_width);
    
    searchSetScVqTopN (scVqTopN);

#if 0
    SCVQSetSilCompression (compress);
#endif

#if defined(PROF_TIME)
    timer_dump ("Initialization");
#endif

    /* Initialize dynamic data structures needed for utterance processing */
    uttproc_init ();
    
    if (rawlogdir)
	uttproc_set_rawlogdir (rawlogdir);
    if (mfclogdir)
	uttproc_set_mfclogdir (mfclogdir);
    
    /* Initialize cepstral mean normalization, AGC, and silence compression options */
    init_norm_agc_cmp ();

    /* If multiple LMs present, choose the unnamed one by default */
    if (get_n_lm() == 1) {
	if (uttproc_set_lm (get_current_lmname()) < 0)
	    E_FATAL ("SetLM() failed\n");
    } else {
	if (uttproc_set_lm ("") < 0)
	    E_WARN ("SetLM(\"\") failed; application must set one before recognition\n");
    }
    
    /* Set the current start word to <s> (if it exists) */
    if (kb_get_word_id ("<s>") >= 0)
	uttproc_set_startword ("<s>");
    
    if (allphone_mode)
	allphone_init (beam_width, new_word_beam_width, phone_insertion_penalty);
    
    E_INFO("libfbs/main COMPILED ON: %s, AT: %s\n\n", __DATE__, __TIME__);
    
    /*
     * Initialization complete; If there was a control file run batch
     */
    
    if (ctl_file_name) {
	if (!time_align_ctl_file_name)
	    run_ctl_file (ctl_file_name);
	else
	    run_time_align_ctl_file (ctl_file_name, time_align_ctl_file_name,
				     out_sent_filename);
	
	uttproc_end ();
    
#if defined(PROF_TIME)
	timer_close();
#endif
#if defined(PROF_MEM)
	_mtr_dump();
#endif
	
	exit(0);
    }
    
    return 0;
}

int32
fbs_end (void)
{
    uttproc_end ();
    return 0;
}


/*
 * Too lazy to put this into uttproc.c.
 */
int32 uttproc_parse_ctlfile_entry (char *line,
				   char *filename, int32 *sf, int32 *ef, char *idspec)
{
    int32 k;
    
    *sf = *ef = -1;	/* Default; process entire file */

    if ((k = sscanf (line, "%s %d %d %s", filename, sf, ef, idspec)) <= 0)
	return -1;

    if (k == 1)
	strcpy (idspec, filename);
    else {
	if ((k == 2) || (*sf < 0) || (*ef <= *sf)) {
	    E_ERROR("Bad ctlfile entry: %s\n", line);
	    return -1;
	}
	if (k == 3)
	    sprintf (idspec, "%s_%d_%d", filename, *sf, *ef);
    }
    
    return 0;
}

void
run_ctl_file (char const *ctl_file_name)
/*-------------------------------------------------------------------------*
 * Sequence through a control file containing a list of utterance
 * NB. This is a one shot routine.
 */
{
    FILE *ctl_fs;
    char line[4096], mfcfile[4096], idspec[4096];
    int32 line_no = 0;
    int32 sf, ef;

    if (strcmp (ctl_file_name, "-") != 0)
	ctl_fs = CM_fopen (ctl_file_name, "r");
    else
	ctl_fs = stdin;
    
    for (;;) {
	if (ctl_fs == stdin)
	    E_INFO ("\nFile(no ext): ");
	if (fgets (line, sizeof(line), ctl_fs) == NULL)
	    break;
	
	if (uttproc_parse_ctlfile_entry (line, mfcfile, &sf, &ef, idspec) < 0)
	    continue;
	
	if (strcmp (mfcfile, "--END-OF-DOCUMENT--") == 0) {
	    search_finish_document ();
	    continue;
	}
	if ((ctl_offset-- > 0) || (ctl_count <= 0) || ((line_no++ % ctl_incr) != 0))
	    continue;

#if 0
	/* This stuff no longer works -- rkm@cs.cmu.edu (02/03/1999) */
	/*
	 * Try to read the reference sentence (all this breaks with the new ctl format)
	 */
   	{
  	    FILE *fp;
            char file_name[1024];
	    char *ptr = strrchr (Utt, '/');

	    if (ptr == 0)
		ptr = Utt; /* if (wsj1Sent) trouble */
	    else {
		if (wsj1Sent) {
  	  	  do {ptr--;} while (ptr >= Utt && *ptr != '/');
		}
	        ptr++;
	    }
	    
            sprintf (file_name, "%s/%s.%s", sent_directory, ptr, sent_ext);
	    fp = fopen (file_name, "r");
	    if (fp != NULL) {
	        fgets (ref_sentence, sizeof(ref_sentence), fp);
		if (ref_sentence[strlen(ref_sentence) - 1] == '\n')
	            ref_sentence[strlen(ref_sentence) - 1] = '\0';
				/* Kill '\n' */
	        fclose (fp);
	    }
	    else {
	        ref_sentence[0] = '\0';
	    }
	}
#endif

	/*
	 * Set force_str
	 */    
        if (forceRec) {
	    force_str = ref_sentence;
        }

	E_INFO ("\nUtterance: %s\n", idspec);

	if (! allphone_mode)
	    run_sc_utterance (mfcfile, sf, ef, idspec);
	else
	    uttproc_allphone_file (mfcfile);

#if 0
	/* This stuff no longer works -- rkm@cs.cmu.edu (02/03/1999) */
	/*
	 * Write the correct file if one was specified.
	 */
	if (correct_file_name) {
	    FILE *fp;
	    char *ptr = strrchr (Utt, '/');

	    if (ptr == 0) ptr = Utt; 
	    else ptr++;
	    
	    fp = CM_fopen (correct_file_name, "a");
	    fprintf (fp, "%s (%s)\n", ref_sentence, ptr);
	    fclose (fp);
	}
#endif

	ctl_count--;

#if (defined(PROF_TIME) && 0)
	/* This stuff no longer works -- rkm@cs.cmu.edu (02/03/1999) */
	timer_dump(Utt);
        fflush (stdout);
#endif
    }
    
    if (ctl_fs != stdin)
	fclose (ctl_fs);
}

int32 uttfile_open (char const *utt)
{
    char inputfile[MAXPATHLEN];
    int32 n, l;
    char const *file_ext;

    /* Figure out file extension to be added, if any */
    file_ext = adc_input ? adc_ext : cep_ext;
    n = strlen(file_ext);
    l = strlen(utt);
    if ((l > n+1) && (utt[l-n-1] == '.') && (strcmp (utt+l-n, file_ext) == 0))
	file_ext = "";	/* Extension already exists */
    
    /* Build input filename */
#ifdef WIN32
    if (data_directory && (utt[0] != '/') && (utt[0] != '\\') &&
	((utt[0] != '.') || ((utt[1] != '/') && (utt[1] != '\\'))))
	sprintf (inputfile, "%s/%s.%s", data_directory, utt, file_ext);
    else
	sprintf (inputfile, "%s.%s", utt, file_ext);
#else
    if (data_directory && (utt[0] != '/') && ((utt[0] != '.') || (utt[1] != '/')))
	sprintf (inputfile, "%s/%s.%s", data_directory, utt, file_ext);
    else
	sprintf (inputfile, "%s.%s", utt, file_ext);
#endif

    if (adc_input) {
	if ((uttfp = fopen (inputfile, "rb")) == NULL) {
	    E_FATAL ("%s(%d): fopen(%s,rb) failed\n", __FILE__, __LINE__, inputfile);
	}
	if (adc_hdr > 0) {
	    if (fseek (uttfp, adc_hdr, SEEK_SET) < 0) {
		E_ERROR("fseek(%s,%d) failed\n", inputfile, adc_hdr);
		return -1;
	    }
	}
#if (__BIG_ENDIAN__)
	if (adc_endian == 1)	/* Little endian adc file */
	    E_INFO("Byte-reversing %s\n", inputfile);
#else
	if (adc_endian == 0)	/* Big endian adc file */
	    E_INFO("Byte-reversing %s\n", inputfile);
#endif
    } else {
	if (cep_read_bin (&coeff, &ncoeff, inputfile) != 0) {
	    E_ERROR ("%s(%d): **ERROR** Read(%s) failed\n", __FILE__, __LINE__, inputfile);
	    ncoeff = 0;
	    return -1;
	}
	ncoeff /= sizeof(float);
	ncoeff_read = 0;
    }
    
    return (0);
}


void uttfile_close ( void )
{
    if (adc_input) {
	if (uttfp)
	    fclose (uttfp);
	uttfp = NULL;
    } else
	free (coeff);
}


/* Return #frames converted to feature vectors; -1 if error */
int32 utt_file2feat (char *utt, int32 nosearch)
{
    static int16 *adbuf = NULL;
    static float *mfcbuf = NULL;
    int32 k;
    
    if (uttfile_open (utt) < 0)
	return -1;
    
    if (uttproc_nosearch (nosearch) < 0)
	return -1;
    
    if (uttproc_begin_utt (utt_name) < 0)
	return -1;
    
    if (adc_input) {
	if (! adbuf)
	    adbuf = (int16 *) CM_calloc (4096, sizeof(int16));
	
	while ((k = adc_file_read (adbuf, 4096)) >= 0)
	    if (uttproc_rawdata (adbuf, k, 1) < 0)
		return -1;
    } else {
	if (! mfcbuf)
	    mfcbuf = (float *) CM_calloc (CEP_VECLEN, sizeof(float));

	while (cep_buf_read (mfcbuf) >= 0)
	    if (uttproc_cepdata (&mfcbuf, 1, 1) < 0)
		return -1;
    }

    if (uttproc_end_utt () < 0)
	return -1;
    
    uttfile_close ();
    
    return (uttproc_get_featbuf (&cep, &dcep, &dcep_80ms, &pcep, &ddcep));
}


char *build_uttid (char *utt)
{
    char *utt_id;
    
    /* Find uttid */
#ifdef WIN32
    {
      int32 i;
      for (i = strlen(utt)-1; (i >= 0) && (utt[i] != '\\') && (utt[i] != '/'); --i);
      utt_id = utt+i;
    }
#else
    utt_id = strrchr (utt, '/');
#endif
    if (utt_id)
    	utt_id++;
    else
    	utt_id = utt;
    strcpy (utt_name, utt_id);

    return utt_name;
}


void
run_time_align_ctl_file (char const *utt_ctl_file_name,
			 char const *pe_ctl_file_name,
			 char const *out_sent_file_name)
/*-------------------------------------------------------------------------*
 * Sequence through a control file containing a list of utterance
 * NB. This is a one shot routine.
 */
{
    FILE *utt_ctl_fs;
    FILE *pe_ctl_fs;
    FILE *out_sent_fs;
    char Utt[1024];
    char time_align_spec[1024];
    int32 line_no = 0;
    char left_word[256];
    char right_word[256];
    char pe_words[1024];
    int32 begin_frame;
    int32 end_frame;
    int32 n_featfr;
    int32 align_all = 0;
    
    time_align_init();
    beam_width = 1e-9;
    time_align_set_beam_width(beam_width);
    E_INFO ("%s(%d): ****** USING WIDE BEAM ****** (1e-9)\n", __FILE__, __LINE__);

    utt_ctl_fs = CM_fopen (utt_ctl_file_name, "r");
    pe_ctl_fs = CM_fopen (pe_ctl_file_name, "r");

    if (out_sent_file_name) {
	out_sent_fs = CM_fopen (out_sent_file_name, "w");
    }
    else
	out_sent_fs = NULL;

    while (fscanf (utt_ctl_fs, "%s\n", Utt) != EOF) {
	fgets(time_align_spec, 1023, pe_ctl_fs);

	if (ctl_offset) {
	    ctl_offset--;
	    continue;
	}
	if (ctl_count == 0)
	    continue;
 	if ((line_no++ % ctl_incr) != 0)
	    continue;

	if  (!strncmp(time_align_spec, "*align_all*", strlen("*align_all*"))) {
	    E_INFO("%s(%d): Aligning whole utterances\n", __FILE__, __LINE__);
	    align_all = 1;
	    fgets(time_align_spec, 1023, pe_ctl_fs);
	}

	if (align_all) {
	    strcpy(left_word, "<s>");
	    strcpy(right_word,"</s>");
	    begin_frame = end_frame = NO_FRAME;
	    time_align_spec[strlen(time_align_spec)-1] = '\0';
	    strcpy(pe_words, time_align_spec);

	    E_INFO ("%s(%d): Utt %s\n", __FILE__, __LINE__, Utt);
	    fflush (stdout);
	}
	else {
	    sscanf(time_align_spec,
		   "%s %d %d %s %[^\n]",
		   left_word, &begin_frame, &end_frame, right_word, pe_words);
	    E_INFO ("\nDoing  '%s %d) %s (%d %s' in utterance %s\n",
		    left_word, begin_frame,
		    pe_words,
		    end_frame, right_word, Utt);
	}

	build_uttid (Utt);
	if ((n_featfr = utt_file2feat (Utt, 1)) < 0)
	    E_ERROR("Failed to load %s\n", Utt);
	else {
	    time_align_utterance (Utt,
				  out_sent_fs,
				  left_word, begin_frame,
				  pe_words,
				  end_frame, right_word);
	}

	--ctl_count;
    }

    fclose (utt_ctl_fs);
    fclose (pe_ctl_fs);
}


/* Macro to byteswap an int32 variable.  x = ptr to variable */
#define SWAP_INT32(x)   *(x) = ((0x000000ff & (*(x))>>24) | \
                                (0x0000ff00 & (*(x))>>8) | \
                                (0x00ff0000 & (*(x))<<8) | \
                                (0xff000000 & (*(x))<<24))


/*
 * Read specified segment [sf..ef] of Sphinx-II format mfc file and write to
 * specified output file.
 */
void s2mfc_read (char *file, int32 sf, int32 ef, char *outfile)
{
    FILE *fp, *outfp;
    int32 n_float32;
    struct stat statbuf;
    int32 i, n, byterev, cepsize;
    float tmpbuf[CEP_SIZE];
    
    E_INFO("Extracting frames %d..%d from %s to %s\n", sf, ef, file, outfile);
    
    cepsize = CEP_SIZE;
    
    /* Find filesize */
    if (stat (file, &statbuf) != 0)
	E_FATAL("stat(%s) failed\n", file);
    
    fp = CM_fopen(file, "rb");
    outfp = CM_fopen (outfile, "wb");
    
    /* Read #floats in header */
    if (fread (&n_float32, sizeof(int32), 1, fp) != 1)
	E_FATAL("fread(%s) failed\n", file);
    
    /* Check if n_float32 matches file size */
    byterev = FALSE;
    if ((n_float32*sizeof(float) + 4) != statbuf.st_size) {
	n = n_float32;
	SWAP_INT32(&n);

	if ((n*sizeof(float) + 4) != statbuf.st_size) {
	    E_FATAL("Header size field: %d(%08x); filesize: %d(%08x)\n",
		    n_float32, n_float32, statbuf.st_size, statbuf.st_size);
	}
	
	n_float32 = n;
	byterev = TRUE;
    }
    if (n_float32 <= 0)
	E_FATAL("Header size field: %d\n",  n_float32);
    
    /* n = #frames of input */
    n = n_float32/cepsize;
    if (n * cepsize != n_float32)
	E_FATAL("Header size field: %d; not multiple of %d\n", n_float32, cepsize);
    
    if (sf > 0)
	fseek (fp, sf*cepsize*sizeof(float), SEEK_CUR);
    
    /* Read mfc data and write to outfile */
    fwrite (&i, sizeof(int32), 1, outfp);
    for (i = sf; i <= ef; i++) {
	if (fread (tmpbuf, sizeof(float), cepsize, fp) != cepsize)
	    E_FATAL("fread(%s) failed\n", file);
	if (fwrite (tmpbuf, sizeof(float), cepsize, outfp) != cepsize)
	    E_FATAL("fwrite(%s) failed\n", outfile);
    }
    fclose (fp);
    
    fflush (outfp);
    fseek (outfp, 0, SEEK_SET);
    i = (ef - sf + 1) * cepsize;
    if (byterev)
	SWAP_INT32(&i);
    fwrite (&i, sizeof(int32), 1, outfp);
    fclose (outfp);
}


static int32 mfcseg_extract (char *mfcfile, int32 sf, int32 ef, char *utt)
{
    char inputfile[1024], outputfile[1024];
    
    assert (! adc_input);
    
    if (cepdir && (mfcfile[0] != '/') && ((mfcfile[0] != '.') || (mfcfile[1] != '/')))
	sprintf (inputfile, "%s/%s.%s", cepdir, mfcfile, cep_ext);
    else
	sprintf (inputfile, "%s.%s", mfcfile, cep_ext);
    
    sprintf (outputfile, "%s.%s", utt, cep_ext);
    
    s2mfc_read (inputfile, sf, ef, outputfile);

    return 0;
}


/*
 * Decode utterance.
 */
search_hyp_t *run_sc_utterance (char *mfcfile, int32 sf, int32 ef, char *idspec)
{
    char startword_filename[1000];
    FILE *sw_fp;
    int32 frmcount, ret;
    char *finalhyp;
    char utt[1024];
    search_hyp_t *hypseg;

    strcpy (utt, idspec);
    build_uttid (utt);
    
    if (nbest > 0)
	bestpath_flag = 1;	/* Force creation of DAG */
    
    /* Select the LM for utt */
    if (get_n_lm() > 1) {
	FILE *lmname_fp;
	char utt_lmname_file[1000], lmname[1000];
	
	sprintf (utt_lmname_file, "%s/%s.%s", utt_lmname_dir, utt_name, lmname_ext);
	E_INFO ("%s(%d): Looking for LM-name file %s\n",
		__FILE__, __LINE__, utt_lmname_file);
	if ((lmname_fp = fopen (utt_lmname_file, "r")) != NULL) {
	    /* File containing LM name for this utt exists */
	    if (fscanf (lmname_fp, "%s", lmname) != 1)
		QUIT((stdout, "%s(%d): Cannot read lmname from file %s\n", __FILE__, __LINE__, utt_lmname_file));
	    fclose (lmname_fp);
	} else {
	    /* No LM name specified for this utt; use default (with no name) */
	    E_INFO ("%s(%d): File %s not found, using default LM\n",
		    __FILE__, __LINE__, utt_lmname_file);
	    lmname[0] = '\0';
	}
	
	uttproc_set_lm (lmname);
    }

    /* Select startword for utt (LISTEN project) */
#ifdef WIN32
    if (startWord_directory && (utt[0] != '\\') && (utt[0] != '/'))
	sprintf (startword_filename, "%s/%s.%s",
		 startWord_directory, utt, startWord_ext);
    else
	sprintf (startword_filename, "%s.%s", utt, startWord_ext);
#else
    if (startWord_directory && (utt[0] != '/'))
	sprintf (startword_filename, "%s/%s.%s",
		 startWord_directory, utt, startWord_ext);
    else
	sprintf (startword_filename, "%s.%s", utt, startWord_ext);
#endif
    if ((sw_fp = fopen(startword_filename, "r")) != NULL) {
	fscanf(sw_fp, "%s", startWord);
	fclose(sw_fp);
	E_INFO("startWord: %s\n", startWord);
    } else startWord[0] = 0;
    
    uttproc_set_startword (startWord);

    if ((sf >= 0) && (ef > 0)) {
	sprintf (utt, "./%s", utt_name);
	mfcseg_extract (mfcfile, sf, ef, utt);
	strcpy (mfcfile, utt);
    }
    
    ret = utt_file2feat (mfcfile, 0);
    
    if ((sf >= 0) && (ef > 0)) {
	strcat (utt, ".");
	strcat (utt, cep_ext);
	unlink (utt);
    }

    if (ret < 0)
	return NULL;

    if (uttproc_result_seg (&frmcount, &hypseg, 1) < 0) {
	E_ERROR("uttproc_result_seg(%s) failed\n", uttproc_get_uttid());
	return NULL;
    }
    search_result (&frmcount, &finalhyp);
    
    /* Should the Nbest generation be in uttproc.c (uttproc_result)?? */
    if (nbest > 0) {
	FILE *nbestfp;
	char nbestfile[4096];
	search_hyp_t *h, **alt;
	int32 i, n_alt, startwid;
	
	startwid = kb_get_word_id ("<s>");
	search_save_lattice ();
	n_alt = search_get_alt (nbest, 0, searchFrame(), -1, startwid, &alt);
	
	sprintf (nbestfile, "%s/%s.%s", nbest_dir, utt_name, nbest_ext);
	if ((nbestfp = fopen (nbestfile, "w")) == NULL) {
	    E_WARN("fopen(%s,w) failed; using stdout\n", nbestfile);
	    nbestfp = stdout;
	}
	for (i = 0; i < n_alt; i++) {
	    for (h = alt[i]; h; h = h->next)
		fprintf (nbestfp, "%s ", h->word);
	    fprintf (nbestfp, "\n");
	}
	if (nbestfp != stdout)
	    fclose (nbestfp);
    }

    if (phone_conf) {
	search_hyp_t *allp, *search_uttpscr2allphone();
	
	allp = search_uttpscr2allphone();
	search_hyp_free (allp);
    }
    
    if (pscr2lat)
	search_uttpscr2phlat_print ();

    return hypseg;
}


/*
 * Run time align on a semi-continuous utterance.
 */
void
time_align_utterance (char const *utt,
		      FILE *out_sent_fp,
		      char const *left_word,
		      int32 begin_frame,
		       /* FIXME: Gets modified in time_align.c - watch out! */
		      char *pe_words,
		      int32 end_frame,
		      char const *right_word)
{
    int32 n_frames;
#ifndef WIN32
    struct rusage start, stop;
    struct timeval e_start, e_stop;
#endif

    if ((begin_frame != NO_FRAME) || (end_frame != NO_FRAME)) {
	E_ERROR ("%s(%d): Partial alignment not implemented\n", __FILE__, __LINE__);
	return;
    }

    if ((n_frames = uttproc_get_featbuf (&cep, &dcep, &dcep_80ms, &pcep, &ddcep)) < 0) {
	E_ERROR("#input speech frames = %d\n", n_frames);
	return;
    }

    time_align_set_input (cep, dcep, dcep_80ms, pcep, ddcep, n_frames);

#ifndef WIN32
#ifndef _HPUX_SOURCE
    getrusage (RUSAGE_SELF, &start);
#endif /* _HPUX_SOURCE */
    gettimeofday (&e_start, 0);
#endif /* WIN32 */

    if (time_align_word_sequence(utt,left_word, pe_words, right_word) == 0) {
	if (seg_file_ext) {
	    unsigned short *seg;
	    int seg_cnt;
	    char seg_file_basename[MAXPATHLEN+1];

	    switch (time_align_seg_output(&seg, &seg_cnt)) {
		case NO_SEGMENTATION:
		E_ERROR("NO SEGMENTATION for %s\n", utt);
		break;

		case NO_MEMORY:
		E_ERROR("NO MEMORY for %s\n", utt);
		break;

		default:
		{
		    /* write the data in the same location as the cep file */
		    if (data_directory && (utt[0] != '/')) {
			if (seg_data_directory) {
			    sprintf (seg_file_basename, "%s/%s.%s", seg_data_directory, utt, seg_file_ext);
			}
			else {
			    sprintf (seg_file_basename, "%s/%s.%s", data_directory, utt, seg_file_ext);
			}
		    }
		    else {
			if (seg_data_directory) {
			    char *spkr_dir;
			    char basename[MAXPATHLEN];
			    char *sl;

			    strcpy(basename, utt);
			    
			    sl = strrchr(basename, '/');
			    *sl = '\0';
			    sl = strrchr(basename, '/');
			    spkr_dir = sl + 1;
			    
			    sprintf (seg_file_basename, "%s/%s/%s.%s",
				     seg_data_directory, spkr_dir, utt_name, seg_file_ext);
			}
			else {
			    sprintf (seg_file_basename, "%s.%s", utt, seg_file_ext);
			}
		    }
		    E_INFO("%s(%d): Seg output %s\n",
			    __FILE__, __LINE__, seg_file_basename);
		    awriteshort(seg_file_basename, seg, seg_cnt);
		}
	    }
	}

	if (out_sent_fp) {
	    char const *best_word_str = time_align_best_word_string();
		
	    if (best_word_str) {
		fprintf(out_sent_fp, "%s (%s)\n", best_word_str, utt_name);
	    }
	    else {
		fprintf(out_sent_fp, "NO BEST WORD SEQUENCE for %s\n", utt);
	    }
	}
    }
    else {
	E_ERROR("%s(%d): No alignment for %s\n", __FILE__, __LINE__, utt_name);
    }
    
#ifndef WIN32
#ifndef _HPUX_SOURCE
    getrusage (RUSAGE_SELF, &stop);
#endif /* _HPUX_SOURCE */
    gettimeofday (&e_stop, 0);

    E_INFO(" %5.2f SoS", n_frames*0.01);
    E_INFO(", %6.2f sec elapsed", MakeSeconds (&e_start, &e_stop));
    if (n_frames > 0)
	E_INFO(", %5.2f xRT", MakeSeconds (&e_start, &e_stop)/(n_frames*0.01));
    
#ifndef _HPUX_SOURCE
    E_INFO(", %6.2f sec CPU", MakeSeconds (&start.ru_utime, &stop.ru_utime));
    if (n_frames > 0)
	E_INFO(", %5.2f xRT",
		MakeSeconds (&start.ru_utime, &stop.ru_utime)/(n_frames*0.01));
#endif /* _HPUX_SOURCE */
    E_INFO("\n");
    
    TotalCPUTime += MakeSeconds (&start.ru_utime, &stop.ru_utime);
    TotalElapsedTime += MakeSeconds (&e_start, &e_stop);
    TotalSpeechTime += n_frames * 0.01;
#endif /* WIN32 */
}


char const *get_ref_sent (void)
{
    return (ref_sentence);
}

char const *get_current_startword (void)
{
    return startWord;
}

int32 query_compute_all_senones (void)
{
    return compute_all_senones;
}

#if 0
char *query_utt_id (void)
{
    return (utt_name);
}
#endif

int32 query_lm_cache_lines ( void )
{
    return (lm_cache_lines);
}

int32 query_report_altpron ( void )
{
    return (report_altpron);
}

int32 query_lattice_size ( void )
{
    return (lattice_size);
}

char const *query_match_file_name ( void )
{
    return (match_file_name);
}

char const *query_matchseg_file_name ( void )
{
    return (matchseg_file_name);
}

#ifdef USE_ILM
int32 query_ilm_ugcache_wt ( void )
{
    return ilm_ugcache_wt;
}

int32 query_ilm_bgcache_wt ( void )
{
    return ilm_bgcache_wt;
}
#endif

int32 query_fwdtree_flag ( void )
{
    return fwdtree_flag;
}

int32 query_fwdflat_flag ( void )
{
    return fwdflat_flag;
}

int32 query_bestpath_flag ( void )
{
    return bestpath_flag;
}

int32 query_topsen_window ( void )
{
    return topsen_window;
}

int32 query_topsen_thresh ( void )
{
    return topsen_thresh;
}

int32 query_blocking_ad_read (void)
{
    return (blocking_ad_read_p);
}

char const *query_dumplat_dir ( void )
{
    return dumplat_dir;
}

int32 query_adc_input ( void )
{
    return adc_input;
}

int32 set_adc_input (int32 value)
{
    int32 old_value;
    
    old_value = adc_input;
    adc_input = value;
    return old_value;
}

char const *query_cdcn_file ( void )
{
    return cdcn_file;
}

int32 query_sampling_rate ( void )
{
    return sampling_rate;
}

char const *query_ctlfile_name ( void )
{
    return ctl_file_name;
}

int32 query_phone_conf ( void )
{
    return phone_conf;
}
