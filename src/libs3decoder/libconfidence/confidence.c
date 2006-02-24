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
/* confidence.c: Calculate confidence scores from word lattices using
 * posterior word probabilities and backoff.
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * 
 * Author: Rong Zhang <rongz@cs.cmu.edu>
 *
 * Rong has followed extensively the paper written by Frank Wessel,
 * Klaus Machery and Ralk Schluter, "Using word probabilities as
 * confidence measures". Proc of ICASSP 1998
 *
 * Arthur Chan has significantly changed these routines when
 * incorporating to Sphinx 3.x
 *
 * $Log$
 * Revision 1.3  2006/02/24  13:40:50  arthchan2003
 * Commented overlap currently.
 * 
 * Revision 1.2  2006/02/21 18:31:09  arthchan2003
 * Merge confidence.c confidence.h and Makefile.am into the trunk.
 *
 * Revision 1.1.2.1  2006/01/16 18:38:25  arthchan2003
 * Adding Rong's confidence routine.  Compare to Rong's routine, this routine used Sphinx3's njmerical routines and make the chances of backward-forward scores inconsistency to be lower.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <bio.h>
#include <ckd_alloc.h>

#include "confidence.h"
#include "srch_output.h"

/**
   Function that give ceiling and floor to additions 
 */
static int int_add(int i, int j)
{
  int k;

  if ((i > 0) && (j > 0)) {
    k = i + j;
    if (k <= 0)
      k = CONFIDENCE_MAX_INT;
    return k;
  }
	
  if ((i < 0) && (j < 0)) {
    k = i + j;
    if (k >= 0)
      k = CONFIDENCE_MIN_INT;
    return k;
  }
  
  k = i + j;
  return k;
}

#if 0
static int overlap(int a, int b, int x, int y, int *start, int *end)
{
  if ((b < x) || (a > y))
    return -1;
  if (a >= x)
    *start = a;
  else
    *start = x;
  if (b >= y)
    *end = y;
  else
    *end = b;
  
  if ((*end - *start) * 3 >= (y - x))
    return 0;
  else
    return -1;
}
#endif

/**
  Delete filler words
 */
static int delete_filler(ca_dag *word_lattice,dict_t *dict, fillpen_t *fillpen)
{
  ca_dagnode *d, *nd, *pd, *dd;
  ca_daglink *l, *ll, *lll;
  int32 score, SilencePenalty, FillerPenalty;
  int32 count;
  s3wid_t SilenceWordId;

  SilencePenalty = fillpen->silprob;
  FillerPenalty = fillpen->fillerprob;
  SilenceWordId = dict_silwid(dict);

  pd = NULL;
  count = 0;
  for (d = word_lattice->nodelist; d; d = nd) {
    nd = d->alloc_next;
#if 1
    if (! dict_filler_word(dict,d->wid)) {
#else
    if (d->wid >=0) {
#endif
      pd = d;
      continue;
    }

    /*    E_INFO("Delete word: %s\n",d->word);*/
    count++;
    for (l = d->predlist; l; l = l->next) {
      dd = l->to;
      lll = NULL;
      for (ll = dd->succlist; ll; ll = ll->next) {
	if (ll->to == d)
	  break;
	lll = ll;
      }
      if (ll == NULL) {
	E_WARN("delete filler: can't find succ node\n");
	return CONFIDENCE_FAILURE;
      }
      if (ll == dd->succlist)
	dd->succlist = ll->next;
      else
	lll->next = ll->next;
      score = ll->ascore;
      if (d->wid == SilenceWordId)
	score = int_add(SilencePenalty, score);
      else
	score = int_add(FillerPenalty, score);
      free(ll);
      dd->fanout--;
      
      for (ll = d->succlist; ll; ll = ll->next) {
	for (lll = dd->succlist; lll; lll = lll->next) 
	  if (lll->to == ll->to)
	    break;
	if (lll) {
	  lll->ascore = logs3_add(int_add(score, ll->ascore), lll->ascore);
	  continue;
	}
	lll = (ca_daglink *) malloc(sizeof(ca_daglink));
	if (lll == NULL) {
	  E_WARN("unable alloc memoery\n");
	  exit (1);
	}
	lll->from = dd;
	lll->to = ll->to;
	lll->ascore = int_add(score, ll->ascore);
	lll->alpha_beta = 0;
	lll->next = dd->succlist;
	dd->succlist = lll;
	dd->fanout++;
      }
      if (d->lef > dd->lef) 
	dd->lef = d->lef;
    }
    
    for (l = d->succlist; l; l = l->next) {
      dd = l->to;
      lll = NULL;
      for (ll = dd->predlist; ll; ll = ll->next) {
	if (ll->to == d)
	  break;
	lll = ll;
      }
      if (ll == NULL) {
	E_WARN("delete filler: can't find pred node\n");
	return -1;
      }
      if (ll == dd->predlist)
	dd->predlist = ll->next;
      else
	lll->next = ll->next;
      score = ll->ascore;
      if (d->wid == SilenceWordId)
	score = int_add(SilencePenalty, score);
      else
	score = int_add(FillerPenalty, score);
      free(ll);
      dd->fanin--;
      
      for (ll = d->predlist; ll; ll = ll->next) {
	for (lll = dd->predlist; lll; lll = lll->next) 
	  if (lll->to == ll->to)
	    break;
	if (lll) {
	  lll->ascore = logs3_add(int_add(score, ll->ascore), lll->ascore);
	  continue;
	}
	lll = (ca_daglink *) malloc(sizeof(ca_daglink));
	if (lll == NULL) {
	  E_WARN("unable alloc memoery\n");
	  exit (1);
	}
	lll->from = dd;
	lll->to = ll->to;
	lll->ascore = int_add(score, ll->ascore);
	lll->alpha_beta = 0;
	lll->next = dd->predlist;
	dd->predlist = lll;
	dd->fanin++;
      }
    }
    
    for (l = d->predlist; l; l = ll) {
      ll = l->next;
      free(l);
    }
    for (l = d->succlist; l; l = ll) {
      ll = l->next;
      free(l);
    }
    if (pd)
      pd->alloc_next = nd;
    else
      word_lattice->nodelist = nd;
    word_lattice->seqidtonode[d->seqid] = NULL;
    free(d);
  }
  E_INFO("%d filler words deleted\n", count);
  return 0;
}

static void pre_check_lattice(ca_dag *word_lattice)
{
  int nnode, npredlink, nsucclink, n;
  ca_dagnode *d;
  ca_daglink *l;

  nnode = 0;
  for (d = word_lattice->nodelist; d; d = d->alloc_next)
    nnode++;
  if (nnode != word_lattice->nnode) 
    E_FATAL("node number should be %d, but the real number is %d\n", word_lattice->nnode, nnode);

  E_INFO("node number: %d\n", nnode);
  
  npredlink = nsucclink = 0;
  for (d = word_lattice->nodelist; d; d = d->alloc_next) {
    n = 0;
    for (l = d->predlist; l; l = l->next)
      n++;
    if (n > nnode) 
      E_FATAL("too many preceeding links\n");

    if (n != d->fanin) 
      E_FATAL("error: the real number of preceeding links is not equal to fanin\n");

    npredlink += n;
    n = 0;
    for (l = d->succlist; l; l = l->next)
      n++;
    if (n > nnode) 
      E_FATAL("too many succeeding links\n");

    if (n != d->fanout) 
      E_FATAL(" the real number of succeeding links is not equal to fanout\n");

    nsucclink += n;
  }

  if (npredlink != word_lattice->nedge) 
    E_FATAL("preceeding link number should be %d, but the real number is %d\n", word_lattice->nedge, npredlink);

  if (nsucclink != word_lattice->nedge) {
    E_FATAL("succeeding link number should be %d, but the real number is %d\n", word_lattice->nedge, nsucclink);
  }
  E_INFO("pred link number: %d   succ link number: %d\n", npredlink, nsucclink);
}


void post_check_lattice(ca_dag *word_lattice,dict_t *dict)
{
  int nnode, npredlink, nsucclink, n;
  ca_dagnode *d, *tod;
  ca_daglink *l, *ll;
  int fwdscore, bwdscore;
  
  nnode = 0;
  for (d = word_lattice->nodelist; d; d = d->alloc_next)
    nnode++;
  if (nnode > word_lattice->nnode) {
    E_WARN("the node number after pruning is greater than that before pruning\n");
    exit(1);
  }
  E_INFO("node number after pruning: %d\n", nnode);
  
  npredlink = nsucclink = 0;
  for (d = word_lattice->nodelist; d; d = d->alloc_next) {
    n = 0;
    for (l = d->predlist; l; l = l->next)
      n++;
    if (n > nnode) {
      E_WARN("error: too many preceeding links\n");
      exit(1);
    }
    if (n != d->fanin) {
      E_WARN("error: the real number of preceeding links is not equal to fanin\n");
      exit(1);
    }
    npredlink += n;
    n = 0;
    for (l = d->succlist; l; l = l->next)
      n++;
    if (n > nnode) {
      E_WARN("error: too many succeeding links\n");
      exit(1);
    }
    if (n != d->fanout) {
      E_WARN("error: the real number of succeeding links is not equal to fanout\n");
      exit(1);
    }
    nsucclink += n;
    
    if (d != word_lattice->exit.to) {
      fwdscore = CONFIDENCE_MIN_INT;
      for (l = d->succlist; l; l = l->next)
	fwdscore = logs3_add(fwdscore, l->ascore); //int_add(fwdscore, l->ascore);
      n = 0;
      bwdscore = CONFIDENCE_MIN_INT;
      for (l = d->succlist; l; l = l->next) {
	tod = l->to;
	for (ll = tod->predlist; ll; ll = ll->next) {
	  if (ll->to == d) {
	    n++;
	    bwdscore = logs3_add(bwdscore, ll->ascore); //int_add(bwdscore, ll->ascore);
	  }
	}
      }
      if (n != d->fanout) {
	E_WARN("error: the number of predlist from other nodes is not equal to fanout\n");
	exit(1);
      }

      /*      E_INFO("word %s, d->wid %d, d->seqid %d, fwdscore %d, bwdscore %d\n", dict_wordstr(dict,d->wid), d->wid, d->seqid, fwdscore, bwdscore);*/
      if (fwdscore != bwdscore) {
	E_WARN("error: the score of predlist from other nodes is not equal to the score of succlist (fwdscore %d, bwdscore %d)\n", fwdscore, bwdscore);
	exit(1);
      }
    }
    
    if (d != word_lattice->entry.to) {
      bwdscore = CONFIDENCE_MIN_INT;
      for (l = d->predlist; l; l = l->next) {
	/*	E_INFO("bwdscore %d, l->ascore %d\n", bwdscore, l->ascore);*/
	bwdscore = logs3_add(bwdscore, l->ascore); //int_add(bwdscore, l->ascore);
      }
      n = 0;
      fwdscore = CONFIDENCE_MIN_INT;
      for (l = d->predlist; l; l = l->next) {
	tod = l->to;
	for (ll = tod->succlist; ll; ll = ll->next) {
	  if (ll->to == d) {
	    n++;
	    /*	    E_INFO("fwdscore %d, ll->ascore %d\n", fwdscore, ll->ascore);*/

	    fwdscore = logs3_add(fwdscore, ll->ascore); //int_add(fwdscore, ll->ascore);
	  }
	}
      }
      if (n != d->fanin) {
	E_WARN("error: the number of succlist from other nodes is not equal to fanin\n");
	exit(1);
      }

      /*      E_INFO("word %s, d->wid %d, d->seqid %d, fwdscore %d, bwdscore %d\n", dict_wordstr(dict,d->wid), d->wid, d->seqid, fwdscore, bwdscore);*/
      if (fwdscore != bwdscore) {
	E_WARN("error: the score of succlist from other nodes is not equal to the score of predlist (fwdscore %d, bwdscore %d)\n", fwdscore, bwdscore);
	exit(1);
      }
    }
    
  }
  
  if (npredlink != nsucclink) {
    E_WARN("after pruning the preceeding link number is unequal to the succeeding link number: %d %d\n", npredlink, nsucclink);
    exit(1);
  }
  E_INFO("pred link number after pruning: %d   succ link number after pruning: %d\n", npredlink, nsucclink);
  
}


static void mark_backward_reachable(ca_dagnode *d)
{
  ca_daglink *l;
  
  d->reachable = 1;
  for (l = d->predlist; l; l = l->next)
    if (!l->to->reachable)
      mark_backward_reachable (l->to);
}


static void mark_forward_reachable(ca_dagnode *d)
{
  ca_daglink *l;
  
  d->reachable = 1;
  for (l = d->succlist; l; l = l->next)
    if (!l->to->reachable)
      mark_forward_reachable (l->to);
}


static void delete_unreachable(ca_dag *word_lattice)
{
  ca_dagnode *d, *pd, *nd, *tod;
  ca_daglink *l, *nl, *ll, *pll, *nll;
	int count;
  
	count = 0;
  pd = NULL;
  for (d = word_lattice->nodelist; d; d = nd) {
    nd = d->alloc_next;
    if (!d->reachable) {
      if (pd)
	pd->alloc_next = nd;
      else
	word_lattice->nodelist = nd;
      d->alloc_next = NULL;
      for (l = d->succlist; l; l = nl) {
	nl = l->next;
	tod = l->to;
	pll = NULL;
	for (ll = tod->predlist; ll; ll = nll) {
	  nll = ll->next;
	  if (ll->to == d) {
	    if (pll)
	      pll->next = nll;
	    else
	      tod->predlist = nll;
	    tod->fanin--;
	    free(ll);
	  }
	  else
	    pll = ll;
	}
	free (l);
      }
      d->succlist = NULL;
      for (l = d->predlist; l; l = nl) {
	nl = l->next;
	tod = l->to;
	pll = NULL;
	for (ll = tod->succlist; ll; ll = nll) {
	  nll = ll->next;
	  if (ll->to == d) {
	    if (pll)
	      pll->next = nll;
	    else
	      tod->succlist = nll;
	    tod->fanout--;
	    free(ll);
	  }
	  else
	    pll = ll;
	}
	free (l);
      }
      d->predlist = NULL;
      word_lattice->seqidtonode[d->seqid] = NULL;
      free(d);
	  count++;
    }
    else
      pd = d;
  }
  E_INFO("%d unreachable nodes deleted\n", count);
}

static int load_node(FILE *fp, ca_dag *word_lattice, lm_t* lm, dict_t *dict )
{
  ca_dagnode *d, *tail;
  char line[2048], word[64];
  int i, j, seqid;
  int sf, fef, lef;
  int wid;

  word_lattice->seqidtonode = (ca_dagnode **) calloc(word_lattice->nnode, sizeof(ca_dagnode *));
  if (word_lattice->seqidtonode == NULL) {
    E_WARN("unable to alloc memory\n");
    exit (1);
  }
  tail = NULL;
  for (i = 0; i < word_lattice->nnode; i++) {
    if (fgets(line, sizeof(line), fp) == NULL) {
      E_WARN("fgets(..) failed\n");
      return CONFIDENCE_FAILURE;
    }
    if (sscanf(line, "%d %s %d %d %d", &seqid, word, &sf, &fef, &lef) != 5) {
      E_WARN("nodes format invalid\n");
      return CONFIDENCE_FAILURE;
    }
    if (seqid != i) {
      E_WARN("seqid invalid: %s %d\n", line, i);
      return CONFIDENCE_FAILURE;
    }

    d = (ca_dagnode *) malloc(sizeof(ca_dagnode));
    if (d == NULL) {
      E_WARN("unable alloc memoery\n");
      exit (1);
    }
    strcpy(d->word, word);

    for (j = strlen(word) - 1; j >= 0; j--)
      if (word[j] == '(')
	break;

    if (j >= 0)
      word[j] = '\0';

    wid=dict_wordid(dict,word);
    if(wid==BAD_S3WID){
      E_FATAL("Found Bad Word (not in dictionary) in load_nodes %d %s\n",wid,word);
    }

#if 0    
    if(dict_filler_word(dict,wid)){
      E_FATAL("Word %S is not a filler word and also not a dictionary word\n",word);
    }
#endif


    d->wid = wid;
    d->seqid = seqid;
    d->sf = sf;
    d->fef = fef;
    d->lef = lef;
    d->reachable = 0;
    d->visited = 0;
    d->fanin = 0;
    d->fanout = 0;
    d->hscore = 0;
    d->pscore = 0;
    d->lscore = 0;
    d->cscore = 0;
    d->succlist = NULL;
    d->predlist = NULL;
    d->alloc_next = NULL;
    
    word_lattice->seqidtonode[i] = d;
    if (word_lattice->nodelist == NULL)
      word_lattice->nodelist = d;
    else
      tail->alloc_next = d;
    tail = d;
  }
  return 0;
}


static int load_edge(FILE *fp, ca_dag *word_lattice, lm_t *lm,dict_t *dict)
{
  ca_dagnode *pd, *d;
  ca_daglink *l, *tail;
  char line[1024];
  int nedge, from, to, ascore, endid;

  if ((endid = dict_finishwid(dict) < 0)) 
    E_FATAL("can not find </s> in dictionary\n");

  tail = NULL;
  nedge = 0;
  while (fgets (line, 1024, fp) != NULL) {
    if (sscanf (line, "%d %d %d", &from, &to, &ascore) != 3) {
      break;
    }
    pd = word_lattice->seqidtonode[from];
    if (pd->wid == endid)
      continue;
    d = word_lattice->seqidtonode[to];
    if ((pd == word_lattice->entry.to) || (d == word_lattice->exit.to) ||
	((d->lef - d->fef >= 0) && (pd->lef - pd->fef >= 0))) {
      l = (ca_daglink *) malloc(sizeof(ca_daglink));
      if (l == NULL) {
	E_FATAL("Unable to allocate memory\n");
      }
      l->from = pd;
      l->to = d;
      l->ascore = ascore;
      l->alpha_beta = 0;
      l->next = pd->succlist;
      pd->succlist = l;
      l = (ca_daglink *) malloc(sizeof(ca_daglink));
      if (l == NULL) {
	E_WARN("unable alloc memoery\n");
	exit (1);
      }
      l->from = d;
      l->to = pd;
      l->ascore = ascore;
      l->alpha_beta = 0;
      l->next = d->predlist;
      d->predlist = l;
      pd->fanout++;
      d->fanin++;
      nedge++;
    }
  }
  word_lattice->nedge = nedge;
  return 0;
}


static ca_dagnode **sort_lattice_forward(ca_dag *word_lattice, int *nnode)
{
  ca_dagnode **list, *d;
  ca_daglink *l;
  int i, from;
  
  *nnode = 0;
  for (d = word_lattice->nodelist; d; d = d->alloc_next)
    (*nnode)++;
  list = (ca_dagnode **) calloc(*nnode, sizeof(ca_dagnode *));
  if (list == NULL) {
    E_WARN("unable to alloc memory\n");
    exit(1);
  }
  for (i = *nnode - 1, d = word_lattice->nodelist; i >= 0 && d; i--, d = d->alloc_next) {
    d->visited = 0;
    list[i] = d;
  }
  if (*nnode == 0)
    return list;
  
  list[0]->visited = 1;
  from = 1;
  while (from < *nnode - 1) {
    for (i = from; i < *nnode; i++) {
      for (l = list[i]->predlist; l; l = l->next)
	if (l->to->visited == 0)
	  break;
      if (l) 
	break;
      else
	list[i]->visited = 1;
    }
    if (i == *nnode)
      break;
    from = i;
    for (i = from + 1; i < *nnode; i++) {
      for (l = list[i]->predlist; l; l = l->next)
	if (l->to->visited == 0) {
	  break;
	}
      if (!l)
	break;
    }
    if (i == *nnode) {
      E_WARN("failed to forward sort lattice\n");
      return NULL;
    }
    d = list[from];
    list[from] = list[i];
    list[i] = d;
    list[from]->visited = 1;
    from++;
  }
  return list;
}

static ca_dagnode **sort_lattice_backward(ca_dag *word_lattice, int *nnode)
{
  ca_dagnode **list, *d;
  ca_daglink *l;
  int i, from;
  
  *nnode = 0;
  for (d = word_lattice->nodelist; d; d = d->alloc_next)
    (*nnode)++;
  list = (ca_dagnode **) calloc(*nnode, sizeof(ca_dagnode *));
  if (list == NULL) {
    E_WARN("unable to alloc memory\n");
    exit(1);
  }
  for (i = 0, d = word_lattice->nodelist; i < *nnode && d; i++, d = d->alloc_next) {
    d->visited = 0;
    list[i] = d;
  }
  if (*nnode == 0)
    return list;
  
  list[0]->visited = 1;
  from = 1;
  while (from < *nnode - 1) {
    for (i = from; i < *nnode; i++) {
      for (l = list[i]->succlist; l; l = l->next)
	if (l->to->visited == 0)
	  break;
      if (l) 
	break;
      else
	list[i]->visited = 1;
    }
    if (i == *nnode)
      break;
    from = i;
    for (i = from + 1; i < *nnode; i++) {
      for (l = list[i]->succlist; l; l = l->next)
	if (l->to->visited == 0) {
	  break;
	}
      if (!l)
	break;
    }
    if (i == *nnode) {
      E_WARN("failed to forward sort lattice\n");
      return NULL;
    }
    d = list[from];
    list[from] = list[i];
    list[i] = d;
    list[from]->visited = 1;
    from++;
  }
  return list;
}

static void heuristic_score(ca_dag *word_lattice)
{
  ca_dagnode **dlist, *d;
  ca_daglink *l;
  int i, nnode, bestscore;
  
  dlist = sort_lattice_backward(word_lattice, &nnode);
  for (i = 0; i < nnode; i++) {
    d = dlist[i];
    if (d->succlist == NULL)
      d->hscore = 0;
    else {
      bestscore = WORST_CONFIDENCE_SCORE;
      for (l = d->succlist; l; l = l->next) {
	if ((l->ascore + l->to->hscore) > bestscore)
	  bestscore = l->ascore + l->to->hscore;
      }
      d->hscore = bestscore;
    }
  }
  free(dlist);
}


int ca_dag_load_lattice(char *filename, ca_dag *word_lattice, lm_t *lm, dict_t *dict, fillpen_t *fpen)
{
  FILE *fp;
  char line[1024], word[64];
  int n;
  ca_dagnode *p;
  int32 ispipe;

  word_lattice->nodelist = NULL;
  word_lattice->seqidtonode = NULL;
  word_lattice->entry.to = NULL;
  word_lattice->exit.to = NULL;
  word_lattice->nfrm = -1;
  word_lattice->nnode = -1;
  word_lattice->nedge = -1;

  printf("reading word lattice file: %s\n", filename);
  if ((fp = fopen_compchk(filename, &ispipe)) == NULL) {
    printf("fopen_compchk(%s) failed\n", filename);
    return CONFIDENCE_FAILURE;
  }

  while (fgets (line, 1024, fp) != NULL) {
    if (line[0] == '#')
      continue;
    if (sscanf(line, "%s", word) != 1)
      continue;
    if (!strcmp(word, "Frames")) {
      if ((sscanf(line, "%s %d", word, &n) != 2) || (n <= 0)) {
	printf("frames parameter missing or invalid\n");
	return CONFIDENCE_FAILURE;
      }
      word_lattice->nfrm = n;
    }
    else if (!strcmp(word, "Nodes")) {
      if ((sscanf(line, "%s %d", word, &n) != 2) || (n <= 0)) {
	printf("nodes parameter missing or invalid\n");
	return CONFIDENCE_FAILURE;
      }
      word_lattice->nnode = n;
      if (load_node(fp, word_lattice,lm,dict) == -1) {
	printf("failed to load node\n");
	return CONFIDENCE_FAILURE;
      }
    }
    else if (!strcmp(word, "Initial")) {
      if ((sscanf(line, "%s %d", word, &n) != 2) || (n < 0) || (n >= word_lattice->nnode)) {
	printf("initial node parameter missing or invalid\n");
	return CONFIDENCE_FAILURE;
      }
      for (p = word_lattice->nodelist; p; p = p->alloc_next) {
	if (p->seqid == n)
	  break;
      }
      if (!p) {
	printf("initial node not found\n");
	return CONFIDENCE_FAILURE;
      }
      word_lattice->entry.to = p;
      word_lattice->entry.ascore = 0;
      word_lattice->entry.from = NULL;
      word_lattice->entry.next = NULL;
    }
    else if (!strcmp(word, "Final")) {
      if ((sscanf(line, "%s %d", word, &n) != 2) || (n < 0) || (n >= word_lattice->nnode)) {
	printf("final node parameter missing or invalid\n");
	return CONFIDENCE_FAILURE;
      }
      for (p = word_lattice->nodelist; p; p = p->alloc_next) {
	if (p->seqid == n)
	  break;
      }
      if (!p) {
	printf("final node not found\n");
	return CONFIDENCE_FAILURE;
      }
      word_lattice->exit.to = p;
      word_lattice->exit.ascore = 0;
      word_lattice->exit.from = NULL;
      word_lattice->exit.next = NULL;
    }
    else if (!strcmp(word, "BestSegAscr"))
      continue;
    else if (!strcmp(word, "Edges")) {
      if (load_edge(fp, word_lattice,lm,dict) == -1) {
	printf("failed to load link\n");
	return CONFIDENCE_FAILURE;
      }
    }
    continue;
  }

  if ((word_lattice->nodelist == NULL) || (word_lattice->seqidtonode == NULL) ||
      (word_lattice->entry.to == NULL) || (word_lattice->exit.to == NULL) || 
      (word_lattice->nfrm <= 0) ||	(word_lattice->nnode <= 0) || (word_lattice->nedge <= 0)) {
    printf("failed to load lattice\n");
    return CONFIDENCE_FAILURE;
  }

  fclose_comp(fp, ispipe);
	
  pre_check_lattice(word_lattice);
  delete_filler(word_lattice, dict,fpen);

  for (p = word_lattice->nodelist; p; p = p->alloc_next)
    p->reachable = 0;
  mark_backward_reachable(word_lattice->exit.to);
  delete_unreachable(word_lattice);

  for (p = word_lattice->nodelist; p; p = p->alloc_next)
    p->reachable = 0;
  mark_forward_reachable(word_lattice->entry.to);
  delete_unreachable(word_lattice);

  heuristic_score(word_lattice);
  post_check_lattice(word_lattice,dict);
  return CONFIDENCE_SUCCESS;
}


int ca_dag_free_lattice(ca_dag *word_lattice)
{
  ca_dagnode *d, *nd;
  ca_daglink *l, *nl;
  
  for (d = word_lattice->nodelist; d; d = nd) {
    nd = d->alloc_next;
    for (l = d->succlist; l; l = nl) {
      nl = l->next;
      free(l);
    }
    for (l = d->predlist; l; l = nl) {
      nl = l->next;
      free(l);
    }
    free(d);
  }
  word_lattice->nodelist = NULL;
  free(word_lattice->seqidtonode);
  word_lattice->seqidtonode = NULL;
  word_lattice->entry.to = NULL;
  word_lattice->exit.to = NULL;
  word_lattice->nfrm = -1;
  word_lattice->nnode = -1;
  word_lattice->nedge = -1;

  return CONFIDENCE_SUCCESS;
}

int ca_lm_ug_score(lm_t* lm, dict_t* dict, int wid1)
{

  int32 r_lscr;
  /*  E_INFO("%d\n",		     lm_bg_score(lm,
						 lm->dict2lmwid[dict_basewid(dict,wid1)],
						 lm->dict2lmwid[dict_basewid(dict,wid2)],
						 wid2));*/

  r_lscr=lm_rawscore(lm, 
		     lm_ug_score(lm, 
				 lm->dict2lmwid[dict_basewid(dict,wid1)],
				 wid1)
		     );

  /*  E_INFO("wid1 %d, wid2 %d r_lscr %d %f\n",wid1, wid2,r_lscr, logs3_to_log10(r_lscr));  */
  return (int)((double) logs3_to_log10(r_lscr) * logs3_10base() * MAGIC_CONFIDENCE_CONSTANT / lm->lw);

}

/*
  HACK! Rong's magical function for computing bigram scores
 */
int ca_lm_bg_score(lm_t* lm, dict_t* dict, int wid1, int wid2)
{

  int32 r_lscr;
  /*  E_INFO("%d\n",		     lm_bg_score(lm,
						 lm->dict2lmwid[dict_basewid(dict,wid1)],
						 lm->dict2lmwid[dict_basewid(dict,wid2)],
						 wid2));*/

  r_lscr=lm_rawscore(lm, 
		     lm_bg_score(lm, 
				 lm->dict2lmwid[dict_basewid(dict,wid1)],
				 lm->dict2lmwid[dict_basewid(dict,wid2)],
				 wid2)
		     );

  /*  E_INFO("wid1 %d, wid2 %d r_lscr %d %f\n",wid1, wid2,r_lscr, logs3_to_log10(r_lscr));  */
  return (int)((double) logs3_to_log10(r_lscr) * logs3_10base() * MAGIC_CONFIDENCE_CONSTANT/ lm->lw);

}

/*
  HACK! Rong's magical function for computing trigram scores
 */

int ca_lm_tg_score(lm_t* lm, dict_t *dict, int wid1, int wid2, int wid3)
{
  int32 r_lscr;
  /*  E_INFO("%d\n",		     lm_tg_score(lm,
						 lm->dict2lmwid[dict_basewid(dict,wid1)],
						 lm->dict2lmwid[dict_basewid(dict,wid2)],
						 lm->dict2lmwid[dict_basewid(dict,wid3)],
						 wid3));*/
  r_lscr=lm_rawscore(lm, 
		     lm_tg_score(lm,
				 lm->dict2lmwid[dict_basewid(dict,wid1)],
				 lm->dict2lmwid[dict_basewid(dict,wid2)],
				 lm->dict2lmwid[dict_basewid(dict,wid3)],
				 wid3)
		     );

  /*  E_INFO("wid1 %d, wid2 %d wid3 %d r_lscr %d %f\n",wid1, wid2,wid3, r_lscr, logs3_to_log10(r_lscr));*/

  return (int)((double) logs3_to_log10(r_lscr) * logs3_10base() * MAGIC_CONFIDENCE_CONSTANT / lm->lw);
}


#undef ACOUSTIC_ONLY
#undef LM_ONLY
#define BOTH_ACOUSTIC_AND_LM

int alpha_beta(ca_dag *word_lattice, lm_t *lm , dict_t *dict)
{
  ca_dagnode **dlist, *d, *fromd, *tod;
  ca_daglink *l, *ll;
  int i, nnode, sum;

  if ((dlist = sort_lattice_forward(word_lattice, &nnode)) == NULL) {
    printf("alpha_beta: failed to sort lattice\n");
    return CONFIDENCE_FAILURE;
  }

  for (i = 1; i < nnode; i++) {
    d = dlist[i];
    for (l = d->predlist; l; l = l->next) {
      fromd = l->to;
      if (fromd->predlist == NULL) {
	if (fromd != word_lattice->entry.to) {
	  printf("alpha_beta: node is unreachable from <s>\n");
	  return CONFIDENCE_FAILURE;
	}
#ifdef ACOUSTIC_ONLY
	l->alpha_beta = l->ascore;
#elif defined(LM_ONLY)
	l->alpha_beta = ca_lm_bg_score(lm, dict, fromd->wid, d->wid) ; 
#else
	l->alpha_beta = int_add(l->ascore, ca_lm_bg_score(lm, dict, fromd->wid, d->wid));
#endif
      }
      else {
	ll = fromd->predlist;
#ifdef ACOUSTIC_ONLY
	sum = ll->alpha_beta;
#else
	sum = int_add(ll->alpha_beta, ca_lm_tg_score(lm, dict, ll->to->wid, fromd->wid, d->wid));
#endif
	for (ll = fromd->predlist->next; ll; ll = ll->next)
#ifdef ACOUSTIC_ONLY
	  sum = logs3_add(sum, ll->alpha_beta);
#else
	sum = logs3_add(sum, int_add(ll->alpha_beta, ca_lm_tg_score(lm, dict, ll->to->wid, fromd->wid, d->wid)));
#endif
#ifdef LM_ONLY
	l->alpha_beta = sum;
#else
	l->alpha_beta = int_add(sum, l->ascore);
#endif
      }
    }
  }
  free(dlist);
  
  if ((dlist = sort_lattice_backward(word_lattice, &nnode)) == NULL) {
    printf("alpha_beta: failed to sort lattice\n");
    return CONFIDENCE_FAILURE;
  }
  
  for (i = 1; i < nnode; i++) {
    d = dlist[i];
    for (l = d->succlist; l; l = l->next) {
      tod = l->to;
      if (tod->succlist == NULL) {
	if (tod != word_lattice->exit.to) {
	  printf("alpha_beta: node is unreachable from </s>\n");
	  return CONFIDENCE_FAILURE;
	}
#ifdef ACOUSTIC_ONLY
	l->alpha_beta = l->ascore;
#elif defined(LM_ONLY)
	l->alpha_beta = 0;
#else
	l->alpha_beta = l->ascore;
#endif
      }
      else {
	ll = tod->succlist;
#ifdef ACOUSTIC_ONLY
	sum = ll->alpha_beta;
#else
	sum = int_add(ll->alpha_beta, ca_lm_tg_score(lm,dict, d->wid, tod->wid, ll->to->wid));
#endif
	for (ll = tod->succlist->next; ll; ll = ll->next)
#ifdef ACOUSTIC_ONLY
	  sum = logs3_add(sum, ll->alpha_beta);
#else
	sum = logs3_add(sum, int_add(ll->alpha_beta, ca_lm_tg_score(lm,dict, d->wid, tod->wid, ll->to->wid)));
#endif
	if (d == word_lattice->entry.to)
#ifdef ACOUSTIC_ONLY
	  l->alpha_beta = int_add(sum, l->ascore);
#elif defined(LM_ONLY)
	l->alpha_beta = int_add(sum, ca_lm_bg_score(lm,dict, d->wid, tod->wid));
#else
	l->alpha_beta = int_add(sum, int_add(l->ascore, ca_lm_bg_score(lm,dict, d->wid, tod->wid)));
#endif
	else
#ifdef LM_ONLY
	  l->alpha_beta = sum;
#else
	l->alpha_beta = int_add(sum, l->ascore);
#endif
      }
    }
  }
  free(dlist);
  
  for (d = word_lattice->nodelist; d; d = d->alloc_next) {
    if (d == word_lattice->entry.to) {
      if (d->succlist == NULL)
	d->cscore = 0;
      else {
	sum = d->succlist->alpha_beta;
	/*	E_INFO("Sum %d AB %d From %s To %s\n",sum, d->succlist->alpha_beta, d->succlist->from->word, d->succlist->to->word);*/

	for (l = d->succlist->next; l; l = l->next){

	  sum = logs3_add(sum, l->alpha_beta);
	  /*	  E_INFO("Sum %d AB %d From %s To %s\n",sum, l->alpha_beta, l->from->word, l->to->word);*/
	}
	d->cscore = sum;
      }
      continue;
    }
    if (d == word_lattice->exit.to) {
      if (d->predlist == NULL)
	d->cscore = 0;
      else {
	sum = d->predlist->alpha_beta;
	/*	E_INFO("Sum %d AB %d From %s To %s\n",sum, d->predlist->alpha_beta, d->predlist->from->word, d->predlist->to->word);*/

	for (l = d->predlist->next; l; l = l->next){
	  sum = logs3_add(sum, l->alpha_beta);
	  /*	  E_INFO("Sum %d AB %d From %s To %s\n",sum, l->alpha_beta, l->from->word, l->to->word);*/
	}
	d->cscore = sum;
      }
      continue;
    }
    if ((d->predlist == NULL) || (d->succlist == NULL)) {
      printf("alpha_beta: node is unreachable\n");
      return CONFIDENCE_FAILURE;
    }
#ifdef ACOUSTIC_ONLY
    sum = int_add(d->predlist->alpha_beta, d->succlist->alpha_beta);
#else
    sum = int_add(int_add(d->predlist->alpha_beta, d->succlist->alpha_beta), ca_lm_tg_score(lm, dict, d->predlist->to->wid, d->wid, d->succlist->to->wid));
#endif
    for (l = d->predlist; l; l = l->next)
      for (ll = d->succlist; ll; ll = ll->next) {
	if ((l == d->predlist) && (ll == d->succlist))
	  continue;
#ifdef ACOUSTIC_ONLY
	sum = logs3_add(sum, int_add(l->alpha_beta, ll->alpha_beta));
#else
	sum = logs3_add(sum, int_add(int_add(l->alpha_beta, ll->alpha_beta), ca_lm_tg_score(lm, dict, l->to->wid, d->wid, ll->to->wid)));
#endif
      }
    d->cscore = sum;
  }
  
  return CONFIDENCE_SUCCESS;
}


int pwp(seg_hyp_line_t *seg_hyp_line, ca_dag *word_lattice)
{
  conf_srch_hyp_t *w;
  ca_dagnode *d;
  int p;
#if 0
  int start, end;
#endif

  p = word_lattice->entry.to->cscore;

  /* In general the scores should always be the same. However, we are
     doing integer math so, it is likely that there are +-5
     difference 

     At this point, there are still bugs in the computation and for
     1/5 of the sentence the two scores are different.
  */

  if(p> word_lattice->exit.to->cscore + 5  || p <word_lattice->exit.to->cscore - 5 ){
  
    /* BUG: Alpha-Beta scores are not exactly the same for 5% of utterances.  
       E_WARN("The alpha-beta scores are different in the entry and the exit nodes (Word-end: %d, Word-begin: %d)\n",p, word_lattice->exit.to->cscore);*/
  }

  if (abs(p) < abs(word_lattice->exit.to->cscore))
    p = word_lattice->exit.to->cscore;
  seg_hyp_line->cscore = p;

  for (w = seg_hyp_line->wordlist; w; w = w->next) {
    w->sh.cscr = MIN_LOG;
    /*
      E_INFO("Word %s\n",w->sh.word);
    */
    for (d = word_lattice->nodelist; d; d = d->alloc_next) {
      /*      E_INFO("hihi, w->sh.cscr %d \n", w->sh.cscr);*/
#if 0 
      /* 20051128 Removed Rong's implementation which required checking of overlapping frame 
	 Still remain a question whether it is better. 
       */
      if ((overlap(d->sf, d->lef, w->sh.sf, w->sh.ef, &start, &end) >= 0) && 
	  ((strstr(d->word, w->sh.word) != NULL) || (strstr(w->sh.word, d->word) != NULL))) {
#endif

      if (strstr(d->word, w->sh.word) != NULL || strstr(w->sh.word, d->word) != NULL) {
	w->sh.cscr = logs3_add(d->cscore - p, w->sh.cscr);
      }
    }
  }

  return CONFIDENCE_SUCCESS;
}


static float backoff_mode(lm_t *lm, int wid1, int wid2, int wid3)
{
  int i, j;
  if (lm_tg_exists(lm, 
		   lm->dict2lmwid[wid1], 
		   lm->dict2lmwid[wid2], 
		   lm->dict2lmwid[wid3]) >= 0) {
    return 3.0;
  }

  i = lm_bg_exists(lm, 
		    lm->dict2lmwid[wid1], 
		    lm->dict2lmwid[wid2]
		    );

  j = lm_bg_exists(lm, 
		    lm->dict2lmwid[wid2], 
		    lm->dict2lmwid[wid3]);
  if ((i >= 0) && (j >= 0)) {
    return 2.5;
  }
  if (j > 0) {
    return 2.0;
  }

  i = lm_ug_exists(lm, lm->dict2lmwid[wid2]);
  j = lm_ug_exists(lm, lm->dict2lmwid[wid3]);
  if ((i >= 0) && (j >= 0)) {
    return 1.5;
  }
  if (j > 0) {
    return 1.0;
  }

  return 0;
}

int compute_lmtype(seg_hyp_line_t *seg_hyp_line, lm_t* lm,dict_t* dict)
{
  s3wid_t finishword, startword, unknownword, w1, w2, w3;
  conf_srch_hyp_t *h;

  finishword = dict_finishwid(dict);
  startword=  dict_startwid(dict);
  unknownword = BAD_S3WID;

#if 0
  if(unknownword==BAD_S3WID){
    E_ERROR("Can't find <UNK> in the dictionary");
    return CONFIDENCE_FAILURE;
  }
#endif
  w1 = finishword;
  w2 = startword;

  for (h = seg_hyp_line->wordlist; h; h = h->next) {
    if (dict_filler_word(dict, h->sh.id)) {
      h->lmtype = 2.0;
      h->l1 = h->l2 = h->l3 = 2.0;
      w3 = unknownword;
    }
    else {
      h->lmtype = backoff_mode(lm, w1, w2, h->sh.id);
      w3 = h->sh.id;
    }
    w1 = w2;
    w2 = w3;
  }

  seg_hyp_line->lmtype = backoff_mode(lm, w1, w2, finishword);
  return CONFIDENCE_SUCCESS;
}


int compute_combined_lmtype(seg_hyp_line_t *seg_hyp_line)
{
  conf_srch_hyp_t *h, *ph;

  ph = NULL;
  for (h = seg_hyp_line->wordlist; h; h = h->next) {
    if (ph == NULL)
      h->l1 = 3;
    else
      h->l1 = ph->lmtype;
    if (h->next == NULL) {
      h->l2 = seg_hyp_line->lmtype;
      h->l3 = 3;
    }
    else {
      h->l2 = h->next->lmtype;
      if (h->next->next == NULL)
	h->l3 = seg_hyp_line->lmtype;
      else
	h->l3 = h->next->next->lmtype;
    }
    ph = h;
  }
  return CONFIDENCE_SUCCESS;
}



int32 confidence_word_posterior(char* dagfile, seg_hyp_line_t *seg_hyp_line, char* uttid, lm_t *lm, dict_t *dict, fillpen_t *fpen)
{
  ca_dag word_lattice;

  if(ca_dag_load_lattice(dagfile,&word_lattice, lm, dict, fpen)==CONFIDENCE_FAILURE){
    E_WARN("Unable to load dag %s for uttid %s\n",dagfile,uttid);
    return CONFIDENCE_FAILURE;
  }

  /* Step 3, Compute Alpha-beta */
  if(alpha_beta(&word_lattice,lm, dict)==CONFIDENCE_FAILURE){
    E_WARN("Unable to compute alpha beta score for uttid %s\n", uttid);
    return CONFIDENCE_FAILURE;
  }

  /* Step 4, Compute Posterior WORD probability */
  if(pwp(seg_hyp_line,&word_lattice)==CONFIDENCE_FAILURE){
    E_WARN("Unable to compute pwp for uttid %s\n",uttid);
    return CONFIDENCE_FAILURE;
  }

  /* Step 8, Delete lattice, delete hypsegline */
  if(ca_dag_free_lattice(&word_lattice)==CONFIDENCE_FAILURE){
    E_WARN("Fail to free lattice.\n");
    return CONFIDENCE_FAILURE;
  }

  return CONFIDENCE_SUCCESS;
}
