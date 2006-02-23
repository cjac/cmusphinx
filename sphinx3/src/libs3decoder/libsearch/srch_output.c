/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
/* srch_output.c
 * HISTORY
 * $Log$
 * Revision 1.2  2006/02/23  05:13:26  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: A wrapper for the outptu functions of the search routine
 * 
 * Revision 1.1.2.1  2006/01/17 01:24:21  arthchan2003
 * The home of log output functions.
 *
 */

#include "srch_output.h"
#include "logs3.h"

int32 compute_scale(int32 sf, int32 ef, int32* scalearray)
{
  int32 i;
  int32 hypscale=0;
  for(i = sf ; i < ef ;i++)
    hypscale += scalearray[i];
  return hypscale;

}
/*
  READ ME.
  Current match_write has four features which is different with log_hypstr. 
  1, match_write allows the use of hdr. 
  2, log_hypstr allows matchexact in output. 
  3, log_hypstr allows output the score after the match file name. 
  4, log_hypstr will dump the pronounciation variation to the code. 

  I don't think they are very important in processing so I removed them.
 */

void match_write (FILE *fp, glist_t hyp, char* uttid, dict_t* dict, char *hdr)
{
    gnode_t *gn;
    srch_hyp_t *h;
    int counter=0;

    if (hyp ==NULL)	/* Following s3.0 convention */
	fprintf (fp, "(null)");

    fprintf (fp, "%s", (hdr ? hdr : ""));

#if 0
    for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
#endif
    for (gn = hyp; gn ; gn = gnode_next(gn)) {
      h = (srch_hyp_t *) gnode_ptr (gn);
      if((!dict_filler_word(dict,h->id)) && (h->id!=dict_finishwid(dict)) && (h->id!=dict_startwid(dict)))
	fprintf(fp,"%s ",dict_wordstr(dict, dict_basewid(dict,h->id)));
      counter++;
    }
    if(counter==0) fprintf(fp," ");
    fprintf (fp, "(%s)\n", uttid);
    fflush (fp);
}


void matchseg_write (FILE *fp, glist_t hyp, char *uttid, char *hdr, int32 fmt,
		     lm_t *lm, dict_t *dict, int32 num_frm, int32* ascale, int32 unnorm
		     )
{
    gnode_t *gn;
    srch_hyp_t *h;
    int32 ascr, lscr, scl, hypscale;
    int32 i;

    ascr = 0;
    lscr = 0;
    scl =0;
    hypscale=0;
    
    for (gn = hyp; gn; gn = gnode_next(gn)) {
	h = (srch_hyp_t *) gnode_ptr (gn);

	ascr += h->ascr;
	lscr += lm_rawscore(lm,lscr);

    }

    for(i=0;i<num_frm;i++)
      scl+=ascale[i];

    if(fmt>SEG_FMT_CTM || fmt <0) {
      E_ERROR("Unknown file format %d, backoff to the default fmt\n");
      fmt=SEG_FMT_SPHINX3;
    }

    if(fmt==SEG_FMT_SPHINX3){
      fprintf (fp, "%s%s S %d T %d A %d L %d", (hdr ? hdr : ""), uttid,
	       scl, ascr+lscr, ascr, lscr);

#if 0      
      for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
#else
      for (gn = hyp; gn; gn = gnode_next(gn)) {
#endif
	h = (srch_hyp_t *) gnode_ptr (gn);
	
	  hypscale=0;
	  if(unnorm)
	    hypscale+=compute_scale(h->sf,h->ef,ascale);
	  
	  
	  fprintf (fp, " %d %d %d %s", h->sf, h->ascr+ hypscale, lm_rawscore(lm, h->lscr),
		   dict_wordstr(dict, h->id));
      }
      fprintf (fp, " %d\n", num_frm);
    }else if(fmt==SEG_FMT_SPHINX2){
      fprintf (fp, "%s%s   ",(hdr ? hdr : ""),uttid);

#if 0
      for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
#else
      for (gn = hyp; gn; gn = gnode_next(gn)) {
#endif
	h = (srch_hyp_t *) gnode_ptr (gn);

	  hypscale=0;
	  if(unnorm)
	    hypscale+=compute_scale(h->sf,h->ef,ascale);
	  
	  /*FIXME!, what is the second output of the matchseg file?*/
	  fprintf (fp, "%s 0 %d %d %d %d ", dict_wordstr(dict, h->id), h->sf, h->ef, h->ascr, lm_rawscore(lm, h->lscr));

      }
      fprintf (fp, "(S= %d (A= %d L= %d))\n", ascr+lscr, ascr, lscr);
    }else if(fmt==SEG_FMT_CTM){

#if 0
      for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
#else
      for (gn = hyp; gn; gn = gnode_next(gn)) {
#endif

	h = (srch_hyp_t *) gnode_ptr (gn);
	/*FIXME!, what is the second output of the matchseg file?*/

	fprintf (fp, "%s%s %s %f %f %s",
		 (hdr ? hdr : ""),
		 uttid,
		 CTM_CHANNEL_MARKER, 
		 (float32)(h->sf)/100.0,
		 (float32)(h->ef)/100.0,

		 dict_wordstr(dict, h->id));

	  
	if(h->cscr <= 0){  /*This is a bad way to decide whether to dump the score */
	    /* The score is in log domain and prescaled by logs3_10base() * 39.5  / lm->lw */
	  /* BUG! What about underflow? */
	  fprintf(fp," %f\n", (float32)logs3_to_log( h->cscr * (int) lm->lw / (int) 39.5 ));
	}else{
	  fprintf(fp," %f\n", CTM_CONFIDENCE_SCORE_STUB);	
	}
      }
      
    }


    fflush (fp);

}

void match_detailed(FILE* fp,glist_t hyp, char* uttid, char* LBL, char* lbl, int32* senscale, dict_t *dict)
{
  int32 ascr, lscr;
  int32 scl;
  gnode_t *gn;
  srch_hyp_t *h;

  ascr=lscr=scl=0;
  assert(dict);
  if(senscale){
    fprintf (fp, "%s:%s> %20s %5s %5s %12s %10s %10s %10s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr(UnNorm)", "LMScore", "AScr+LScr", "AScale");
  }else{
    fprintf (fp, "%s:%s> %20s %5s %5s %12s %10s %10s %10s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr(Norm)", "LMScore", "AScr+LScr", "AScale");
  }

  for (gn = hyp; gn; gn = gnode_next(gn)) {
    h = (srch_hyp_t *) gnode_ptr (gn);

    if(h->id!=dict_finishwid(dict) && h->id!=dict_startwid(dict)){
    
      scl=0;

      scl+=compute_scale(h->sf,h->ef,senscale);
      
      if(senscale){
	fprintf (fp, "%s:%s> %20s %5d %5d %12d %10d %10d %10d \n",lbl, uttid,
		 dict_wordstr(dict, h->id), h->sf,h->ef, h->ascr+scl, h->lscr,h->ascr+scl+h->lscr, scl
		 );
      }else{
	fprintf (fp, "%s:%s> %20s %5d %5d %12d %10d %10d %10d\n",lbl, uttid,
		 dict_wordstr(dict, h->id), h->sf,h->ef, h->ascr, h->lscr, h->ascr+h->lscr, scl
		 );
      }
      
      ascr += h->ascr;
      
      if(senscale)
	ascr+=scl;
      
      lscr += h->lscr;
    }
  }

  fprintf (fp, "%s:%s> %20s %5s %5s %12d %10d\n", LBL, uttid,
	   "TOTAL", "", "", ascr, lscr);

}

/* CODE DUPLICATION! Sphinx 3.0 family of logging hyp and hyp segments */
/* Write hypothesis in old (pre-Nov95) NIST format */
void log_hypstr (FILE *fp, srch_hyp_t *hypptr, char *uttid, int32 exact, int32 scr,dict_t *dict)
{
    srch_hyp_t *h;
    s3wid_t w;
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, "(null)");
    
    for (h = hypptr; h; h = h->next) {
      if(h->sf!=h->ef){ /* Take care of abnormality caused by various different reasons*/
	w = h->id;
	if (! exact) {
	    w = dict_basewid (dict,w);
	    if ((w != dict->startwid) && (w != dict->finishwid) && (! dict_filler_word (dict,w)))
		fprintf (fp, "%s ", dict_wordstr(dict,w));
	} else
	    fprintf (fp, "%s ", dict_wordstr(dict,w));
      }
    }
    if (scr != 0)
	fprintf (fp, " (%s %d)\n", uttid, scr);
    else
	fprintf (fp, " (%s)\n", uttid);
    fflush (fp);
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
void log_hypseg (char *uttid,
		 FILE *fp,	/* Out: output file */
		 srch_hyp_t *hypptr,	/* In: Hypothesis */
		 int32 nfrm,	/* In: #frames in utterance */
		 int32 scl,	/* In: Acoustic scaling for entire utt */
		 float64 lwf,	/* In: LM score scale-factor (in dagsearch) */
		 dict_t* dict,  /* In: dictionary */
		 lm_t *lm,      
		 int32 unnorm   /**< Whether unscaled the score back */
		 )
{
    srch_hyp_t *h;
    int32 ascr, lscr, tscr;
    
    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
	ascr += h->ascr;
	if (dict_basewid(dict,h->id) != dict->startwid) {
	    lscr += lm_rawscore (lm,h->lscr);
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
	    lscr = (dict_basewid(dict,h->id) != dict->startwid) ? lm_rawscore (lm,h->lscr) : 0;
	    fprintf (fp, " %d %d %d %s", h->sf, h->ascr, lscr, dict_wordstr (dict,h->id));
	}
	fprintf (fp, " %d\n", nfrm);
    }
    
    fflush (fp);
}



/* Log hypothesis in detail with word segmentations, acoustic and LM scores  */
void log_hyp_detailed (FILE *fp, srch_hyp_t *hypptr, char *uttid, char *LBL, char *lbl, int32* senscale)
{
    srch_hyp_t *h;
    int32 scale, ascr, lscr;

    ascr = 0;
    lscr = 0;

    if(senscale){
      fprintf (fp, "%s:%s> %20s %5s %5s %12s %10s %10s %10s \n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr(UnNorm)", "LMScore","AScr+LScr","AScale");
    }else{
      fprintf (fp, "%s:%s> %20s %5s %5s %12s %10s %10s %10s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr(Norm)", "LMScore","AScr+LScr", "AScale");
    }
    
    for (h = hypptr; h; h = h->next) {
	scale = 0;
       
	if(senscale)
	  scale =compute_scale(h->sf,h->ef,senscale);


	if(senscale){
	  fprintf (fp, "%s:%s> %20s %5d %5d %12d %10d %10d %10d\n", lbl, uttid,
		 h->word, h->sf, h->ef, h->ascr + scale, h->lscr, h->ascr + scale + h->lscr, scale);
	}else{
	  fprintf (fp, "%s:%s> %20s %5d %5d %12d %10d %10d %10d\n", lbl, uttid,
		 h->word, h->sf, h->ef, h->ascr, h->lscr, h->ascr + h->lscr, scale);
	}

	ascr += h->ascr ;

	if(senscale)
	  ascr += scale;

	lscr += h->lscr;
    }

    fprintf (fp, "%s:%s> %20s %5s %5s %12d %10d\n", LBL, uttid,
	     "TOTAL", "", "", ascr, lscr);
}


static int get_word(char **string, char *word)
{
  char *p = word;
  
  while ((**string != '\0') && ((**string == '\n') || (**string == '\t') || (**string == ' ')))
    (*string)++;
  
  while ((**string != '\0') && (**string != '\n') && (**string != '\t') && (**string != ' '))
    *p++ = *(*string)++;
  
  *p = '\0';
  return strlen(word);
}

/*Code we need to incorporate into 3.6, a seg reader */
/*****************************************************************************/

/*
  %s     S 0 T    %d            A    %   d    L   %   d    %d    %d    %d    %s         %d 
uttid          totascr+totlscr      totascr      totlscr   sf   ascr  lscr  word        nfr
*/

int read_s3hypseg_line(char *line, seg_hyp_line_t *seg_hyp_line, lm_t* lm, dict_t *dict)
{
  char *p, str[128];
  conf_srch_hyp_t *hyp_word, *tail, *g, *h;
  int  sum, t, i;
  s3wid_t wid;
  
  p = line;

  if (!get_word(&p, str)) {
    printf("failed to read sequence number in the line: %s\n", line);
    return HYPSEG_FAILURE;
  }

  strcpy(seg_hyp_line->seq, str);
  
  if (!get_word(&p, str) || strcmp(str, "S"))
    E_FATAL("failed to read S in the line: %s\n", line);

  get_word(&p,str);

  if (!get_word(&p, str)|| strcmp(str, "T")) 
    E_FATAL("failed to read T in the line: %s\n", line);

  if (!get_word(&p, str)) 
    E_FATAL("failed to read ascr+lscr in the line: %s\n", line);

  sum = atoi(str);

  if (!get_word(&p, str)|| strcmp(str, "A") )
    E_FATAL("failed to read A in the line: %s\n", line);

  if (!get_word(&p, str)) 
    E_FATAL("failed to read ascr in the line: %s\n", line);

  seg_hyp_line->ascr = atoi(str);

  if (!get_word(&p, str)||strcmp(str, "L")) 
    E_FATAL("failed to read L in the line: %s\n", line);

  if (!get_word(&p, str))
    E_FATAL("failed to read lscr in the line: %s\n", line);

  seg_hyp_line->lscr = atoi(str);

#if 0
  if (!get_word(&p, str) || strcmp(str, "0")) {
    E_FATAL("failed to find 0 in the line: %s\n", line);

  }
#endif

  if (seg_hyp_line->ascr + seg_hyp_line->lscr != sum) {
    E_FATAL("the sum of ascr and lscr %d is wrong (%d): %s\n",
	    seg_hyp_line->ascr + seg_hyp_line->lscr, sum, line);
  }
  
  seg_hyp_line->wordlist = NULL;
  seg_hyp_line->wordno = 0;
  seg_hyp_line->nfr = 0;
  seg_hyp_line->cscore = WORST_CONFIDENCE_SCORE;
  tail = NULL;

  while (1) {
    if (!get_word(&p, str)) 
      E_FATAL("failed to read sf or nfr in the line: %s\n", line);

    t = atoi(str);

    if (!get_word(&p, str)) {
      seg_hyp_line->nfr = t;
      break;
    }

    if ((hyp_word = (conf_srch_hyp_t *)ckd_calloc(1,sizeof(conf_srch_hyp_t))) == NULL ||
	(hyp_word->sh.word=(char*) ckd_calloc(1024,sizeof(char)))==NULL
	) {
      E_FATAL("fail to allocate memory\n");
    }

    hyp_word->sh.sf = t;
    hyp_word->sh.ascr = atoi(str);
    hyp_word->next = NULL;

    if (!get_word(&p, str)) 
      E_FATAL("failed to read lscr in the line: %s\n", line);

    hyp_word->sh.lscr = atoi(str);

    if (!get_word(&p, str)) 
      E_FATAL("failed to read word in the line: %s\n", line);

    strcpy(hyp_word->sh.word, str);
    for (i = strlen(str) - 1; i >= 0; i--)
      if (str[i] == '(')
	break;
    if (i >= 0)
      str[i] = '\0';


    if (dict) {
      wid=dict_wordid(dict,str);
      if(wid==BAD_S3WID){
	E_FATAL("String %s doesn't exists in the dictionary\n",str);
      }

      hyp_word->sh.id=wid;
    }
    
    hyp_word->compound = 0;
    hyp_word->matchtype = 0;
    
    seg_hyp_line->wordno++;
    if (seg_hyp_line->wordlist == NULL)
      seg_hyp_line->wordlist = hyp_word;
    else
      tail->next = hyp_word;
    tail = hyp_word;
  }


  if (seg_hyp_line->wordlist == NULL) {
    printf("word list is NULL\n");
    return HYPSEG_FAILURE;
  }

  g = seg_hyp_line->wordlist;
  for (h = g->next; h; h = h->next) {
    g->sh.ef = h->sh.sf - 1;
    g = h;
  }
  g->sh.ef = seg_hyp_line->nfr - 1;

  sum = 0;
  for (h = seg_hyp_line->wordlist; h; h = h->next)
    sum += h->sh.ascr;
  if (sum != seg_hyp_line->ascr) 
    E_FATAL("the ascr of words is not equal to the ascr of utt: %s\n", line);

  sum = 0;
  for (h = seg_hyp_line->wordlist; h; h = h->next)
    sum += h->sh.lscr;

  if (sum != seg_hyp_line->lscr) 
    E_WARN("the lscr of words is not equal to the lscr of utt: %s %d %d\n", seg_hyp_line->seq, sum, seg_hyp_line->lscr);

  for (h = seg_hyp_line->wordlist; h; h = h->next){
    if (h->sh.ef <= h->sh.sf) {
      E_FATAL("word %s ef <= sf in the line: %s\n", h->sh.word, line);
    }
  }

  return HYPSEG_SUCCESS;
}


int free_seg_hyp_line(seg_hyp_line_t *seg_hyp_line)
{
  conf_srch_hyp_t *w, *nw;

  for (w = seg_hyp_line->wordlist; w; w = nw) {
    nw = (conf_srch_hyp_t*)w->next;
    free(w->sh.word);
    free(w);
  }
  return HYPSEG_SUCCESS;
}

