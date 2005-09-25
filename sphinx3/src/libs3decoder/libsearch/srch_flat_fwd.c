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

/* srch_flat_fwd.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.1.2.2  2005/09/18  01:45:19  arthchan2003
 * Filled in all implementation in srch_flat_fwd.[ch], like the FSG mode, it takes care of reporting itselft.
 * 
 * Revision 1.1.2.1  2005/07/24 01:40:37  arthchan2003
 * (Incomplete) The implementation of flat-lexicon decoding.
 *
 *
 *
 */

#include "srch_flat_fwd.h"
#include "srch.h"
#include "whmm.h"

/** FIXME! I am ugly */
extern int32 *st_sen_scr;


void dump_all_whmm (srch_FLAT_FWD_graph_t *fwg, whmm_t **whmm, int32 n_frm, int32 n_state, int32 *senscr);

void dump_all_word (srch_FLAT_FWD_graph_t *fwg, whmm_t **whmm, int32 n_state);

void whmm_renorm (srch_FLAT_FWD_graph_t *fwg, whmm_t **whmm,int32 n_state, int32 bestscr);

void whmm_transition (srch_FLAT_FWD_graph_t *fwg, whmm_t** whmm, int32 w, whmm_t *h, int32 n_state, int32 final_state) ;

void word_enter (srch_FLAT_FWD_graph_t *fwg, s3wid_t w, int32 n_state, int32 score, s3latid_t l, s3cipid_t lc);

int32 whmm_eval (srch_FLAT_FWD_graph_t *fwg, int32 *senscr, int32 n_state);

void whmm_exit (srch_FLAT_FWD_graph_t *fwg,
		whmm_t **whmm, 
		latticehist_t *lathist, 
		int32 n_state, 
		int32 final_state, 
		int32 thresh, 
		int32 wordthresh, 
		int32 phone_penalty);


void word_trans (srch_FLAT_FWD_graph_t* fwg, 
		 whmm_t **whmm, 
		 int32 n_state, 
		 latticehist_t* lathist, 
		 int32 thresh, 
		 int32 phone_penalty);


dag_t* dag_build (srch_FLAT_FWD_graph_t* fwg, s3latid_t endid, latticehist_t * lathist, dict_t *dict, lm_t *lm, ctxt_table_t* ctxt, fillpen_t* fpen);

void s3flat_fwd_dag_dump(char *dir, int32 onlynodes, char *id, char* latfile_ext, latticehist_t *lathist, int32 n_frm, dag_t *dag, 
			 lm_t *lm, dict_t *dict, ctxt_table_t *ctxt, fillpen_t *fpen
			 );


static fwd_dbg_t* init_fwd_dbg(srch_FLAT_FWD_graph_t *fwg)
{
    char *tmpstr;
    fwd_dbg_t *fd;

    fd=(fwd_dbg_t*) ckd_calloc(1,sizeof(fwd_dbg_t));

    assert(fd);
    /* Word to be traced in detail */
    if ((tmpstr = (char *) cmd_ln_access ("-tracewhmm")) != NULL) {
	fd->trace_wid = dict_wordid (fwg->kbcore->dict,tmpstr);
	if (NOT_S3WID(fd->trace_wid))
	    E_ERROR("%s not in dictionary; cannot be traced\n", tmpstr);
    } else
	fd->trace_wid = BAD_S3WID;

    /* Active words to be dumped for debugging after and before the given frame nos, if any */
    fd->word_dump_sf=(int32) 0x7ffffff0;
    if(cmd_ln_int32("-worddumpsf"))
      fd->word_dump_sf=cmd_ln_int32("-worddumpsf");

    fd->word_dump_ef=(int32) 0x7ffffff0;
    if(cmd_ln_int32("-worddumpef"))
      fd->word_dump_ef=cmd_ln_int32("-worddumpef");

    /* Active HMMs to be dumped for debugging after and before the given frame nos, if any */
    fd->hmm_dump_sf=(int32) 0x7ffffff0;
    if(cmd_ln_int32("-hmmdumpsf"))
      fd->hmm_dump_sf=cmd_ln_int32("-hmmdumpsf");

    fd->hmm_dump_ef=(int32) 0x7ffffff0;
    if(cmd_ln_int32("-hmmdumpef"))
      fd->hmm_dump_ef=cmd_ln_int32("-hmmdumpef");

    return fd;
}

/** ARCHAN: Dangerous! Mixing global and local */
static void dump_fwd_dbg_info(srch_FLAT_FWD_graph_t *fwg, fwd_dbg_t *fd, ascr_t *ascr, int32 bestscr, int32 whmm_thresh, int32 word_thresh,int32* senscr)
{
  whmm_t *h;
  int32 n_frm;
  int32 n_state;
  kbcore_t *kbc;
  tmat_t *tmat;
  dict_t *dict;
  mdef_t *mdef;

  n_frm=fwg->n_frm;  
  n_state=fwg->n_state;
  kbc=fwg->kbcore;
  dict=kbcore_dict(kbc);
  tmat=kbcore_tmat(kbc);
  mdef=kbcore_mdef(kbc);

  /* Dump bestscore and pruning thresholds if any detailed tracing specified */
  if (((fd->hmm_dump_sf  < n_frm) && (n_frm <  fd->hmm_dump_ef)) || 
      ((fd->word_dump_sf < n_frm) && (n_frm < fd->word_dump_ef)) ||
      (IS_S3WID(fd->trace_wid) && fwg->whmm[fd->trace_wid])) {
    printf ("[%4d]: >>>> bestscore= %11d, whmm-thresh= %11d, word-thresh= %11d\n",
	    n_frm, bestscr, whmm_thresh, word_thresh);
  }
    
  /* Dump all active HMMs or words, if indicated */
  if (fd->hmm_dump_sf < n_frm && n_frm < fd->hmm_dump_ef)
    dump_all_whmm (fwg, fwg->whmm, n_frm, n_state, ascr->senscr);
  else if (fd->word_dump_sf < n_frm && n_frm < fd->word_dump_ef)
    dump_all_word (fwg, fwg->whmm, n_state);
  
  /* Trace active HMMs for specified word, if any */
  if (IS_S3WID(fd->trace_wid)) {
    for (h = fwg->whmm[fd->trace_wid]; h; h = h->next)
      dump_whmm (fd->trace_wid, h, senscr, tmat, n_frm, n_state, dict,mdef);
  }

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




int srch_FLAT_FWD_init(kb_t *kb, /**< The KB */
		       void* srch /**< The pointer to a search structure */
		       )
{
  srch_FLAT_FWD_graph_t* fwg;
  kbcore_t* kbc;
  srch_t* s;
  mdef_t* mdef;
  dict_t* dict;
  lm_t* lm;

  kbc=kb->kbcore;
  s=(srch_t *)srch;
  mdef=kbcore_mdef(kbc);
  dict=kbcore_dict(kbc);
  lm=kbcore_lm(kbc);

  fwg=ckd_calloc(1,sizeof(srch_FLAT_FWD_graph_t));

  E_INFO("Initialization\n");
  /* HMM states information */
  fwg->n_state = mdef->n_emit_state + 1;
  fwg->final_state = fwg->n_state - 1;

  /* Search control information */
  fwg->multiplex=cmd_ln_int32("-multiplex_multi");
  fwg->multiplex_singleph=cmd_ln_int32("-multiplex_single");

  /* ARCHAN : BUG, though allowing both options of multiple_multi
     and multiplex_single, currently !multiplex &&
     multiplex_singleph is not taken care correctly. */

  if(fwg->multiplex && !(fwg->multiplex_singleph))
    E_FATAL("Forced exit: Disallow de-multiplex a single phone word without de-multiplexing multi phone word");

  /* Allocate whmm structure */
  fwg->whmm = (whmm_t **) ckd_calloc (dict->n_word, sizeof(whmm_t *));

  /* Data structures needed during word transition */
  /* These five things need to be tied into the same structure.  Such that when multiple LM they could be switched.  */
  fwg->rcscore= NULL;
  fwg->rcscore = (int32 *) ckd_calloc (mdef->n_ciphone, sizeof(int32));
  fwg->ug_backoff = (backoff_t *) ckd_calloc (mdef->n_ciphone, sizeof(backoff_t));
  fwg->filler_backoff = (backoff_t *) ckd_calloc (mdef->n_ciphone, sizeof(backoff_t));
  fwg->tg_trans_done = (uint8 *) ckd_calloc (dict->n_word, sizeof(uint8));
  fwg->word_ugprob = init_word_ugprob(mdef,lm,dict);

  /* Input candidate-word lattices information to restrict search; if any */
  fwg->word_cand_dir = (char *) cmd_ln_access ("-inlatdir");
  fwg->latfile_ext = (char *) cmd_ln_access ("-latext");
  fwg->word_cand_win = *((int32 *) cmd_ln_access ("-inlatwin"));
  if (fwg->word_cand_win < 0) {
    E_ERROR("Invalid -inlatwin argument: %d; set to 50\n", fwg->word_cand_win);
    fwg->word_cand_win = 50;
  }
  /* Allocate pointers to lists of word candidates in each frame */
  if (fwg->word_cand_dir) {
    fwg->word_cand = (word_cand_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(word_cand_t *));
    fwg->word_cand_cf = (s3wid_t *) ckd_calloc (dict->n_word+1, sizeof(s3wid_t));
  }


  /* Initializing debugging information such as trace_wid,
     word_dump_sf, word_dump_ef, hmm_dump_sf and hmm_dump_ef */

  fwg->fwdDBG=init_fwd_dbg(fwg);

  fwg->ctr_mpx_whmm=pctr_new("mpx");
  fwg->ctr_nonmpx_whmm=pctr_new("~mpx");
  fwg->ctr_latentry=pctr_new("lat");

    /** Initialize the context table */
  fwg->ctxt=ctxt_table_init(kbcore_dict(kbc),kbcore_mdef(kbc));

  /* Variables for speeding up whmm evaluation */
  st_sen_scr = (int32 *) ckd_calloc (fwg->n_state-1, sizeof(int32));

  /** For convenience */
  fwg->kbcore=s->kbc;

  /* Glue the graph structure */
  s->grh->graph_struct=fwg;
  s->grh->graph_type=GRAPH_STRUCT_FLAT;
  
 

  return SRCH_SUCCESS;
  
}

int srch_FLAT_FWD_uninit(void* srch)
{

  srch_FLAT_FWD_graph_t* fwg ;
  srch_t* s;
  
  s=(srch_t *)srch;
  fwg=(srch_FLAT_FWD_graph_t*) s->grh->graph_struct;

  if(st_sen_scr)
    ckd_free(st_sen_scr);

  if(fwg->rcscore)
    ckd_free(fwg->rcscore);

  if(fwg->ug_backoff)
    ckd_free(fwg->ug_backoff);

  if(fwg->filler_backoff)
    ckd_free(fwg->filler_backoff);

  if(fwg->tg_trans_done)
    ckd_free(fwg->tg_trans_done);
    
  if(fwg->word_cand_cf)
    ckd_free(fwg->word_cand_cf);

  if (fwg->word_cand_dir) {
    if(fwg->word_cand)
      ckd_free(fwg->word_cand);
    if(fwg->word_cand_cf)
      ckd_free(fwg->word_cand_cf);
  }

  pctr_free(fwg->ctr_mpx_whmm);
  pctr_free(fwg->ctr_nonmpx_whmm);
  pctr_free(fwg->ctr_latentry);

  return SRCH_SUCCESS;
}

int srch_FLAT_FWD_begin(void* srch)
{
  srch_FLAT_FWD_graph_t* fwg ;
  srch_t* s;  
  kbcore_t* kbc;
  int32 w, ispipe;
  char str[1024];
  FILE *fp;
  dict_t *dict;
  
  s=(srch_t *)srch;
  fwg=(srch_FLAT_FWD_graph_t*) s->grh->graph_struct;
  kbc=s->kbc;
  dict=kbcore_dict(kbc);

  assert(fwg);

  stat_clear_utt(s->stat);
  ptmr_reset (&(fwg->tm_hmmeval));
  ptmr_reset (&(fwg->tm_hmmtrans));
  ptmr_reset (&(fwg->tm_wdtrans));

  latticehist_reset(s->lathist);

  /* If input lattice file containing word candidates to be searched specified; use it */
  if (fwg->word_cand_dir) {
    sprintf (str, "%s/%s.%s", fwg->word_cand_dir, s->uttid, fwg->latfile_ext);
    E_INFO("Reading input lattice: %s\n", str);
    
    if ((fp = fopen_compchk (str, &ispipe)) == NULL)
      E_ERROR("fopen_compchk(%s) failed; running full search\n", str);
    else {
      if ((fwg->n_word_cand = word_cand_load (fp,fwg->word_cand,dict,s->uttid)) <= 0) {
	E_ERROR("Bad or empty lattice file: %s; ignored\n", str);
	word_cand_free (fwg->word_cand);
	fwg->n_word_cand=0;
      } else
	E_INFO("%d lattice entries read\n", fwg->n_word_cand);
      
      fclose_comp (fp, ispipe);
    }
  }

  if(fwg->n_word_cand>0)
    latticehist_n_cand(s->lathist)=fwg->n_word_cand;
  
  /* Enter all pronunciations of startwid (begin silence) */


  fwg->n_frm=-1;
  for (w = dict->startwid; IS_S3WID(w); w = dict->word[w].alt)
    word_enter (fwg, w, fwg->n_state, 0, BAD_S3LATID,
		dict->word[dict->silwid].ciphone[dict->word[dict->silwid].pronlen-1]);

  fwg->renormalized = 0;
  fwg->n_frm=0;

#if 0
  E_INFO("After\n");

  dump_all_whmm(fwg,fwg->whmm,fwg->n_frm,fwg->n_state,NULL);
#endif

  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_end(void* srch)
{
  srch_FLAT_FWD_graph_t* fwg ;
  srch_t* s;  
  kbcore_t* kbc;
  dict_t *dict;
  srch_hyp_t *hyp, *tmph;
  int32 ascr, lscr, scl;
  int32 i;
  char *bscrdir;
  stat_t* st;
  dag_t *dag;
  float32 *f32arg;
  float64 lwf;
  whmm_t *h, *nexth;
  s3wid_t w;
  lm_t *lm;
  s3latid_t l;
  FILE *bptfp;

  s=(srch_t *)srch;
  fwg=(srch_FLAT_FWD_graph_t*) s->grh->graph_struct;
  kbc=s->kbc;
  dict=kbcore_dict(kbc);
  st = s->stat;

  lm=s->kbc->lmset->cur_lm;

  s->lathist->frm_latstart[fwg->n_frm] = s->lathist->n_lat_entry;	/* Add sentinel */
  pctr_increment (fwg->ctr_latentry, s->lathist->n_lat_entry);

  /* Free whmm search structures */

  for (w = 0; w < dict->n_word; w++) {
    for (h = fwg->whmm[w]; h; h = nexth) {
      nexth = h->next;
      
      if(dict->word[w].pronlen == 1)
	assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex_singleph));
      else
	assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex));
      
      whmm_free (h);
    }
    fwg->whmm[w] = NULL;
  }

  if (fwg->n_word_cand > 0){
    word_cand_free (fwg->word_cand);
    fwg->n_word_cand=0;
  }

  /* Check if bptable should be dumped (for debugging) */
  if (cmd_ln_str("-bptbldir")) {
    char file[8192];
    sprintf (file, "%s/%s.bpt",cmd_ln_str("-bptbldir") , s->uttid);
    if ((bptfp = fopen (file, "w")) == NULL) {
      E_ERROR("fopen(%s,w) failed; using stdout\n", file);
      bptfp = stdout;
    }
    
    latticehist_dump (s->lathist, bptfp, s->kbc->dict, fwg->ctxt, 0);
    
    if (bptfp != stdout)
      fclose (bptfp);
  }
  /* Get rid of old hyp, if any */
  hyp_free (hyp);

  
  /* Backtrack through lattice for Viterbi result */
  l = lat_final_entry (s->lathist, dict, fwg->n_frm, s->uttid);
  if (NOT_S3LATID(l)){
    E_INFO("lattice ID: %d\n",l);
    E_ERROR("%s: NO RECOGNITION\n", s->uttid);
  }
  else{
    /* BAD_S3WID => Any right context */
    lattice_backtrace (s->lathist, l, BAD_S3WID, &hyp, lm, s->kbc->dict, fwg->ctxt, s->kbc->fillpen);
  }

  /* FIXME */
  if ( cmd_ln_int32("-backtrace") )
    log_hyp_detailed(stdout, hyp, s->uttid, "FV", "fv", NULL);

  /* Total scaled acoustic score and LM score */
  ascr = lscr = 0;
  for (tmph = hyp; tmph; tmph = tmph->next) {
    ascr += tmph->ascr;
    lscr += tmph->lscr;
  }


  /* Print sanitized recognition */
  printf ("FWDVIT: ");
  log_hypstr(stdout, hyp, s->uttid, 0, ascr + lscr,dict);
  lm_cache_stats_dump (lm);


  if(cmd_ln_str("-outlatdir")||cmd_ln_int32("-bestpath")){
    dag=dag_build(fwg,l,s->lathist,s->kbc->dict,lm,fwg->ctxt,s->kbc->fillpen);
    
    if (cmd_ln_str("-outlatdir"))

      /*FIXME, 2nd argument*/
      s3flat_fwd_dag_dump(cmd_ln_str("-outlatdir"), 0, s->uttid, cmd_ln_str("-latext"), 
			  s->lathist, fwg->n_frm, dag, 
			  lm,s->kbc->dict,fwg->ctxt,s->kbc->fillpen);


    if(cmd_ln_int32("-bestpath")){

      f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
      lwf = f32arg ? ((*f32arg) / *((float32 *) cmd_ln_access ("-lw"))) : 1.0;
      
      tmph = dag_search (dag,s->uttid,lwf, s->lathist->lattice[dag->latfinal].dagnode,
		      s->kbc->dict,lm,s->kbc->fillpen);

      if (tmph) 
	hyp = tmph;
      else
	E_ERROR("%s: Bestpath search failed; using Viterbi result\n", s->uttid);

      if (cmd_ln_int32("-backtrace")) {
	/* FIXME */
	log_hyp_detailed (stdout, hyp, s->uttid, "BP", "bp", NULL);
      }
      
      /* Total scaled acoustic score and LM score */
      ascr = lscr = 0;
      for (tmph = hyp; tmph; tmph = tmph->next) {
	ascr += tmph->ascr;
	lscr += tmph->lscr;
      }
      
      printf ("BSTPTH: ");
      /* FIXME */
      log_hypstr (stdout, hyp, s->uttid, 1, ascr + lscr,s->kbc->dict);
      
      printf ("BSTXCT: ");
      /* FIXME */

      log_hypseg (s->uttid, stdout, hyp, fwg->n_frm, 0.0, lwf,s->kbc->dict,lm);

    }
    dag_destroy(dag);

    lm_cache_stats_dump (lm);
    lm_cache_reset (lm);

  }


/* Log recognition output to the standard match and matchseg files */

  printf ("FWDXCT: ");
  /* FIXME */
  log_hypseg (s->uttid, stdout, hyp, fwg->n_frm, 0.0, 1.0,s->kbc->dict,lm);
  lm_cache_stats_dump (lm);


#if 0
  if ((bscrdir = (char *) cmd_ln_access ("-bestscoredir")) != NULL)
    write_bestscore (bscrdir, s->uttid, bestscr, nfr);
#endif

  stat_report_utt(st,s->uttid);
  stat_update_corpus(st);

  
  ptmr_reset (&(st->tm_sen));
  ptmr_reset (&(st->tm_srch));
  ptmr_reset (&(st->tm_ovrhd));
  
#if (!defined(WIN32))
  if (! system("ps auxgw > /dev/null 2>&1")) {
    system ("ps aguxwww | grep /live | grep -v grep");
    system ("ps aguxwww | grep /dec | grep -v grep");
  }
#endif


  return SRCH_SUCCESS;

}

int srch_FLAT_FWD_set_lm(void* srch_struct, const char* lmname)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_add_lm(void* srch, lm_t *lm, const char *lmname)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_delete_lm(void* srch, const char *lmname)
{  
  return SRCH_SUCCESS;
}



int srch_FLAT_FWD_srch_one_frame_lv2(void* srch)
{
  int32 bestscr;	/* Best state score for any whmm evaluated in this frame */
  int32 whmm_thresh;	/* Threshold for any whmm to stay alive in search */
  int32 word_thresh;	/* Threshold for a word-final whmm to succeed */
  int32 phone_penalty;
  srch_FLAT_FWD_graph_t* fwg ;
  srch_t* s;
  
  s=(srch_t *)srch;
  fwg=(srch_FLAT_FWD_graph_t*) s->grh->graph_struct;
  
  ptmr_start (&(fwg->tm_hmmeval));
  bestscr = whmm_eval (fwg,s->ascr->senscr,fwg->n_state);
  /*  E_INFO("bestscr %d RENORM_THRESH %d\n",bestscr, RENORM_THRESH);*/
  ptmr_stop (&(fwg->tm_hmmeval));
  
  whmm_thresh = bestscr + s->beam->hmm;
  word_thresh = bestscr + s->beam->word;
  phone_penalty = logs3(cmd_ln_float32("-phonepen")); 

  assert(s->ascr->senscr);
  /*  E_INFO("fwg->n_frm %d\n",fwg->n_frm);*/
  dump_fwd_dbg_info(fwg, fwg->fwdDBG, s->ascr, bestscr, whmm_thresh, word_thresh,s->ascr->senscr);
  
  {
    ptmr_start (&(fwg->tm_hmmtrans));
    s->lathist->frm_latstart[fwg->n_frm] = s->lathist->n_lat_entry;
    whmm_exit (fwg, fwg->whmm, s->lathist,fwg->n_state, fwg->final_state, whmm_thresh, word_thresh, phone_penalty);
    ptmr_stop (&(fwg->tm_hmmtrans));
    
    /* Please read, the In whmm_exit, if word ends are reach,
       n_lat_entry will increase, see whmm_exit(). Then word_trans
       will be triggered.
    */
    
    ptmr_start (&(fwg->tm_wdtrans));
    if (s->lathist->frm_latstart[fwg->n_frm] < s->lathist->n_lat_entry) 
      word_trans (fwg,fwg->whmm,fwg->n_state,s->lathist, whmm_thresh,phone_penalty);
    ptmr_stop (&(fwg->tm_wdtrans));
  }
  
  if (bestscr < RENORM_THRESH) {
    E_INFO("Frame %d: bestscore= %d; renormalizing\n", fwg->n_frm, bestscr);
    whmm_renorm (fwg,fwg->whmm,fwg->n_state, bestscr);
  }
  
  fwg->n_frm++;
  return SRCH_SUCCESS;
}


int srch_FLAT_FWD_shift_one_cache_frame(void *srch,int32 win_efv)
{
  ascr_t *ascr;
  srch_t* s;

  s=(srch_t*) srch;

  ascr=s->ascr;

  ascr_shift_one_cache_frame(ascr,win_efv);

  return SRCH_SUCCESS;
}

int srch_FLAT_FWD_select_active_gmm(void *srch)
{
  kbcore_t* kbc;
  s3wid_t w;
  whmm_t *h;
  int32 st;
  s3pid_t p;
  s3senid_t *senp;
  ascr_t *ascr;
  srch_FLAT_FWD_graph_t* fwg ;
  srch_t* s;
  mdef_t *mdef;
  dict_t *dict;
  
  s=(srch_t *)srch;
  fwg=(srch_FLAT_FWD_graph_t*) s->grh->graph_struct;
  ascr=s->ascr;
  kbc=s->kbc;
  mdef=kbcore_mdef(kbc);
  dict=kbcore_dict(kbc);

  ascr_clear_sen_active(ascr);
  
  /* Flag active senones */
  for (w = 0; w < dict->n_word; w++) {
    for (h = fwg->whmm[w]; h; h = h->next) {
      
      if(dict->word[w].pronlen == 1)
	assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex_singleph));
      else
	assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex));
      
      if(h->type==MULTIPLEX_TYPE){
	for (st = fwg->n_state-2; st >= 0; --st) {
	  p = h->pid[st];
	  senp = mdef->phone[p].state;
	  ascr->sen_active[senp[st]] = 1;
	}
      }else{
	p = *(h->pid);
	senp = mdef->phone[p].state;
	for (st = fwg->n_state-2; st >= 0; --st)
	  ascr->sen_active[senp[st]] = 1;
	
      }
    }
  }
  
  return SRCH_SUCCESS;
}

int srch_FLAT_FWD_frame_windup(void *srch_struct,int32 frmno){
  return SRCH_SUCCESS;
}
