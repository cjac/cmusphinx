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
 * stat.c -- statistics of the searching process, including timers and counters. 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.2  2006/02/22  20:01:06  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: 1, Take care of the situation when the hmm_eval =0 (but ugly). 2, Add a free function for stat_t.
 * 
 * Revision 1.1.4.3  2005/11/17 06:12:18  arthchan2003
 * Take care of situation where some metrics are recorded to be zero.
 *
 * Revision 1.1.4.2  2005/09/25 19:05:00  arthchan2003
 * Clarify statistics report in corpus and utterance level of code.
 *
 * Revision 1.1.4.1  2005/07/03 22:56:51  arthchan2003
 * Add stat_free.
 *
 * Revision 1.1  2005/06/21 20:58:09  arthchan2003
 * Add a statistics inventory structure, it takes care of 1, counters, 2, timers. Interfaces are provided to allow convenient clearing and updating of structures
 *
 * Revision 1.6  2005/04/25 19:22:47  archan
 * Refactor out the code of rescoring from lexical tree. Potentially we want to turn off the rescoring if we need.
 *
 * Revision 1.5  2005/04/20 03:44:10  archan
 * Create functions for clear/update/report statistics.  It wraps up code which was slightly spaghatti-like in the past
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 24-Mar-2004 Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             start
 */

#include "stat.h"

/** \file stat.c
 * \brief The wrapper structure for all statistics in sphinx 3.x
 */

stat_t* stat_init(){
  
  stat_t *st;
  st=(stat_t*)ckd_calloc(1,sizeof(stat_t));
  ptmr_init (&(st->tm_sen));
  ptmr_init (&(st->tm_srch));
  ptmr_init (&(st->tm_ovrhd));

  stat_clear_corpus(st);
  stat_clear_utt(st);

  return st;

}

void stat_free(stat_t* st)
{
  if(st){
    ckd_free((void*) st);
  }
}

void stat_clear_utt(stat_t* st)
{
  st->nfr =0 ;
  st->utt_hmm_eval =0; 
  st->utt_sen_eval =0; 
  st->utt_gau_eval =0; 
  st->utt_cisen_eval =0;
  st->utt_cigau_eval =0;
  st->utt_wd_exit = 0 ;
}

void stat_clear_corpus(stat_t* st)
{
  st->tot_fr = 0;
  st->tot_sen_eval = 0.0;
  st->tot_ci_sen_eval = 0.0;
  st->tot_gau_eval = 0.0;
  st->tot_ci_gau_eval = 0.0;
  st->tot_hmm_eval = 0.0;
  st->tot_wd_exit = 0.0;
}

void stat_update_corpus(stat_t* st)
{
  st->tot_sen_eval += st->utt_sen_eval;
  st->tot_gau_eval += st->utt_gau_eval;
  st->tot_ci_sen_eval += st->utt_cisen_eval;
  st->tot_ci_gau_eval += st->utt_cigau_eval;
  st->tot_hmm_eval += st->utt_hmm_eval;
  st->tot_wd_exit += st->utt_wd_exit;
}


void stat_report_utt(stat_t* st,char * uttid){    
  /** do not print anything if nfr is 0 */
  if (st->nfr > 0) {
    /** Before mode 3 could measure hmm/fr and word/fr correctly 
	Don't report it when it is too small to be true. 
     */
    if(	   (st->utt_hmm_eval + (st->nfr >> 1)) / st->nfr != 0){

      E_INFO("%4d frm;  %4d cdsen/fr, %4d cisen/fr, %5d cdgau/fr, %5d cigau/fr, Sen %4.2f, CPU %4.2f "
	   "Clk [Ovrhd %4.2f CPU %4.2f Clk];  "
	   "%5d hmm/fr, %3d wd/fr, Search: %4.2f CPU %4.2f Clk (%s)  \n",
	   st->nfr,
	   (st->utt_sen_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_cisen_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_gau_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_cigau_eval + (st->nfr >> 1)) / st->nfr,
	   st->tm_sen.t_cpu * 100.0 / st->nfr,
	   st->tm_sen.t_elapsed * 100.0 / st->nfr,
	   st->tm_ovrhd.t_cpu * 100.0 / st->nfr,
	   st->tm_ovrhd.t_elapsed * 100.0 / st->nfr,
	   (st->utt_hmm_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_wd_exit + (st->nfr >> 1)) / st->nfr,
	   st->tm_srch.t_cpu * 100.0 / st->nfr,
	   st->tm_srch.t_elapsed * 100.0 / st->nfr,
	   uttid);
    }else{
      E_INFO("%4d frm;  %4d cdsen/fr, %4d cisen/fr, %5d cdgau/fr, %5d cigau/fr, Sen %4.2f, CPU %4.2f "
	   "Clk [Ovrhd %4.2f CPU %4.2f Clk];  "
	   "Search: %4.2f CPU %4.2f Clk (%s)  \n",
	   st->nfr,
	   (st->utt_sen_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_cisen_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_gau_eval + (st->nfr >> 1)) / st->nfr,
	   (st->utt_cigau_eval + (st->nfr >> 1)) / st->nfr,
	   st->tm_sen.t_cpu * 100.0 / st->nfr,
	   st->tm_sen.t_elapsed * 100.0 / st->nfr,
	   st->tm_ovrhd.t_cpu * 100.0 / st->nfr,
	   st->tm_ovrhd.t_elapsed * 100.0 / st->nfr,
	   st->tm_srch.t_cpu * 100.0 / st->nfr,
	   st->tm_srch.t_elapsed * 100.0 / st->nfr,
	   uttid);
    }
  }else{
    assert(st->tot_fr > 0);
    E_INFO("%4d frm , No report\n",0);
  }

}



void stat_report_corpus(stat_t * st){

  if (st->tot_fr != 0){

    E_INFO("SUMMARY:  %d fr;  %d cdsen/fr, %d cisen/fr, %d cdgau/fr, %d cigau/fr, %.2f xCPU %.2f xClk [Ovhrd %.2f xCPU %2.f xClk];  %d hmm/fr, %d wd/fr, %.2f xCPU %.2f xClk;  tot: %.2f xCPU, %.2f xClk\n",
	   st->tot_fr,
	   (int32)(st->tot_sen_eval / st->tot_fr),
	   (int32)(st->tot_ci_sen_eval / st->tot_fr),
	   (int32)(st->tot_gau_eval / st->tot_fr),
	   (int32)(st->tot_ci_gau_eval / st->tot_fr),
	   st->tm_sen.t_tot_cpu * 100.0 / st->tot_fr,
	   st->tm_sen.t_tot_elapsed * 100.0 / st->tot_fr,
	   st->tm_ovrhd.t_tot_cpu * 100.0 / st->tot_fr,
	   st->tm_ovrhd.t_tot_elapsed * 100.0 / st->tot_fr,
	   (int32)(st->tot_hmm_eval / st->tot_fr),
	   (int32)(st->tot_wd_exit / st->tot_fr),
	   st->tm_srch.t_tot_cpu * 100.0 / st->tot_fr,
	   st->tm_srch.t_tot_elapsed * 100.0 / st->tot_fr,
	   st->tm.t_tot_cpu * 100.0 / st->tot_fr,
	   st->tm.t_tot_elapsed * 100.0 / st->tot_fr);
  }else{
    assert(st->tot_fr == 0);
    E_INFO("SUMMARY:  0 fr , No report\n");
  }

}


