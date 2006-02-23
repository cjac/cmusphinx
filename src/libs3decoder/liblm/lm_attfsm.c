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
/*
 * lm_attfsm.c -- language model dumping in FSM format. Mainly adapted
 * from LIUM Sphinx's modification. Also doesn't work for class-based
 * LM and bigram at this point. 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.2  2006/02/23  04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert lm to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 * 
 * Revision 1.1.2.1  2006/01/16 19:57:28  arthchan2003
 * Added LIUM's ATT FSM generation routine.
 *
 *
 */

#include "lm.h"

static int32 idx_tg_hist (lm_t *lm, bg_t *bgptr, int32 i, int32 j) { /* return absolute first trigram index for bigram (i, bgptr[j].wid) */
  int32 b;
  
  b=lm->ug[i].firstbg + j;
  return (lm->tg_segbase[b >> lm->log_bg_seg_sz] + bgptr[j].firsttg) ;
}

int32 lm_write_att_fsm(lm_t *lm, const char *filename) { 
  FILE *file;
  int32 i, j, k, l, nb_bg, nb_bg2, nb_tg, bowt;
  char filesymbolsname[2048];
  bg_t *bgptr,*bgptr2;
  tg_t *tgptr;
	
  short prune_lowprobtg=0;

  int32 st_ug=lm_n_ug(lm)+1, st_end=lm_n_ug(lm)+2; /* unigram state, end state */
  int32 nbwtg=lm_n_ug(lm)+3; /* Number of id states used without trigrams */
  int32 id_hist1, id_hist2; /* indexes of trigram histories */

  sprintf(filesymbolsname,"%s.sym",filename);

  if (!(file=fopen(filesymbolsname, "w")))
    E_FATAL ("fopen(%s,w) failed\n", file);
  fprintf(file, "<eps>\t0\n");
  for (i=0;i<lm_n_ug(lm); i++)
    fprintf(file, "%s\t%d\n", lm->wordstr[i], i+1); /* i+1 because id of <eps> HAS TO be EQUAL to 0 */
  fclose(file);
  
  if (!(file=fopen(filename, "w")))
    E_FATAL ("fopen(%s,w) failed\n", file);
  
  if (lm_n_ug(lm)<=0)
    E_FATAL("ngram1=%d",lm_n_ug(lm));
  

  /* Start state */
	
  for (i=0; i<lm_n_ug(lm); i++) {
    if (i%1000==0) fprintf(stderr, ".");
    
    if (i==lm->finishlwid) {
	fprintf(file, "%d\t%d\t%d\t%f\n", st_ug, st_end, lm->finishlwid+1, -lm->ug[i].prob.f); 
    }
    else {	
      if (i!=lm->startlwid) {
	fprintf(file, "%d\t%d\t%d\t%f\n", st_ug, i, i+1, -lm->ug[i].prob.f); /* 1g->2g */
      }
      fprintf(file, "%d\t%d\t0\t%f\n", i, st_ug, -lm->ug[i].bowt.f); /* 2g->1g */
      nb_bg=lm_bglist(lm, i, &bgptr, &bowt); /* bowt not used ... */

      for (j=0; j<nb_bg; j++) {
	if (bgptr[j].wid!=lm->finishlwid) { 
	  id_hist1=idx_tg_hist (lm, bgptr, i, j);
	  nb_tg= lm_tglist (lm, i, bgptr[j].wid, &tgptr, &bowt);
	  if (nb_tg>0) {
	    fprintf(file, "%d\t%d\t%d\t%f\n", i, id_hist1+nbwtg, bgptr[j].wid+1, -lm->bgprob[bgptr[j].probid].f ); /* 2g->3g */
	  }
	  fprintf(file, "%d\t%d\t0\t%f\n", id_hist1+nbwtg, bgptr[j].wid, -lm->tgbowt[bgptr[j].bowtid].f); /* 3g->2g */
	    
	  for (k=0; k<nb_tg; k++) { /* 3g->3g */
	    if (tgptr[k].wid==lm->finishlwid) {
	      fprintf(file, "%d\t%d\t%d\t%f\n", id_hist1+nbwtg, st_end, tgptr[k].wid+1, -lm->tgprob[tgptr[k].probid].f );
	    }
	    else {
	      nb_bg2=lm_bglist(lm, bgptr[j].wid, &bgptr2, &bowt); /* bowt not used ... */						
	      l=find_bg(bgptr2, nb_bg2, tgptr[k].wid);
	      if (l>-1) {
		if (tgptr[k].wid!=lm->finishlwid) {
		  id_hist2=idx_tg_hist (lm, bgptr2, bgptr[j].wid, l);
		  if (prune_lowprobtg) {
		    if (lm->tgprob[tgptr[k].probid].f > (lm->tgbowt[bgptr[j].bowtid].f + lm_bg_score (lm, bgptr[j].wid, tgptr[k].wid, 0)) ) {
		      fprintf(file, "%d\t%d\t%d\t%f\n", id_hist1+nbwtg, id_hist2+nbwtg, tgptr[k].wid+1, -lm->tgprob[tgptr[k].probid].f );
		    }
		  }
		  else {
		    fprintf(file, "%d\t%d\t%d\t%f\n", id_hist1+nbwtg, id_hist2+nbwtg, tgptr[k].wid+1, -lm->tgprob[tgptr[k].probid].f );
		  }
		}
	      }
	    }
	  }
	}
	else {
	  fprintf(file, "%d\t%d\t%d\t%f\n", i, st_end, bgptr[j].wid+1, -lm->bgprob[bgptr[j].probid].f);  /* 2g->2g*/
	}
      }
    }
  }

  /* End state */
  fprintf(file, "%d\t0\n", st_end); 
  
  fclose(file);
  fprintf(stderr, "\nFSM written\n\n");
  return LM_SUCCESS;
}
