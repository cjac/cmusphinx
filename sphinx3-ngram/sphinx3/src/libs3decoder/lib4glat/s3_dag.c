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
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * dag.c -- DAG search
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
 * 2008/06/23  N. Coetmeur, supervised by Y. Esteve
 * Replace lm_bg_score, lm_tg_score and lm_qg_score functions by lm_ng_score of
 * the new LM Ng model.
 *
 * 28-Jul-04    ARCHAN at Carnegie Mellon Unversity
 *              First adapted from s3.              
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 04-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_chk_linkscr().
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Implemented -maxedge argument to control memory usage.
 * 
 * 21-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxlmop and -maxlpf options to abort utterance if exceeded.
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added exact reporting of word sequence and scores from dag_search.
 * 		For this, added daglink_t.bypass, daglink_t.lscr, daglink_t.src, and
 * 		added bypass argument to dag_link and dag_bypass_link, and changed
 * 		dag_backtrace to find exact best path.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 * 
 * 03-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */
#define DEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "libutil.h"
/*#include <s3.h>*/

#include "s3types.h"
#include "s3_dag.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "s3_dag_search.h"
#include "logs3.h"
#include "linklist.h"


static dag_t dag[2];

static s3wid_t startwid;	/* Begin silence */
static s3wid_t finishwid;	/* End silence */



/* Global variables : Hack! */
dict_t *dict;		/* The dictionary */
fillpen_t *fpen;         /* The fillpenalty data structure */
lm_t *lm, *lm3g=NULL;                /* Global variables */


static s3_dag_srch_hyp_t *hyp = NULL;	/* The final recognition result */

static int32 maxlmop;		/* Max LM ops allowed before utterance aborted */
static int32 lmop;		/* #LM ops actually made */
static int32 maxedge;		/* Max #edges in DAG allowed before utterance aborted */
static uint8 le_meta = 0;

static int32 dag_lm_score_recusirf(lm_t *lm, daglink_t *pl,
								   daglink_t *l);


static daglink_t * succlist(void *  e) {
	if (le_meta ==0) return ((dagnode_t *)e)->succlist;
	return ((daglink_t *) e)->succlist;
}
static daglink_t ** psucclist(void *  e) {
	if (le_meta ==0) return &((dagnode_t *)e)->succlist;
	return & ((daglink_t *) e)->succlist;
}
static daglink_t * predlist(void *  e) {
	if (le_meta ==0) return ((dagnode_t *)e)->predlist;
	return ((daglink_t *) e)->predlist;
}



static daglink_t ** ppredlist(void *  e) {
	if (le_meta ==0) return &((dagnode_t *)e)->predlist;
	return &((daglink_t *) e)->predlist;
}



/* Get rid of old hyp, if any */
static void hyp_free ( void )
{
	s3_dag_srch_hyp_t *tmphyp;
	
	while (hyp) {
		tmphyp = hyp->next;
		listelem_free_es ((char *)hyp, sizeof(s3_dag_srch_hyp_t));
		hyp = tmphyp;
	}
}


static int32 filler_word (s3wid_t w)
{
	if ((w == startwid) || (w == finishwid))
		return 0;
	if ((w >= dict->filler_start) && (w <= dict->filler_end))
		return 1;
	return 0;
}
static int32 gram=3;
static int32 dumplattice =0;
void s3_dag_init (dict_t* _dict )
{ int32 i ;
	dict = _dict;
	
	/* Some key word ids */
	startwid = dict_wordid (dict, S3_START_WORD);
	finishwid = dict_wordid (dict,S3_FINISH_WORD);
	if ((NOT_S3WID(startwid)) || (NOT_S3WID(finishwid)))
		E_FATAL("%s or %s missing from dictionary\n", S3_START_WORD, S3_FINISH_WORD);
	
	/* Initialize DAG structure */
	for (i=0; i<2 ;i++)    
		dag[i].list = NULL;
	gram =  *((int32 *) cmd_ln_access ("-gram"));
	dumplattice =  *((int32 *) cmd_ln_access ("-dumplattice"));
	/* Set limit on max DAG edges allowed after which utterance is aborted */
	maxedge = *((int32 *) cmd_ln_access ("-maxedge"));
	dag[0].maxlink = *((int32 *) cmd_ln_access ("-maxlink"));
}
/* id e mais meta>=1 */
static int32 dag_link_meta (daglink_t *pd, daglink_t *d, int32 ascr, int32 ef, daglink_t **byp)
{
	daglink_t *l,*al;
	l=NULL;
	/* Link d into successor list for pd */
	if (pd) {	/* Special condition for root node which doesn't have a predecessor */
		
		l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
		
		l->succlist=NULL;
		l->predlist=NULL;
		l->node = d;
		l->src = pd;
		l->meta = le_meta;
		l->bypass = NULL;	/* This is a FORWARD link!! */
		l->ascr = ascr;
		l->pscr = (int32)0x80000000;
		l->pscr_valid = 0;
		l->history = NULL;
		l->ef = ef;
		l->next = pd->succlist;
		l->notpruned =1;
		pd->succlist = l;
	}
	al=l;
	/* Link pd into predecessor list for d */
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->succlist=NULL;
	l->predlist=NULL;
	l->reci =al;
	if (al) al->reci=l;
	l->notpruned =1  ;  /// au niveau meta pass de pruned
	l->node = pd; /* 10000; */
	l->src = d;
	l->meta =le_meta;
	l->bypass = NULL;
	if (byp ){
		l->bypass =*byp;
		*byp =l; // link des exits
		l->notpruned =-1;   //for meta+1
		if(al) al->notpruned =-1;
		
	}
	
	
	/*  link for exit */
	l->ascr_save=l->ascr = ascr;
	l->pscr = (int32)0x80000000;
	l->pscr_valid = 0;
	l->history = NULL;
	l->ef = ef;
	l->next = predlist(d);
	d->predlist= l;
	l->list=dag[le_meta].linklist;
	dag[le_meta].linklist=l;
	dag[le_meta].nlink++;
	
	return (dag[le_meta].nlink > maxedge) ? -1 : 0;
}


/*
 * Link two DAG nodes with the given arguments
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
static int32 dag_link (void *pd, void *d, int32 ascr, int32 ef, daglink_t *byp)
{
	daglink_t *l,*al;
	l=NULL;
	/* Link d into successor list for pd */
	if (pd) {	/* Special condition for root node which doesn't have a predecessor */
		l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
		l->succlist=NULL;
		l->predlist=NULL;
		l->node = d; 
		l->isfiller=(byp)? -1 : 0 ;/* l'arc n'existait pas */
		l->src = pd;
		l->meta = le_meta;
		l->bypass = byp;	/* This is a FORWARD link!! */
		l->ascr_save=l->ascr = ascr;
		l->pscr = (int32)0x80000000;
		l->pscr_valid = 0;
		l->history = NULL;
		l->ef = ef;
		l->next = succlist(pd);
		l->notpruned = dag[le_meta].maxlink==0;
		*psucclist(pd) = l;
	} 
	al=l;
	/* Link pd into predecessor list for d */
	
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->succlist=NULL;
	l->predlist=NULL;
	l->reci =al;
	l->notpruned =dag[le_meta].maxlink==0;
	if (al) al->reci=l; 
	else
		l->notpruned =1;
	l->node = pd; /* 10000; */
	l->src = d;
	l->meta =le_meta;
	l->bypass = byp;		/* This is a FORWARD link!! */
	l->ascr_save=l->ascr = ascr;
	l->pscr = (int32)0x80000000;
	l->pscr_valid = 0;
	l->history = NULL;
	l->ef = ef; 
	l->isfiller= (byp)? -1 : 0;
	l->next = predlist(d);
	*ppredlist(d) = l;
	
	dag[le_meta].nlink++;
	
	return (dag[le_meta].nlink > maxedge) ? -1 : 0;
}


static daglink_t *find_succlink (void *  src, void * dst)
{
	daglink_t *l;
	// le != n'est pas beautiful mais is deux void *
	for (l = succlist(src) ; l && (l->node != dst); l = l->next);
	return l;
}


static daglink_t *find_predlink (void * src, void * dst)
{
	daglink_t *l;
	
	for (l = predlist(src); l && (l->node != dst); l = l->next);
	return l;
}


/*
 * Like dag_link but check if link already exists.  If so, replace if new score better.
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
static int32 dag_update_link (dagnode_t *pd, dagnode_t *d, int32 ascr,
							  int32 ef, daglink_t *byp)
{
	daglink_t *l, *r;
	l = find_succlink (pd, d);
	
	if (! l)
		return (dag_link (pd, d, ascr, ef, byp));
	
	if (l->ascr < ascr) {
		r = find_predlink (d, pd);
		
		assert (r && (r->ascr == l->ascr) && r->reci==l);
		l->ascr = r->ascr = ascr;
		l->ef = r->ef = ef;
		l->bypass = r->bypass = byp;
	}
	
	return 0;
}


static int32 dag_param_read (FILE *fp, char *param, int32 *lineno)
{
	char line[1024], wd[1024];
	int32 n;
	
	while (fgets (line, 1024, fp) != NULL) {
		(*lineno)++;
		if (line[0] == '#')
			continue;
		if ((sscanf (line, "%s %d", wd, &n) == 2) && (strcmp (wd, param) == 0))
			return n;
	}
	return -1;
}


/*
 * Remove filler nodes from DAG by replacing each link TO a filler with links
 * to its successors.  In principle, successors can be fillers and the process
 * must be repeated.  But removing fillers in the order in which they appear in
 * dag.list ensures that succeeding fillers have already been eliminated.
 * Return value: 0 if successful; -1 if DAG maxedge limit exceeded.
 */
static int32 dag_remove_filler_nodes ( void )
{
	dagnode_t *d, *pnode, *snode;
	daglink_t *plink, *slink,*l;
	int32 ascr=0,i=0;
	
	assert(dag[le_meta].list);
	
	for (d = dag[le_meta].list; d; d = d->alloc_next) {
		if ( !d->alloc_next) { fprintf(stderr,"cela doit ete le dernier %d %d\n",d->index,d->seqid);}
		if (! filler_word (d->wid))
			continue;
		if (! d->alloc_next) { fprintf(stderr,"cela doit ete le dernier %d\n",d->index);
			for (l= d->succlist; l ; l= l->next) {printf ("%d ",i++);} 
		}	
		// fprintf(stderr," %d \n",d->seqid);
		/* Replace each link TO d with links to d's successors */
		for (plink = d->predlist; plink; plink = plink->next) {
			pnode = plink->node;
			//fprintf(stderr,"pred  %d  \n",pnode->seqid);
			ascr = plink->ascr; 
			ascr += fillpen(fpen,dict_basewid (dict,d->wid));
			
			/* Link this predecessor of d to successors of d */
			for (slink = d->succlist; slink; slink = slink->next) {
				snode = slink->node;
				//fprintf(stderr," %d  %d ",pnode->seqid,snode->seqid);
				/* Link only to non-filler successors; fillers have been eliminated */
				if (! filler_word (snode->wid) || snode->wid == finishwid) {
					/* Update because a link may already exist */
					//  fprintf(stderr,"connecte ");
					if (dag_update_link (pnode, snode,
										 ascr + slink->ascr, plink->ef, slink) < 0)
						return -1;
				} //fprintf(stderr,"\n");
			}
		}
		if (! d->alloc_next) { printf("cela doit ete le dernier %d\n",d->index);
			for (l= d->succlist; l ; l= l->next) {printf ("%d ",i++);} 
		}
	}
	return 0;
}
typedef daglink_t * pdaglink_t;
static int compare( const void *a, const void *b)
{ 
	const pdaglink_t *A=a ,*B=b;
	//fprintf(stderr,"je suis par ici \n");
	return  (*B)->lscr - (*A)->lscr;
	// fprintf(stderr,"je suis par la %d %d \n", (*A)->ef,(*B)->ef);
}

static void do_forward_backward(char * file) {
	int32 *alpha =(int32 *) ckd_malloc( sizeof(int32) * dag[0].nnode);
	int32 *beta =(int32 *) ckd_malloc( sizeof(int32) * dag[0].nnode);
	int32 newscore; 
	daglink_t ** tab=  (daglink_t **)ckd_malloc( sizeof(daglink_t * ) * dag[0].nlink);
	int i,compter ;
	int i_node;
	int supl=0,i_link =0;
	daglink_t *l;
	dagnode_t *node, *snode,*pnode;
	int32 theprune = dag[0].nfrm*dag[0].maxlink;
	s3lmwid32_t lwid[2];
	lm_t *lm_switched=lm;
	
	if (lm3g)
		lm_switched=lm3g;
	else
		if ( *(int32 *)cmd_ln_access("-gram") == 4)
			E_WARN("-lm3gPAP argument missing: BE SURE THAT 4G LM DOESN'T USE KNESER-NEY DISCOUNTING METHOD\n");
	
	
	E_INFO ("begin forward ");
	for (i=0 ; i<dag[0].nnode;i++) 
		alpha[i]=beta[i] = 10000;
	alpha[dag[0].nnode-1]=0.0;
	for (i_node =dag[0].nnode-1; i_node>=0 ; i_node--)
	{int32 tmpalpha = alpha[i_node];
		node= dag[0].darray[i_node]; 
		if (filler_word(node->wid)) {   for (l= node->predlist; l ; l=l->next)
		{ l->isfiller= l->reci->isfiller=1;}
			for (l= node->succlist; l ; l=l->next) l->reci->lscr= 0xefffffff;
			continue;}
		if (tmpalpha ==  10000) {
			//E_INFO("alpha pas connecte %d ",i_node);
			for (l= node->succlist; l ; l=l->next)
			{ l->notpruned=0;l->reci->notpruned=0;}
			
			continue;
		}
		//    fprintf(stdout,"--------------------%d %d %d %d\n",i_node,node->wid,node->lef,i_link);
		
		for (l=node->succlist; l; l=l->next) {
			snode = l->node;
			if (filler_word(snode->wid))
				continue;
			//      fprintf(stdout,"%d %d %d\n",snode->wid,snode->sf,snode->seqid);
			lwid[0] = lm_switched->dict2lmwid[dict_basewid(dict,node->wid)];
			lwid[1] = lm_switched->dict2lmwid[dict_basewid(dict,snode->wid)];
			l->lscr=l->reci->lscr = lm_ng_score (lm_switched, 2, lwid, snode->wid);
			
			tab[i_link++] = l; /* je l'ai pris a aller et j'ai mis notpruned */
			l->notpruned=l->reci->notpruned =1;
			newscore = tmpalpha+ l->lscr+ l->ascr;
			if (alpha[snode->index]== 10000)
				alpha[snode->index] = newscore;
			else  alpha[snode->index] = logmath_add(lm_switched->logmath, alpha[snode->index], newscore);
			//else  alpha[snode->index] = logs3_add( alpha[snode->index],newscore);
			assert (alpha[snode->index] <=0);
		}
	}
	E_INFO( "begin bacward");
	assert (i_link <= dag[0].nlink);
	E_INFO(" %d ilink\n",i_link);
	beta[0]=0;
	for (i=i_link-1; i>=0 ; i--) {
		l=tab[i];
		node= l->node;
		pnode=l->src;
		if (beta[node->index] == 10000) 
		{//printf(" pas vu ret %d %d,\n",i, node->index); 
			l->notpruned=l->reci->notpruned=0;
			continue;
		}
		newscore = beta[node->index] + l->lscr+l->ascr;
		if (beta[pnode->index] == 10000){ beta[pnode->index] = newscore;
			//    printf("j'ai vu %d avec %d \n",pnode->index,i);
		} 
		else
			beta[pnode->index] = logmath_add(lm_switched->logmath, beta[pnode->index], newscore);
		
	}
	E_INFO(" fin alpha-beta %d %d %d %s\n",beta[dag[0].nnode-1],alpha[0],abs(beta[dag[0].nnode-1]-alpha[0]),file);E_INFO( " max_link %d link %d\n", theprune,i_link);
	for (i=0;i<i_link;i++){ l= tab[i];
		if (!l->notpruned) l->lscr = (int32)(int32)0xd0000000; else
			
			l->lscr = l->lscr +l->ascr + alpha[((dagnode_t *)l->src)->index] + beta[((dagnode_t*) l->node)->index];
	}  
	qsort(tab,i_link,sizeof(daglink_t *), compare);
	if(  theprune >i_link) {
		for (i=0; i < i_link; i++)
		{ l= tab[i]; l->notpruned = l->reci->notpruned=1;   
			l->reci->lscr=   l->lscr = (int32)(int32)0xd0000000; //190104
		}}
	else {
		E_INFO("je prune %d %d \n", tab[0]->lscr,tab[theprune-1]->lscr);
		for (i = theprune  ; i< i_link; i++)
		{ l= tab[i]; l->notpruned = l->reci->notpruned=0;
			l->reci->lscr=   l->lscr = (int32)(int32)0xd0000000; //190104
			//l->map=l->lscr;
		}
		E_INFO("end pruning sort\n");
	}
	supl = (theprune >i_link)?  i_link : theprune;
	l= tab[0]; l->notpruned = l->reci->notpruned=1;l->reci->list=NULL;
	for (i=1; i < supl ; i++)
	{ l= tab[i]; l->notpruned = l->reci->notpruned=1;
		l->lscr = (int32)(int32)0xd0000000;
		l->reci->list = tab[i-1]->reci;
		//l->reci->map=l->lscr;
	}
	dag[0].linklist = tab[i-1]->reci;
	
	
	ckd_free(tab);  ckd_free(beta); ckd_free(alpha);
	compter=1;  /*   0 is for pseudo link final */
	for (i_node=0 ; i_node < dag[0].nnode ; i_node++) {
		node = dag[0].darray[i_node];
		for (l= node->predlist; l ; l=l->next)
		{ if (l->notpruned) l->index=compter++ ;}
	}
	dag[1].nnode = compter+1;
	E_INFO(" nprune %d,  ncompte %d \n", supl,compter);
	assert(compter==supl+1);
}


static void do_map_tg(lm_t *lm_switched, char * file)  {
	int32 *alpha =(int32 *) ckd_malloc( sizeof(int32) * dag[1].nnode);
	int32 *beta =(int32 *) ckd_malloc( sizeof(int32) * dag[1].nnode);
	daglink_t ** tab=  (daglink_t **)ckd_malloc( sizeof(daglink_t * ) * dag[1].nlink);
	daglink_t ** darray =(daglink_t **)  ckd_calloc(sizeof(daglink_t *) , dag[1].nnode);
	int32 newscore,proba_ph; 
	int compte=0;
	int i ;
	int i_node;
	int i_link =0;
	daglink_t *l;
	daglink_t *node, *snode,*pnode;
	
	E_INFO ("begin map ");
	E_INFO("je fais pas");
	return;
	darray[0] = NULL;
	for (l=dag[0].linklist; l ; l=l->list) 
	{ darray[l->index] =l;compte++;}
	E_INFO(" compte %d %d \n",compte,dag[1].nnode);
	for (i=0 ; i<dag[1].nnode;i++) 
		alpha[i]=beta[i] = 10000;
	assert (darray[dag[1].nnode-1]->node==NULL);
	alpha[dag[1].nnode-1]=0.0;
	
	for (i_node =dag[1].nnode-1; i_node>0 ; i_node--)  /* 0 est pseudo node */
	{
		int32 tmpalpha = alpha[i_node];
		node= darray[i_node]; 
		if (tmpalpha ==  10000) {
			// E_INFO("alpha pas connecte %d ",i_node);
			for (l= node->succlist; l ; l=l->next)
			{ l->notpruned=0;l->reci->notpruned=0;}
			
			continue;
		}
		//    fprintf(stdout,"--------------------%d %d %d %d\n",i_node,node->wid,node->lef,i_link);
		
		for (l=node->succlist; l; l=l->next) {
			snode = l->node;
			tab[i_link++] =l;
			//      fprintf(stdout,"%d %d %d\n",snode->wid,snode->sf,snode->seqid);
			l->lscr=l->reci->lscr = dag_lm_score_recusirf(lm_switched, node,snode); 
			l->notpruned=l->reci->notpruned =1;
			
			//			newscore = tmpalpha+ l->lscr/lm_switched->lw + l->ascr/10; /* On essaie de faire ça proprement. Yannick */
			//				newscore = tmpalpha+ l->lscr + l->ascr; /* On essaie de faire ça proprement. Yannick */
			newscore = tmpalpha+ (l->lscr + l->ascr)/10; /* On essaie de faire ça proprement. Yannick */
			
			if (alpha[snode->index]== 10000)
				alpha[snode->index] = newscore;
			else  alpha[snode->index] = logmath_add(lm_switched->logmath, alpha[snode->index], newscore);
			assert (alpha[snode->index] <=0);
		}
		
	}
	
	newscore=10000;
	for (l=((dagnode_t *) dag[0].exit.node)->predlist; l; l=l->next) {
		if (l->notpruned && alpha[l->index] !=10000) {
			if (newscore !=10000)  newscore = logmath_add(lm_switched->logmath, newscore, alpha[l->index]);
			else newscore = alpha[l->index];
		}
	}
	alpha[0] = newscore;
	assert (newscore <0);
	assert (i_link<= dag[1].nlink);
	E_INFO( "begin backward\n");
	beta[0]=0;
	for (l=((dagnode_t *) dag[0].exit.node)->predlist; l; l=l->next) {
		if (l->notpruned && alpha[l->index] !=10000) {
			beta[l->index] = beta[0];
		}
	}
	
	for (i=i_link-1; i>=0 ; i--) {
		l=tab[i];
		node= l->node;
		pnode=l->src;
		if (beta[node->index] == 10000) 
		{//printf(" pas vu ret %d %d,\n",i, node->index); 
			l->notpruned=l->reci->notpruned=0;
			continue;
		}
		
		//	  newscore = beta[node->index] + l->lscr+l->ascr;
		newscore = beta[node->index] + (l->lscr+l->ascr)/10;
		
		
		
		if (beta[pnode->index] == 10000){ beta[pnode->index] = newscore;
			//    printf("j'ai vu %d avec %d \n",pnode->index,i);
		} 
		else
			beta[pnode->index] = logmath_add(lm_switched->logmath, beta[pnode->index], newscore);
		assert (beta[pnode->index] && beta[pnode->index] + alpha[pnode->index] <=alpha[0]+20 );
	}
	
	E_INFO(" fin alpha-beta %d %d %d %s\n",beta[dag[1].nnode-1],alpha[0],abs(beta[dag[1].nnode-1]-alpha[0]),file);E_INFO( "  link %d\n",i_link);
	if (beta[dag[0].nnode-1] <alpha[0] ) proba_ph= beta[dag[0].nnode-1] ;
	proba_ph= alpha[0];
	for (i=0;i<i_link;i++) {
		l= tab[i];
		if (!l->notpruned) l->lscr = (int32)(int32)0xd0000000; else
			//      newscore = l->lscr/lm_switched->lw +l->ascr/10 + alpha[((daglink_t *)l->src)->index] + beta[((daglink_t*) l->node)->index]; /* On essaie encore de faire ça proprement ... */
			//	      newscore = l->lscr + l->ascr + alpha[((daglink_t *)l->src)->index] + beta[((daglink_t*) l->node)->index]; /* On essaie encore de faire ça proprement ... */
			newscore = (l->lscr + l->ascr)/10 + alpha[((daglink_t *)l->src)->index] + beta[((daglink_t*) l->node)->index]; /* On essaie encore de faire ça proprement ... */
		
		assert(newscore-proba_ph<10);
		l->reci->index=l->index=newscore-proba_ph;
	}  
	for (i=0; i < i_link; i++)
	{ l= tab[i]; l->notpruned = l->reci->notpruned=1;   
		l->reci->lscr=   l->lscr = (int32)(int32)0xd0000000; //190104
	}
	ckd_free(darray);
	ckd_free(tab); ckd_free(beta);   ckd_free(alpha);
	
}
/* fin de do pap */


/*
 * Load a DAG from a file: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * dagnodes_list = linear list of DAG nodes allocated, ordered such that nodes earlier
 * in the list can follow nodes later in the list, but not vice versa:  Let two DAG
 * nodes d1 and d2 have start times sf1 and sf2, and end time ranges [fef1..lef1] and
 * [fef2..lef2] respectively.  If d1 appears later than d2 in dag.list, then
 * fef2 >= fef1, because d2 showed up later in the word lattice.  If there is a DAG
 * edge from d1 to d2, then sf1 > fef2.  But fef2 >= fef1, so sf1 > fef1.  Reductio ad
 * absurdum.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 s3dag_dag_load (char *file)
{
	FILE *fp;
	char line[16384], wd[1024];
	int32 nfrm, nnode, sf, fef, lef, ef, lineno;
	int32 i, j, k, l, final, seqid, from, to, ascr;
	int32 fudge, min_ef_range;
	dagnode_t *d, *pd, *tail, **darray;
	s3wid_t w;
	struct lat_s {
		dagnode_t *node;
		int32 ef;
		int32 ascr;
	} *lat;		/* Lattice (bptable) entries in each frame */
    int32 *frm2lat;	/* frm2lat[f] = first lattice entry for frame f */
    float32 lb, f32arg;
    int32 ispipe;
    
    dag[le_meta].list = NULL;
    dag[le_meta].nlink = 0;
    dag[le_meta].nbypass =0;
    
    tail = NULL;
    darray = NULL;
    lat = NULL;
    frm2lat = NULL;
    lineno = 0;
    
    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen_compchk (file, &ispipe)) == NULL) {
		E_ERROR("fopen_compchk(%s) failed\n", file);
		return -1;
    }
    
    /* Read and verify logbase (ONE BIG HACK!!) */
    if (fgets (line, sizeof(line), fp) == NULL) {
		E_ERROR ("Premature EOF(%s)\n", file);
		goto load_error;
    }
    if (strncmp (line, "# getcwd: ", 10) != 0) {
		E_ERROR ("%s does not begin with '# getcwd: '\n", file);
		goto load_error;
    }
    if (fgets (line, sizeof(line), fp) == NULL) {
		E_ERROR ("Premature EOF(%s)\n", file);
		goto load_error;
    }
    if ((strncmp (line, "# -logbase ", 11) != 0) || (sscanf (line+11, "%f", &lb) != 1)) {
		E_WARN ("%s: Cannot find -logbase in header\n", file);
    } else {
		f32arg = *((float64 *) cmd_ln_access ("-logbase"));
		if ((lb <= 1.0) || (lb > 2.0) || (f32arg <= 1.0) || (f32arg > 2.0))
			E_ERROR ("%s: logbases out of range; cannot be verified\n", file);
		else {
			int32 orig, this;
			float64 diff;
			
			/* HACK TRES MOCHE (Antho, LIUM) */
			logmath_t *l_orig, *l_this;
			
			l_orig = logs3_init(lb, 0, 0);
			l_this = logs3_init(f32arg, 0, 0);
			
			orig = logs3(l_orig, lb - 1.0);
			this = logs3(l_this, f32arg - 1.0);
			diff = ((orig - this) * 1000.0) / orig;
			if (diff < 0)
				diff = -diff;
			
			if (diff > 1.0)		/* Hack!! Hardwired tolerance limits on logbase */
				E_ERROR ("%s: logbase inconsistent: %e\n", file, lb);
		}
    }
    
	
    /* Min. endframes value that a node must persist for it to be not ignored */
    min_ef_range = *((int32 *) cmd_ln_access ("-min_endfr"));
    
    /* Read Frames parameter */
    nfrm = dag_param_read (fp, "Frames", &lineno);
    if (nfrm <= 0) {
		E_ERROR("Frames parameter missing or invalid\n");
		goto load_error;
    }
    dag[le_meta].nfrm = nfrm;
	
    
    /* Read Nodes parameter */
    lineno = 0;
    nnode = dag_param_read (fp, "Nodes", &lineno);
    if (nnode <= 0) {
		E_ERROR("Nodes parameter missing or invalid\n");
		goto load_error;
    }
    
    /* Read nodes */
    darray = (dagnode_t **) ckd_calloc (nnode, sizeof(dagnode_t *));
    dag[0].nnode= nnode;
    dag[0].darray = darray;
	for (i = 0; i < nnode; i++) {
		if (fgets (line, 1024, fp) == NULL) {
			E_ERROR ("Premature EOF(%s)\n", file);
			goto load_error;
		}
		lineno++;
		
		if ((k = sscanf (line, "%d %s %d %d %d", &seqid, wd, &sf, &fef, &lef)) != 5) {
			E_ERROR("Bad line: %s\n", line);
			goto load_error;
		}
		
		w = dict_wordid (dict,wd);
		if (NOT_S3WID(w)) {
			
			E_INFO("Unknown word: %s\n", line);
			strcpy (wd,"<unk>");
			w = dict_wordid (dict,wd);
			if (NOT_S3WID(w)) {
				E_ERROR("Unknown word: %s\n", line);
				goto load_error;}
		}
		
		if (seqid != i) {
			E_ERROR("Seqno error: %s\n", line);
			goto load_error;
		}
		
		d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));
		darray[i] = d;
		d->index =i;
		d->wid = w;
		d->seqid = seqid;
		d->sf = sf;
		d->fef = fef;
		d->lef = lef;
		d->reachable = 0;
		d->succlist = NULL;
		d->predlist = NULL;
		d->alloc_next = NULL;
		
		if (! dag[le_meta].list)
			dag[le_meta].list = d;
		else
			tail->alloc_next = d;
		tail = d;
	}
	
    /* Read initial node ID */
    k = dag_param_read (fp, "Initial", &lineno);
    if ((k < 0) || (k >= nnode)) {
		E_ERROR("Initial node parameter missing or invalid\n");
		goto load_error;
    }
    dag[le_meta].entry.node = darray[k];
    dag[le_meta].entry.ascr = 0;
    dag[le_meta].entry.next = NULL;
    dag[le_meta].entry.pscr_valid = 0;
    
    /* Read final node ID */
    k = dag_param_read (fp, "Final", &lineno);
    if ((k < 0) || (k >= nnode)) {
		E_ERROR("Final node parameter missing or invalid\n");
		goto load_error;
    }
    dag[le_meta].exit.node = darray[k];
    dag[le_meta].exit.ascr = 0;
    dag[le_meta].exit.next = NULL;
    dag[le_meta].exit.pscr_valid = 0;
    dag[le_meta].exit.bypass = NULL;
    final = k;
	
    /* Read bestsegscore entries */
    if ((k = dag_param_read (fp, "BestSegAscr", &lineno)) < 0) {
		E_ERROR("BestSegAscr parameter missing\n");
		goto load_error;
    }
	
    lat = (struct lat_s *) ckd_calloc (k, sizeof(struct lat_s));
    frm2lat = (int32 *) ckd_calloc (nfrm+1, sizeof(int32));
	
	
	// il n' y a pas de bestscore don dagfudege is not good 
	
    j = -1;
    for (i = 0; i < k; i++) {
		if (fgets (line, 1024, fp) == NULL) {
			E_ERROR("Premature EOF(%s)\n", line);
			goto load_error;
		}
		
		lineno++;
		
		if (sscanf (line, "%d %d %d", &seqid, &ef, &ascr) != 3) {
			E_ERROR("Bad line: %s\n", line);
			goto load_error;
		}
		
		if ((seqid < 0) || (seqid >= nnode)) {
			E_ERROR("Seqno error: %s\n", line);
			goto load_error;
		}
		
		if (ef != j) {
			for (j++; j <= ef; j++)
				frm2lat[j] = i;
			--j;
		}
		lat[i].node = darray[seqid];
		lat[i].ef = ef;
		lat[i].ascr = ascr;
		if (le_meta ==0) {
			if ((seqid == final) && (ef ==((dagnode_t *) dag[le_meta].exit.node)->lef))
				dag[le_meta].exit.ascr = ascr;
		}    }
    for (j++; j <= nfrm; j++)
		frm2lat[j] = (k>0) ? k : -1 ; //si pas de bestscore pas de frm2lat
    
    /* Read in edges */
    while (fgets (line, 1024, fp) != NULL) {
		lineno++;
		
		if (line[0] == '#')
			continue;
		if ((sscanf (line, "%s%d", wd,&k) == 1) && (strcmp (wd, "Edges") == 0))
			break;
    }
    k = 0;
    while (fgets (line, 1024, fp) != NULL) {
		lineno++;
		if (sscanf (line, "%d %d %d", &from, &to, &ascr) != 3)
			break;
		pd = darray[from];
		if (pd->wid == finishwid)
			continue;
		d = darray[to];
		
		/* Skip short-lived nodes */
		if ((pd == dag[le_meta].entry.node) || (d == dag[le_meta].exit.node) ||
			((d->lef - d->fef >= min_ef_range-1) && (pd->lef - pd->fef >= min_ef_range-1))) {
			if (dag_link (pd, d, ascr, d->sf-1, NULL) < 0) {
				E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, maxedge);
				goto load_error;
			}
			
			k++;
		}
    }
    if (strcmp (line, "End\n") != 0) {
		E_ERROR("Terminating 'End' missing\n");
		goto load_error;
    }
    
#if 0
    /* Build edges from lattice end-frame scores if no edges input */
    if (k == 0) {
		E_INFO("No edges in dagfile; using lattice scores\n");
		for (d = dag[le_meta].list; d; d = d->alloc_next) {
			if (d->sf == 0)
				assert (d->wid == startwid);
			else if ((d == dag[le_meta].exit.node) || (d->lef - d->fef >= min_ef_range-1)) {
				/* Link from all end points == d->sf-1 to d */
				for (l = frm2lat[d->sf-1]; l < frm2lat[d->sf]; l++) {
					pd = lat[l].node;		/* Predecessor DAG node */
					if (pd->wid == finishwid)
						continue;
					
					if ((pd == dag[le_meta].entry.node) || (pd->lef - pd->fef >= min_ef_range-1)) {
						dag_link (pd, d, lat[l].ascr, d->sf-1, NULL);
						k++;
					}
				}
			}
		}
    }
#endif
	
    fudge = *((int32 *) cmd_ln_access ("-dagfudge"));
    if (fudge > 0 && frm2lat[0] >=0 ) { // si pas de bestscore pas de frm2lat et donc pas de fuge
		// a mom avis le #endif aurait du aller plus loin paul
		/* Add "illegal" links that are near misses */
		for (d = dag[le_meta].list; d; d = d->alloc_next) {
			if (d->lef - d->fef < min_ef_range-1)
				continue;
			
			/* Links to d from nodes that first ended just when d started */
			for (l = frm2lat[d->sf]; l < frm2lat[d->sf+1]; l++) {
				pd = lat[l].node;		/* Predecessor DAG node */
				if ((pd->wid != finishwid) && (pd->fef == d->sf) &&
					(pd->lef - pd->fef >= min_ef_range-1)) {
					dag_link (pd, d, lat[l].ascr, d->sf-1, NULL);
					k++;
				}
			}
			
			if (fudge < 2)
				continue;
			
			/* Links to d from nodes that first ended just BEYOND when d started */
			for (l = frm2lat[d->sf+1]; l < frm2lat[d->sf+2]; l++) {
				pd = lat[l].node;		/* Predecessor DAG node */
				if ((pd->wid != finishwid) && (pd->fef == d->sf+1) &&
					(pd->lef - pd->fef >= min_ef_range-1)) {
					dag_link (pd, d, lat[l].ascr, d->sf-1, NULL);
					k++;
				}
			}
		}
    }
    
    fclose_comp (fp, ispipe);
	
    ckd_free (lat);
    ckd_free (frm2lat);
    
    /*
     * HACK!! Change dag[le_meta].exit.node wid to finishwid if some other filler word,
     * to avoid complications with LM scores at this point.
     */
    if (le_meta == 0) {
		dag[le_meta].orig_exitwid = ((dagnode_t *)dag[le_meta].exit.node)->wid;
		if (filler_word(((dagnode_t *)dag[le_meta].exit.node)->wid))
			((dagnode_t *)	dag[le_meta].exit.node)->wid = finishwid;
    }
    /* Add links bypassing filler nodes */
	if (dag_remove_filler_nodes () < 0) {
		E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, maxedge);
		return -1;
	}
	
	if (dag[0].maxlink !=0)
		do_forward_backward(file);
    ckd_free (darray);
    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link (NULL, dag[le_meta].entry.node, 0, -1, NULL);
    // the dummy in list of link not pruned    for dag[0]
    ((dagnode_t *) dag[le_meta].entry.node)->predlist->list =  dag[0].linklist;
    dag[0].linklist=   ((dagnode_t *) dag[le_meta].entry.node)->predlist;
    dag[0].linklist->index=dag[1].nnode-1;
    E_INFO("%5d frames, %6d nodes, %8d edges\n", nfrm, nnode, dag[le_meta].nlink);
    
    return dag[le_meta].nfrm;
	
load_error:
    E_ERROR("Failed to load %s\n", file);
    if (fp)
		fclose_comp (fp, ispipe);
    if (darray)
		ckd_free (darray);
    if (lat)
		ckd_free (lat);
    if (frm2lat)
		ckd_free (frm2lat);
    return -1;
}


int32 s3_dag_destroy ( void )
{
	dagnode_t *d, *nd;
	daglink_t *l, *nl;
	int32 i;
	for (d = dag[le_meta].list; d; d = nd) {
		nd = d->alloc_next;
		
		for (l = d->succlist; l; l = nl) {
			nl = l->next;
			listelem_free_es ((char *)l, sizeof(daglink_t));
		}
		for (l = d->predlist; l; l = nl) {
			nl = l->next;
			listelem_free_es ((char *)l, sizeof(daglink_t));
		}
		
		listelem_free_es ((char *)d, sizeof(dagnode_t));
	}
	for (i=0; i<2 ;i++)    
		dag[i].list = NULL;
	dag[le_meta].list = NULL;
	linklist_stats();
	return 0;
}

static void dag_l2n(daglink_t *pl,daglink_t *l,dagnode_t ** eclat,uint32 * teclat)
{// the commun node is do by l and not by pl
	assert (l==NULL || pl->meta == l->meta|| 1); // faudrait pas charier
	if (pl->node != NULL)
	{if (pl->meta ==0) eclat[(*teclat)++] = pl->node;
    else dag_l2n(pl->node,NULL,eclat,teclat);
	}
	if (l!=NULL) {
		if (l->meta ==0) {
			eclat[(*teclat)++]=l->node;
			eclat[(*teclat)++]=l->src;
		} else {
			dag_l2n(l->node,l->src,eclat,teclat);
		}
	}
}




void dag_dodagmeta( int meta) {
	int32 ameta=le_meta;
	daglink_t * red_dir, * red_node, *red_src,*l;
	
	
	
	dag[meta].linklist=NULL;
	dag[meta].entry.next=NULL;
	dag[meta].exit.next=NULL;
	dag[meta].entry.node=dag[meta].exit.node=NULL;
	le_meta = meta;
	if (meta == 1) {
		// le dag -1 a des dag_node au milieu
		dagnode_t * milieu, *exit = dag[0].exit.node;
		for (l= exit->predlist ; l ; l=l->next)
			l->notpruned = l->reci->notpruned = - l->notpruned;
		
		for (red_src = dag[meta-1].linklist ; red_src ; red_src= red_src->list) {
			// bacwardlink red_src -> red_node but the true is red_node->red->src
			if (red_src->notpruned <1) continue;
			milieu = red_src->src;
			assert (milieu != exit);
			
			for (red_dir= milieu->succlist; red_dir ; red_dir= red_dir->next){
				assert(red_dir->reci); // il y a bacward link
				red_node=red_dir->reci; // le meta is fait sur backward
				if (red_node->notpruned) {
					if (red_node->notpruned <0)
						dag_link_meta(red_src,red_node,red_node->ascr,red_node->ef,&dag[meta].exit.next);
					else
						dag_link_meta(red_src,red_node,red_node->ascr,red_node->ef,NULL);
				}
			}
			dag_link_meta(NULL,((dagnode_t *)dag[0].entry.node)->predlist,0,-meta-1,NULL);
			dag[meta].entry.predlist=((dagnode_t *)dag[0].entry.node)->predlist;
		}
	}
	else {assert (meta ==1); } //a faire plus tard
	le_meta=ameta;
	{ int i= 0;
		daglink_t * l;
		for (l=dag[meta].linklist ; l ;l=l->list) {assert (l->meta == meta); i++;}
		E_INFO(" niv %d : %d edges\n",meta,i);
		dag[meta].nlink=i;
	}
}


static void do_freemeta(int meta) {
	daglink_t *l,*al,*temp =NULL;
	
	assert(meta==1);
	free(dag[meta].eLM);
	
	for (l=dag[meta].linklist ; l ; l=al) {
		al= l->list;
		{temp=l->reci;
			listelem_free_es ((char *)l, sizeof(daglink_t ));
			if (temp )listelem_free_es ((char *)temp, sizeof(daglink_t ));
		}
		
	}
}
static dagnode_t * fin ( daglink_t * l) { if (l->meta==0) 
	return l->src;
else  return fin(l->src);
}



static int32 dag_lm_score_recusirf(lm_t *lm, daglink_t *pl,
								   daglink_t *l)
{
	dagnode_t * eclat[20];  // meta <20 il y a de la marge
	s3lmwid32_t lwid[4];
	int32 ret,i;
	uint32  nnode=0;
	
	dag_l2n(pl,l,eclat,&nnode);
	
	if ( (nnode >= 2) && (nnode <= 4) ) {
		lwid[0] = lm->dict2lmwid[dict_basewid(dict,eclat[0]->wid)]; 
		lwid[1] = lm->dict2lmwid[dict_basewid(dict,eclat[1]->wid)];
		if (nnode >= 3)
			lwid[2] = lm->dict2lmwid[dict_basewid(dict,eclat[2]->wid)];
		if (nnode >= 4)
			lwid[3] = lm->dict2lmwid[dict_basewid(dict,eclat[3]->wid)];
		
		ret = lm_ng_score (lm, nnode, lwid, lwid[nnode-1]);
	}
	else
		assert (nnode < 0);
	
	if(DEBUG) 
	{
	    fprintf(stderr,"LMTRACE: ");
	    for (i=0;i<nnode; i++)
			fprintf(stderr,"%s ",dict_wordstr(dict,dict_basewid(dict,eclat[i]->wid)));
	    fprintf(stderr,"%i\n",ret);
	}
	return ret;
}
static int32 nb_prune_dyn =0;

static int32 dag_bestpath_meta ( daglink_t *l)	/* Backward link! */
{
	daglink_t *d, *pd;
	daglink_t *pl;
	int32 lscr, score;
	assert (! l->pscr_valid);
	// printf("bespath %d\n,",l->ef);
	if ((d = l->node) == NULL) {
		/* If no destination at end of l, src is root node.  Recursion termination */
		assert (dict_basewid(dict,(fin(l)->wid)) == startwid);
		l->lscr = 0;
		l->pscr = 0;
		l->pscr_valid = 1;
		l->history = NULL;
		
		return 0;
	}
	/*  l->lscr=(int32)0x80000000;
	 if (1) {
	 if (verif==NULL) verif=fopen("verif.dat","w");
	 for (pl = d->predlist; pl; pl = pl->next) {
	 assert (pl->meta >0);
	 if (!pl->notpruned && pl->node != NULL) continue;
	 dag_lm_score_recusirf_verif(verif, pl,l);
	 }
	 fprintf(verif,"------------------------------------------------------------------------------------\n");
	 }
	 */
	/* Search all predecessor links of l */
	for (pl = d->predlist; pl; pl = pl->next) {
		assert (pl->meta >0);
		if (!pl->notpruned && pl->node != NULL) continue;     
		pd = pl->node;
		
		// not filler si meta >=1    if ( (pd && filler_word (pd->wid)))	/* Skip filler node */
		//continue;
		
		/* Evaluate best path along pl if not yet evaluated (recursive step) */
		if (! pl->pscr_valid)
			if (dag_bestpath_meta (pl) < 0)
				return -1;
		
		/* Accumulated path score along pl->l */
		if (pl->pscr > (int32)0x80000000) {
			score = pl->pscr + l->ascr;
			if (score > l->pscr) {	/* rkm: Added 20-Nov-1996 */
				lscr = dag_lm_score_recusirf(lm, pl,l);
				//        lscr = l->lscr;
				score += lscr;
				
				if (lmop++ >= maxlmop)
				{nb_prune_dyn ++;   
					return -1;
				}
				/* Update best path and score beginning with l */
				if (score > l->pscr) {
					l->lscr = lscr;
					l->pscr = score;
					l->history = pl;
				}
			}
		}
	}
	l->pscr_valid = 1;
	
	return 0;
}
// for backwardlink
static daglink_t * linkfin ( daglink_t * l) { if (l->meta==0) 
	return l;
else  return linkfin(l->src);
}

static daglink_t * dag_meta_to_norm (daglink_t *l) 
{//l is the best exit metalink 
	//return the best link exit and do traceback link
	daglink_t * lfin=NULL, *afin=NULL, *retour=NULL;
	retour = linkfin(l);
	for (;l;l=l->history) {
		lfin = linkfin(l);
		lfin->lscr= l->lscr;
		lfin->pscr = l->pscr;
		lfin->pscr_valid = l->pscr_valid;
		lfin->index = l->index;
		assert(l->pscr_valid);
		if(afin) afin->history = lfin;
		afin=lfin;
	}
	return retour;
}
static void    to_meta0(void) {
	daglink_t *l,*temp =NULL;  
	assert(gram==4 );
	for (l=dag[1].linklist ; l ; l=l->list)
	{temp= linkfin(l);
		if (0 && (l->lscr>=0 ||  temp->lscr>=0 )) {
			
			dagnode_t * eclat[20];  // meta <20 il y a de la marge
			uint32  nnode=0,i;
			dag_l2n(l,temp ,eclat,&nnode);
			printf(" :");
			for (i=0;i<nnode; i++)
				if (eclat[i]!=NULL)   printf("%s ",dict_wordstr(dict,eclat[i]->wid));
				else printf(" null ");
			
			
			printf(" score 0 :%d %d %llu %llu %d\n", l->lscr, temp->lscr, (uint64)l, (uint64)temp, temp->notpruned);	    
			
		}
		if (l->lscr > temp->lscr)  temp->lscr= l->lscr;
	}
}
/*
 * Recursive step in dag_search:  best backward path from src to root beginning with l.
 * Return value: 0 if successful, -1 otherwise.
 */
static int32 dag_bestpath (
						   daglink_t *l,	/* Backward link! */
						   dagnode_t *src)	/* Source node for backward link l */
{
	dagnode_t *d, *pd;
	daglink_t *pl;
	int32 lscr, score;
	s3lmwid32_t wid[3];
	assert (l->src == src);
	assert (! l->pscr_valid);
	
	if ((d = l->node) == NULL) {
		/* If no destination at end of l, src is root node.  Recursion termination */
		assert (dict_basewid(dict,src->wid) == startwid);
		l->lscr = 0;
		l->pscr = 0;
		l->pscr_valid = 1;
		l->history = NULL;
		
		return 0;
	}
	
	/* Search all predecessor links of l */
	for (pl = d->predlist; pl; pl = pl->next) {
		if (!pl->notpruned && pl->node != NULL) continue;     
		pd = pl->node;
		if ( (pd && filler_word (pd->wid)))	/* Skip filler node */
			continue;
		
		/* Evaluate best path along pl if not yet evaluated (recursive step) */
		if (! pl->pscr_valid)
			if (dag_bestpath (pl, d) < 0)
				return -1;
		
		/* Accumulated path score along pl->l */
		if (pl->pscr > (int32)0x80000000) {
			score = pl->pscr + l->ascr;
			if (score > l->pscr) {	/* rkm: Added 20-Nov-1996 */
				wid[2] = lm->dict2lmwid[dict_basewid(dict,src->wid)],
				wid[1] = lm->dict2lmwid[dict_basewid(dict,d->wid)];
				if (pd) {
					wid[0] = lm->dict2lmwid[dict_basewid(dict,pd->wid)];
					lscr = lm_ng_score (lm, 3, wid, src->wid);
				}
				else
					lscr = lm_ng_score (lm, 2, &(wid[1]), src->wid);
				score += lscr;
				
				if (lmop++ >= maxlmop)
					return -1;
				
				/* Update best path and score beginning with l */
				if (score > l->pscr) {
					l->lscr = lscr;
					l->pscr = score;
					l->history = pl;
				}
			}
		}
	}
	
#if 0
	printf ("%s,%d -> %s,%d = %d\n",
			dict_wordstr (dict,d->wid), d->sf,
			dict_wordstr (dict,src->wid), src->sf,
			l->pscr);
#endif
	
	l->pscr_valid = 1;
	
	return 0;
}


/*
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
static s3_dag_srch_hyp_t *dag_backtrace (daglink_t *l)
{
	s3_dag_srch_hyp_t *h, *hhead, *htail;
	int32 pscr;
	dagnode_t *src, *dst;
	daglink_t *bl, *hist;
	
	hyp = NULL;
	dst = NULL;
	
	for (; l; l = hist) {
		hist = l->history;
		
		if (hyp)
	    {  hyp->lscr = l->lscr;	/* As lscr actually applies to successor node */
			//	hyp->map =l->map;
			hyp->notpruned = l->notpruned;}
		if (! l->node) {
			assert (! l->history);
			break;
		}
		if (le_meta ==0) {dagnode_t * pnode =l->node;
			if (! l->bypass) {
				/* Link did not bypass any filler node */
				h = (s3_dag_srch_hyp_t *) listelem_alloc (sizeof(s3_dag_srch_hyp_t));
				h->wid =pnode->wid;
				h->pap = l->index;
				h->word = dict_wordstr (dict,h->wid);
				h->sf = pnode->sf;
				h->ef = l->ef;
				h->ascr = l->ascr;
				//h->map =l->map;
				h->notpruned = l->notpruned;
				h->next = hyp;
				hyp = h;
			} else {
				/* Link bypassed one or more filler nodes; restore bypassed link seq. */
				hhead = htail = NULL;
				
				src = pnode;	/* Note that l is a PREDECESSOR link */
				for (; l; l = l->bypass) {
					h = (s3_dag_srch_hyp_t *) listelem_alloc (sizeof(s3_dag_srch_hyp_t));
					h->wid = src->wid;
					h->word = dict_wordstr (dict,h->wid);
					h->sf = src->sf;
					h->pap =0;
					if (hhead)
						h->lscr = fillpen (fpen,dict_basewid (dict,src->wid));
					else
						h->pap = l->index;
					
					if (l->bypass) {
						dst = l->bypass->src;
						assert (filler_word (dst->wid));
						bl = find_succlink (src, dst);
						assert (bl);
					} else
						bl = l;
					
					h->ef = bl->ef;
					h->ascr = bl->ascr;
					if (htail)
						htail->next = h;
					else
						hhead = h;
					htail = h;
					
					src = dst;
				}
				
				htail->next = hyp;
				hyp = hhead;
			}}
	}
	
	/* Compute path score for each node in hypothesis */
	pscr = 0;
	for (h = hyp; h; h = h->next) {
		pscr = pscr + h->lscr + h->ascr;
		h->pscr = pscr;
	}
	
	return hyp;
}


static int32 dag_chk_linkscr (dag_t *dag)
{
	dagnode_t *d;
	daglink_t *l;
	
	for (d = dag->list; d; d = d->alloc_next) {
		for (l = d->succlist; l; l = l->next) {
			/*	  E_INFO("l->ascr %d\n",l->ascr); */
			
			/* 20040909: I change this from >= to > because s3.5 sometimes generate lattice which has 0 as the beginning node of the lattice.  This should be regarded as temporary change*/
			if (l->ascr > 0x10000000){
				return -1;
			}
		}
	}
	
	return 0;
}

static void dump_lat_htk(char * utt);
/*
 * Final global best path through DAG constructed from the word lattice.
 * Assumes that the DAG has already been constructed and is consistent with the word
 * lattice.
 * The search uses a recursive algorithm to find the best (reverse) path from the final
 * DAG node to the root:  The best path from any node (beginning with a particular link L)
 * depends on a similar best path for all links leaving the endpoint of L.  (This is
 * sufficient to handle trigram LMs.)
 */
s3_dag_srch_hyp_t *s3dag_dag_search (char *utt)
{
	daglink_t *l, *bestl;
	dagnode_t *d, *final;
	int32 bestscore;
	int32 k;
	
	/* Free any old hypothesis */
	hyp_free ();
	
	/*
	 * Set limit on max LM ops allowed after which utterance is aborted.
	 * Limit is lesser of absolute max and per frame max.
	 */
	maxlmop = *((int32 *) cmd_ln_access ("-maxlmop"));
	k = *((int32 *) cmd_ln_access ("-maxlpf"));
	k *= dag[le_meta].nfrm;
	if (maxlmop > k)
		maxlmop = k;
	lmop = 0;
	nb_prune_dyn =0;
	/* Find the backward link from the final DAG node that has the best path to root */
	final = dag[le_meta].exit.node;
	bestscore = (int32) 0x80000000;
	bestl = NULL;
	
	/* Check that all edge scores are -ve; error if not so. */
	if (dag_chk_linkscr (dag+le_meta) < 0){
		E_ERROR("Some edges are not negative\n");
		return NULL;
	}
	if (gram==4)  { 
		dag_dodagmeta(1);
		if (lm3g)
			do_map_tg(lm3g,utt);
		else {
			E_WARN("USING trigrams of 4G LM to compute word posteriors !!\n");
			do_map_tg(lm,utt);
		}
	}
	if(gram==3) {
		for (l = final->predlist; l; l = l->next) {
			
			d = l->node;
			if (! filler_word (d->wid)) {	/* Ignore filler node */
				/* Best path to root beginning with l */
				if (dag_bestpath (l, final) < 0) {
					E_ERROR("%s: Max LM ops (%d) exceeded\n", utt, maxlmop);
					bestl = NULL;
					break;
				}
				
				if (l->pscr > bestscore) {
					bestscore = l->pscr;
					bestl = l;
				}
			}
		}
	}else {assert (gram==4);
		for (l=dag[1].exit.next ; l ; l = l->bypass) // exit edges link by bypass a meta
		{  if (dag_bestpath_meta (l) < 0) {
			E_ERROR("%s: Max LM ops (%d) exceeded\n", utt, maxlmop);
			bestl = NULL;
			break;
		}
			
			if (l->pscr > bestscore) {
				bestscore = l->pscr;
				bestl = l;
			}
		}
		if (!bestl){
			E_ERROR("Bestpath meta search failed for %s  :%d \n", utt,nb_prune_dyn);
			do_freemeta(1);
			return NULL;
		}
		bestl = dag_meta_to_norm(bestl);   
	}
	if (! bestl) {
		E_ERROR("Bestpath search failed for %s %d \n", utt,nb_prune_dyn);
		do_freemeta(1);
		return NULL;
		
	}
	
	/*
	 * At this point bestl is the best (reverse) link/path leaving the final node.  But
	 * this does not include the acoustic score for the final node itself.  Add it.
	 */
	l = &(dag[le_meta].exit);
	l->history = bestl;
	l->pscr += bestl->pscr + l->ascr;
	l->ef = dag[le_meta].nfrm - 1;
	if (gram==4 && ! dumplattice) do_freemeta(1);
	/* Backtrack through DAG for best path */
	hyp = dag_backtrace (l);
	assert(hyp);
	if (dumplattice && gram==4) { 
		to_meta0();
		dump_lat_htk(utt);
		
		
		
		do_freemeta(1);}
	if (dumplattice && gram==3) {
		/* je ne sais pas si cela marche mais bon */
		dump_lat_htk(utt);
	}
	if(hyp==NULL){
		E_INFO("At this point hyp is NULL  %d \n",nb_prune_dyn);
	}else{
		E_INFO("At this point hyp it is not NULL%d\n",nb_prune_dyn);
	}
	E_INFO("lmop : %i \n",lmop); 
	return (hyp);
}
static void dump_lat_htk(char * utt) {
	const 	float32 log10003 = log(1.0003);
	float32 f32arg = *((float64 *) cmd_ln_access ("-logbase"));
	logmath_t *logm = logs3_init(f32arg, 0, 0);
	float32 fudge = cmd_ln_float32 ("-langwt");
	float32 wip = logs3(logm, cmd_ln_float32("-inspen"));
	int32 n_node, n_edges;
	int32 i;
	daglink_t *l;     
	dagnode_t *d;
	FILE *fp;
	FILE  * poub;
	int32 ispipe;
	static int prems=1;
	static char repertoire[1024],rep[1024];
	char filename[1024];
	dagnode_t **tab =ckd_calloc(dag[0].nnode,sizeof(dagnode_t *));
	
	
	if (prems) { prems=0; repertoire[0]='\0';}
	sscanf(utt,"%[^-]-%*f-%*f-%*s",rep);
	if (strcmp(rep,repertoire)!=0) {
		strcpy(repertoire,rep);
		sprintf(filename,"pour_confucius/%s/%s",(char *) cmd_ln_access ("-inlatdir"),repertoire);
		mkdir(filename,0777);
	}
	sprintf(filename,"pour_confucius/%s/%s/%s.lat.gz",(char *) cmd_ln_access ("-inlatdir"),repertoire,utt);
	if ((fp = fopen_comp (filename, "w", &ispipe)) == NULL) {
		E_ERROR("fopen_comp (%s,w) failed\n", filename);
	}
	n_node = dag[0].nnode;
	d= dag[0].list;
	for (i=0 ; i<n_node ;i++ , d=d->alloc_next) {
		tab[n_node-i-1] =d;
		d->index=n_node-i-1;
	}
	
	n_edges=0;
	if (0) poub= fopen("poubelle","w");
	for (i=0 ; i<n_node ;i++)
		for ( l= tab[i]->succlist ; l; l=l->next)
			if (( l->reci && l->reci->notpruned && ((int32) l->reci->lscr> 0xd0000000)) && l->bypass)
			{ daglink_t  *abyp, * byp = l->bypass;
				for ( ; byp ; byp=byp->bypass){abyp =byp; 
					assert (filler_word(((dagnode_t *)byp->src)->wid));
					if (byp->reci) { 
						byp->reci->notpruned=1 ;
						
						if  ( l->reci->lscr> byp->reci->lscr )   byp->reci->lscr = l->reci->lscr;
					}
				}
				
				
			}
	
	
	
	for (i=0 ; i<n_node ;i++)
		for ( l= tab[i]->succlist ; l; l=l->next)
			if (( l->reci && l->reci->notpruned && l->reci->lscr>((int32) 0xd0000000) && l->isfiller ==0 ) ||  l->isfiller==1 ) n_edges++; 
	
	fprintf(fp, "VERSION=1.0\n");
	fprintf(fp, "lmscale=%f\n", fudge);
	fprintf(fp, "wdpenalty=%f\n", log10003*wip);
	
	fprintf (fp, "N=%d\tL=%d\n", n_node, n_edges);
	d= dag[0].list;
	for (i=0 ; i<n_node ;i++) 
		
    { char *s,* p; int n; 
		s=dict_wordstr(dict,tab[i]->wid);
		p =strstr(s,"(");
		if (p!= NULL) 
			sscanf(p,"(%d)",&n);
		else n=1;
		
		fprintf (fp, "I=%d\tt=%.2f\tW=%s\tv=%d\n",i, tab[i]->sf/100.0, 
				 dict_wordstr(dict,dict_basewid(dict,tab[i]->wid)),n );
    }
	
	n_edges =0;
	for (i=0 ; i<n_node ;i++)
		for ( l= tab[i]->succlist ; l; l=l->next){
			if (0 && l->reci) fprintf(poub,"notpruned %d, filler %d , ls =%d , S=%d ,E=%d , t=%.2f W=%s %llu\n", l->reci->notpruned ,l->isfiller, l->reci->lscr,i, ((dagnode_t *)l->node)->index, tab[i]->sf/100.0, 
									  dict_wordstr(dict,dict_basewid(dict,tab[i]->wid)),(uint64)l->reci);
			if ( l->reci && l->reci->notpruned && l->reci->lscr> ((int32) 0xd0000000) && l->isfiller ==0)
			{  fprintf (fp, "J=%d\tS=%d\tE=%d\ta=%f\tl=%f\n", n_edges++, i , ((dagnode_t *)l->node)->index, log10003*( (float) l->ascr_save), l->reci->lscr == 0 ? 0 : 
						log10003*((l->reci->lscr - wip)/fudge ));
				
				// if (filler_word(tab[i]->wid ))  fprintf(stderr ," %d",n_edges-1);
			}
			else 
				if (l->isfiller==1 )
					fprintf (fp, "J=%d\tS=%d\tE=%d\ta=%f\tl=%f\n", n_edges++, i , ((dagnode_t *)l->node)->index, log10003*( (float) l->ascr_save), 
							 log10003*(( fillpen (fpen,dict_basewid (dict,((dagnode_t *)l->node)->wid)  )/fudge )) );
			
			
			
			
			
		}	
	//fclose(poub);
	ckd_free((void *) tab);
	fclose_comp (fp, ispipe);
	
	
	
}
