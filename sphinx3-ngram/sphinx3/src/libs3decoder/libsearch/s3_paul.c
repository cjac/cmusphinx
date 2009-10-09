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
 * astar.c -- A* DAG search to create N-best lists
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
 * 27-Feb-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check in building DAG for avoiding cycles with dagfudge.
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice and nbest files.
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxedge argument to control memory usage.
 * 		Added -maxlmop and -maxlpf options to control execution time.
 * 		Added -maxppath option to control CPU/memory usage.
 * 
 * 18-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added reporting of acoustic and LM scores for each word in each hyp.
 * 		Changed LM scores in nbest files to be unscaled (i.e., without any
 * 		language weight or insertion penalty.
 * 
 * 09-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added code in dag_link_bypass to update an existing link, if any,
 * 		instead of adding several bypass links between the same two nodes.
 * 		This reduces CPU and memory requirements considerably.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 * 
 * 28-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started, copying from nbest.c.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "s3_paul.h"
#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "logs3.h"
#include "vithist.h"
#include "pio.h"

static void *__listelem_alloc__ (int32 elemsize, char *file, int32 line);

/** 
    Macro of __listelem_alloc__
*/
#define listelem_alloc(sz)      __listelem_alloc__((sz),__FILE__,__LINE__)

/** Free link-list element of given size 
 */


/**
   Print number of allocation, numer of free operation stats 
*/


#include <err.h>
#include <ckd_alloc.h>



/*
 * A separate linked list for each element-size.  Element-size must be a multiple
 * of pointer-size.
 */
typedef struct list_s {
    char **freelist;            /* ptr to first element in freelist */
    struct list_s *next;        /* Next linked list */
    int32 elemsize;             /* #(char *) in element */
    int32 blocksize;            /* #elements to alloc if run out of free elments */
    int32 blk_alloc;            /* #Alloc operations before increasing blocksize */
    int32 n_alloc;
    int32 n_freed;
} list_t;
static list_t *head = NULL;

#define MIN_ALLOC	50      /* Min #elements to allocate in one block */


static void *
__listelem_alloc__(int32 elemsize, char *caller_file, int32 caller_line)
{
    int32 j;
    char **cpp, *cp;
    list_t *prev, *list;

    /* Find list for elemsize, if existing */
    prev = NULL;
    for (list = head; list && (list->elemsize != elemsize);
         list = list->next)
        prev = list;

    if (!list) {
        /* New list element size encountered, create new list entry */
        if ((elemsize % sizeof(void *)) != 0)
            E_FATAL
                ("List item size (%d) not multiple of sizeof(void *)\n",
                 elemsize);

        list = (list_t *) ckd_calloc(1, sizeof(list_t));
        list->freelist = NULL;
        list->elemsize = elemsize;
        list->blocksize = MIN_ALLOC;
        list->blk_alloc = (1 << 18) / (list->blocksize * sizeof(elemsize));
        list->n_alloc = 0;
        list->n_freed = 0;

        /* Link entry at head of list */
        list->next = head;
        head = list;
    }
    else if (prev) {
        /* List found; move entry to head of list */
        prev->next = list->next;
        list->next = head;
        head = list;
    }

    /* Allocate a new block if list empty */
    if (list->freelist == NULL) {
        /* Check if block size should be increased (if many requests for this size) */
        if (list->blk_alloc == 0) {
            list->blocksize <<= 1;
            list->blk_alloc =
                (1 << 18) / (list->blocksize * sizeof(elemsize));
            if (list->blk_alloc <= 0)
                list->blk_alloc = (int32) 0x70000000;   /* Limit blocksize to new value */
        }

        /* Allocate block */
        cpp = list->freelist =
            (char **) __ckd_calloc__(list->blocksize, elemsize,
                                     caller_file, caller_line);
        cp = (char *) cpp;
        for (j = list->blocksize - 1; j > 0; --j) {
            cp += elemsize;
            *cpp = cp;
            cpp = (char **) cp;
        }
        *cpp = NULL;
        --(list->blk_alloc);
    }

    /* Unlink and return first element in freelist */
    cp = (char *) (list->freelist);
    list->freelist = (char **) (*(list->freelist));
    (list->n_alloc)++;

    return (cp);
}


static void
listeelem_free_paul(void *elem, int32 elemsize)
{
    char **cpp;
    list_t *prev, *list;

    /* Find list for elemsize */
    prev = NULL;
    for (list = head; list && (list->elemsize != elemsize);
         list = list->next)
        prev = list;

    if (!list) {
        E_FATAL("Unknown list item size: %d\n", elemsize);
    }
    else if (prev) {
        /* List found; move entry to head of list */
        prev->next = list->next;
        list->next = head;
        head = list;
    }

    /*
     * Insert freed item at head of list.
     * NOTE: skipping check for size being multiple of (void *).
     */
    cpp = (char **) elem;
    *cpp = (char *) list->freelist;
    list->freelist = cpp;
    (list->n_freed)++;
}

/*
static void
linklist_stats(void)
{
    list_t *list;
    char **cpp;
    int32 n;

    E_INFO("Linklist stats:\n");
    for (list = head; list; list = list->next) {
        for (n = 0, cpp = list->freelist; cpp;
             cpp = (char **) (*cpp), n++);
        printf
            ("\telemsize %d, #alloc %d, #freed %d, #freelist %d\n",
             list->elemsize, list->n_alloc, list->n_freed, n);
    }
}
*/






















/* ---------------------------- NBEST CODE ---------------------------- */

/*
 * A node along each partial path.  Partial paths form a tree structure rooted at the
 * start node.
 */
typedef struct ppath_s {
    struct ppath_s *hist;	/* Immediately previous ppath node; NULL if none */
    struct ppath_s *lmhist;	/* Previous LM (non-filler) ppath node; NULL if none */
    dagnode_t *dagnode;		/* Dagnode (word,startfrm) represented by ppath node */
    int32 lscr;		/* LM score for this node given past history */
    int32 pscr;		/* Path score (from initial node) ending at this node startfrm */
    int32 tscr;		/* pscr + heuristic score (this node startfrm -> end of utt) */
  int32 num;
   uint32 histhash;	/* Hash value of complete history, for spotting duplicates */
    int32 pruned;	/* If superseded by another with same history and better score */
    struct ppath_s *hashnext;	/* Next node with same hashmod value */
    struct ppath_s *next;	/* Links all allocated nodes for reclamation at the end;
				   NULL if last in list */
} ppath_t;

/* Heap (sorted) partial path nodes */
typedef struct heap_s {
    ppath_t *ppath;	/* Partial path node */
    int32 nl, nr;	/* #left/right descendants from this node (for balancing tree) */
    struct heap_s *left;	/* Root of left descendant heap */
    struct heap_s *right;	/* Root of right descendant heap */
} aheap_t;

typedef struct astar_s {
  dag_t *dag;              /** DAG */
  dict_t *dict;            /** Dictionary */
  lm_t *lm;                /** Language Model */
  fillpen_t *fpen;         /** Filler word probabilities */
  ppath_t *ppath_list;     /** Complete list of allocated ppath nodes */
  int32 n_ppath;           /** #Partial paths allocated (to control memory usage) */
  int32 maxppath;          /** Max partial paths allowed before aborting */
  int32 beam;
  int32 timeForBeam ; 
  int32 besttscr;
  int32 n_pop, n_exp, n_pp;
  float32 lwf;
  float32 startTime;
  char showName[100];
  char * uttName;
  aheap_t *heap_root;
#define HISTHASH_MOD	200003  /* A prime */
  /**
   * For tracking ppath nodes with identical histories.  For rapid location of duplicates
   * keep a separate list of nodes for each (ppath_t.histhash % HISTHASH_MOD) value.
   * Two paths have identical histories (are duplicates) iff:
   * 	1. Their tail nodes have the same dagnode (same <wid,sf> value), and
   * 	2. Their LM histories are identical.
   */
  ppath_t **hash_list;     /* A separate list for each hashmod value (see above) */
} astar_t;



/*
 * Insert the given new ppath node in sorted (sub)heap rooted at the given heap node
 * Heap is sorted by tscr (i.e., total path score + heuristic score to end of utt).
 * Return the root of the new, updated (sub)heap.
 */
static aheap_t *aheap_insert (aheap_t *root, ppath_t *new)
{
    aheap_t *h;
    ppath_t *pp;
    
    if (! root) {
	h = (aheap_t *) listelem_alloc (sizeof(aheap_t));
	h->ppath = new;
	h->left = h->right = NULL;
	h->nl = h->nr = 0;
	return h;
    }

    /* Root already exists; if new node better, replace root with it */
    pp = root->ppath;
    if (pp->tscr < new->tscr) {
	root->ppath = new;
	new = pp;
    }

    /* Insert new or old (replaced) node in right or left subtree; keep them balanced */
    if (root->nl > root->nr) {
	root->right = aheap_insert (root->right, new);
	root->nr++;
    } else {
	root->left = aheap_insert (root->left, new);
	root->nl++;
    }

    return root;
}


/*
 * Pop the top element off the heap and return root of adjust heap.
 * Root must be non-NULL when this function to be called.
 */
static aheap_t *aheap_pop (aheap_t *root)
{
    aheap_t *l, *r;

    /* Propagate best value from below into root, if any */
    l = root->left;
    r = root->right;
    if (! l) {
	if (! r) {
	    listeelem_free_paul ((char *) root, sizeof(aheap_t));
	    return NULL;
	} else {
	    root->ppath = r->ppath;
	    root->right = aheap_pop (r);
	    root->nr--;
	}
    } else {
	if ((! r) || (l->ppath->tscr >= r->ppath->tscr)) {
	    root->ppath = l->ppath;
	    root->left = aheap_pop (l);
	    root->nl--;
	} else {
	    root->ppath = r->ppath;
	    root->right = aheap_pop (r);
	    root->nr--;
	}
    }

    return root;
}


/*
 * Check if pplist already contains a better (better pscr) path identical to the
 * extension of lmhist by node.  Return 1 if true, 0 if false.  Also, if false, but
 * an inferior path did exist, mark it as pruned.
 */
static ppath_t * ppath_dup (astar_t *astar,ppath_t *hlist, ppath_t *lmhist, dagnode_t *node,
			uint32 hval, int32 pscr)
{int count;
    ppath_t *h1, *h2;
        /* Compare each entry in hlist to new, proposed path */
    for (; hlist; hlist = hlist->hashnext) {
	if ((hlist->dagnode != node) || (hlist->histhash != hval))
	    continue;
	count=dict_filler_word(astar->dict,node->wid)? 3 :2;
	for (h1 = hlist->lmhist, h2 = lmhist;count>0&& h1 && h2; h1 = h1->lmhist, h2 = h2->lmhist, count--) {
	    if ((h1 == h2) ||	/* Histories converged; identical */
		(dict_basewid (astar->dict, h1->dagnode->wid) != dict_basewid (astar->dict, h2->dagnode->wid)))
		break;
	}

	if (h1 == h2|| count==0) {	    /* Identical history already exists */
	    if (hlist->pscr >= pscr)	/* Existing history is superior */
		return hlist;
	    else {
		/*
		 * New path is better; prune existing one.  There may be other paths
		 * in the list as well, but all of them must have been pruned by
		 * hlist or others later in the list!
		 */
		hlist->pruned = 1;
		return hlist;
	    }
	}
    }

    return NULL;
}
static int32 final=-1;
static int32 edgeCount=0,nodeCount=0;
/*
 * Create a new ppath node for dagnode reached from top via link l.  Assign the
 * proper scores for the new, extended path and insert it in the sorted heap of
 * ppath nodes.  But first check if it's a duplicate of an existing ppath but has an
 * inferior score, in which case do not insert.
 */
static void ppath_insert (astar_t * astar,ppath_t *top, daglink_t *l, int32 pscr, int32 tscr, int32 lscr,
			  FILE* fp){
  ppath_t *pp, *lmhist,*prec,*loop;
  uint32 h, hmod;
  /*  s3wid_t w1,w2;*/
  s3wid_t w[4];
  int32 compte;
  int32 num;
  /* Extend path score; Add acoustic and LM scores for link */
  pscr = top->pscr + l->ascr + lscr;
    
  /*
   * Check if extended path would be a duplicate one with an inferior score.
   * First, find hash value for new node.
   */
  /* passage rn quadri ici pour la fonction de hachage paul */
  lmhist = dict_filler_word(astar->dict,top->dagnode->wid) ? top->lmhist : top;
  compte=0;
  if (dict_filler_word(astar->dict,l->node->wid)) {
    /* w1=lmhist ? dict_basewid(astar->dict,lmhist->dagnode->wid) :BAD_S3WID;
       w2= lmhist && lmhist->lmhist ? dict_basewid(astar->dict,lmhist->lmhist->dagnode->wid) :BAD_S3WID;*/
  }
  else{
    w[compte++]=dict_basewid(astar->dict,l->node->wid);
    /*w1= dict_basewid(astar->dict,l->node->wid);
      w2=lmhist ? dict_basewid(astar->dict,lmhist->dagnode->wid) :BAD_S3WID;*/
  }
  loop=lmhist;
  for( loop=lmhist ; loop  && compte<3; loop=loop->lmhist,compte++) 
    w[compte]= dict_basewid(astar->dict,loop->dagnode->wid);
  for( ; compte<3; compte++)
    w[compte]=BAD_S3WID;

  h = *(uint32 *)l->node;
  h = (h >> 5) | (h << 27);
  h=w[0]+ 37*w[1]+ 127*w[2];
  /*     w = lmhist->dagnode->wid;
	 h = lmhist->histhash - w + dict_basewid (dict, w);
	 h = (h >> 5) | (h << 27);
	 h += l->node->wid;*/
  hmod = h % HISTHASH_MOD;
    
  /* If new node would be an inferior duplicate, skip creating it */
  prec=ppath_dup (astar,astar->hash_list[hmod], lmhist, l->node, h, pscr);
  if (prec) 
    num=prec->num; 
  else { 
    char *s,* p; int version; 
    if (l->node==astar->dag->end) {
      if (final <0) {
	final=nodeCount++;
	if (fp) fprintf(fp,"I=%d\tt=%.2f\tW=%s\tv=%d\n",
		final,l->node->sf/100.0,
		dict_wordstr(astar->dict,dict_basewid(astar->dict,l->node->wid)),
		1 );
      }  
      num =final;
    }
    else
      {
	num= nodeCount++;
	s=dict_wordstr(astar->dict,l->node->wid);
	p =strstr(s,"(");
	if (p!= NULL) 
	  sscanf(p,"(%d)",&version);
	else version=1;
	if (fp) fprintf(fp,"I=%d\tt=%.2f\tW=%s\tv=%d\n",
		num,l->node->sf/100.0,
		dict_wordstr(astar->dict,dict_basewid(astar->dict,l->node->wid)),
		version );
      }
  }
  if (fp) fprintf(fp,"J=%d\tS=%d\tE=%d\ta=%f\tl=%f\n",edgeCount++,top->num,
	  num, logmath_log_to_ln(astar->lm->logmath, l->ascr), logmath_log_to_ln(astar->lm->logmath, lm_rawscore (astar->lm,lscr)));
	
  /* Add heuristic score from END OF l until end of utterance */
  tscr = pscr + l->hscr;


  /* Initialize new partial path node */
  if (prec && !prec->pruned) return;
  pp = (ppath_t *) listelem_alloc (sizeof(ppath_t));
  pp->num=num;
  pp->dagnode = l->node;
  pp->hist = top;
  pp->lmhist = lmhist;
  pp->lscr = lscr;
  pp->pscr = pscr;
  pp->tscr = tscr;
  pp->histhash = h;
  pp->hashnext = astar->hash_list[hmod];
  astar->hash_list[hmod] = pp;
  pp->pruned = 0;

  pp->next = astar->ppath_list;
  astar->ppath_list = pp;
    
  astar->heap_root = aheap_insert (astar->heap_root, pp);
    
  astar->n_ppath++;
}


static int32 ppath_free ( astar_t *astar )
{
    ppath_t *pp;
    int32 n;
    
    n = 0;
    while (astar->ppath_list) {
	pp = astar->ppath_list->next;
	listeelem_free_paul ((char *) astar->ppath_list, sizeof(ppath_t));
	astar->ppath_list = pp;
	n++;
    }
    
    return n;
}

static void ppath_seg_write (FILE *fp, ppath_t *pp, int32 ascr,astar_t *astar)
{   int32 lscr_base;
    int bon=1;
    if (pp->num>=0) return;
    if (0 && pp->hist)
      ppath_seg_write (fp, pp->hist, pp->pscr - pp->hist->pscr - pp->lscr,astar);

    lscr_base = pp->hist ? lm_rawscore (astar->lm, pp->lscr) : 0;
    if ( pp->dagnode->wid==dict_finishwid(astar->dict)){
      if (final==-1) 
	final= pp->num= nodeCount++;
      else
	{pp->num=final;bon=0;}
    }
    else
    pp->num= nodeCount++;
    if (0) fprintf (stdout, " %d %d %d %s",
	     pp->dagnode->sf, ascr,lscr_base,dict_wordstr (astar->dict, pp->dagnode->wid));
    
    if (bon)
    { char *s,* p; int version; 
      s=dict_wordstr(astar->dict,pp->dagnode->wid);
      p =strstr(s,"(");
      if (p!= NULL) 
	sscanf(p,"(%d)",&version);
      else version=1;
      if (fp) fprintf(fp,"OI=%d\tt=%.2f\tW=%s\tv=%d\n",
	      pp->num,pp->dagnode->sf/100.0,dict_wordstr(astar->dict,dict_basewid(astar->dict,pp->dagnode->wid)),version );
      
      
    }
    if (pp->hist)    if (fp) fprintf(fp,"J=%d\tS=%d\tE=%d\ta=%f\tl=%f\n",edgeCount++,pp->hist->num,
			     pp->num,logmath_log_to_ln(astar->lm->logmath, pp->pscr - pp->hist->pscr - pp->lscr),logmath_log_to_ln(astar->lm->logmath, lscr_base));
}

static void imprimer(FILE *fp, ppath_t *pp,int ascr,astar_t * astar,int fin);
 void s3paul_imprimer(FILE *fp, void *pp,int ascr,void * astar,int fin) {

   /*je sais c'est risque mais bon a la guerre comme a la guerre */
  imprimer(fp,
	   (ppath_t *)  pp, 
	   ascr, 
	   (astar_t *) astar,
	   fin );
 }


 static void imprimer(FILE *fp, ppath_t *pp,int ascr,astar_t * astar,int fin) {
  if (pp->hist)
    imprimer(fp,pp->hist,pp->pscr - pp->hist->pscr - pp->lscr,
	     astar,pp->dagnode->sf);
  fprintf(fp,"%s 1 %.2f %.2f %s %s %d %d %d\n",
	  astar->showName,
	  astar->startTime+pp->dagnode->sf/100.0,
	  (fin-pp->dagnode->sf)/100.0,
	  dict_wordstr (astar->dict, pp->dagnode->wid), 
	  astar->uttName,pp->pscr, ascr,
	  (pp->hist ? lm_rawscore (astar->lm, pp->lscr) : 0));
}

static void nbest_hyp_write (FILE *fp, ppath_t *top, int32 pscr, int32 nfr,astar_t *astar)
{
    int32 lscr, lscr_base;
    ppath_t *pp;
    
    lscr_base = 0;
    for (lscr = 0, pp = top; pp; lscr += pp->lscr, pp = pp->hist) {
	if (pp->hist)
	    lscr_base += lm_rawscore (astar->lm, pp->lscr);
	else
	    assert (pp->lscr == 0);
    }

    if (0) fprintf (stdout, "T %d A %d L %d", pscr, pscr - lscr, lscr_base);

    ppath_seg_write (fp, top, pscr - top->pscr,astar);

    if (0) fprintf (stdout, " %d\n", nfr);
   
}
static astar_t *
astar_init(dag_t *dag, dict_t *dict, lm_t *lm, fillpen_t *fpen, float64 beam, float64 lwf)
{
    astar_t *astar;
    ppath_t *pp;
    int i;
    edgeCount=0;
    nodeCount=0;
    astar = ckd_calloc(1, sizeof(*astar));
    astar->dag = dag;
    astar->dict = dict;
    astar->lm = lm;
    astar->fpen = fpen;
    astar->lwf = lwf;
    E_INFO(" je veux voir %e\n",beam);
    astar->beam = logs3(lm->logmath, beam);
    astar->heap_root = NULL;
    astar->ppath_list = NULL;
    astar->hash_list = (ppath_t **) ckd_calloc(HISTHASH_MOD, sizeof(ppath_t *));

    for (i = 0; i < HISTHASH_MOD; i++)
        astar->hash_list[i] = NULL;

    /* Set limit on max #ppaths allocated before aborting utterance */
    astar->maxppath = cmd_ln_int32("-maxppath");
    astar->n_ppath = 0;

    /* Insert start node into heap and into list of nodes-by-frame */
    pp = (ppath_t *) listelem_alloc( sizeof(*pp));
    pp->dagnode = dag->root;
    pp->hist = NULL;
    pp->lmhist = NULL;
    pp->lscr = 0;
    pp->pscr = 0;
    pp->tscr = 0;               /* HACK!! Not really used as it is popped off rightaway */
    pp->histhash = pp->dagnode->wid;
    pp->hashnext = NULL;
    pp->pruned = 0;
    pp->num=nodeCount++;
    pp->next = NULL;
    astar->ppath_list = pp;

    /* Insert into heap of partial paths to be expanded */
    astar->heap_root = aheap_insert(astar->heap_root, pp);

    /* Insert at head of (empty) list of ppaths with same hashmod value */
    astar->hash_list[pp->histhash % HISTHASH_MOD] = pp;

    astar->n_pop = astar->n_exp = astar->n_pp = 0;
    astar->besttscr = (int32) 0x80000000;

    return astar;
}


void nbest_dag_htk (dag_t *dag, char *filename, char *uttid, float64 lwf,
		   dict_t *dict, lm_t *lm, fillpen_t *fpen ,FILE * ctmfp)
{
    FILE *fp = NULL;
    float32 f32arg;
    int32 nbest_max, n_pop, n_exp, n_hyp, n_pp;
    int32 besthyp, worsthyp, besttscr;
    ppath_t *top=NULL, *pp,*lebest=NULL;
    dagnode_t *d;
    daglink_t *l;
    int32 lscr, pscr, tscr;
    s3wid_t bwm1,bw0, bw1, bw2;
    int32 ispipe;
    int32 ppathdebug=cmd_ln_boolean("-ppathdebug");
    astar_t *astar;
    float64 v;
    s3lmwid32_t lwid[4];
    if (cmd_ln_exists("-beamastar"))
      v= cmd_ln_float64("-beamastar");
    else 
      v=cmd_ln_float64("-beam"); 
    E_INFO("biz %e\n",v);
    astar = astar_init(dag, dict, lm, fpen,v, lwf);

    /* Create Nbest file and write header comments */
    if (filename)
    if ((fp = fopen_comp (filename, "w", &ispipe)) == NULL) {
	E_ERROR("fopen_comp (%s,w) failed\n", filename);
	fp = stdout;
    }
    if (fp) fprintf(fp, "VERSION=1.0\n");
    if (fp) fprintf(fp, "lmscale=%f\n", cmd_ln_float32("-lw") *lwf);
    f32arg = cmd_ln_float32 ("-wip");
    if (fp) fprintf(fp, "wdpenalty=%f\n", f32arg);
  
    final=-1;
    

sscanf(uttid,"%[^-]-%f",astar->showName,&(astar->startTime));
    astar->uttName=uttid;
    
    
    /*
     * Set limit on max LM ops allowed after which utterance is aborted.
     * Limit is lesser of absolute max and per frame max.
     */
   
    
    /* Set limit on max #ppaths allocated before aborting utterance */
    
    pp=astar->heap_root->ppath;    
       if (fp) fprintf(fp,"I=%d\tt=%.2f\tW=%s\tv=%d\n",
	    pp->num,pp->dagnode->sf/100.0,dict_wordstr(dict,dict_basewid(dict,pp->dagnode->wid)),1);
    
    /* Astar-search */
    n_hyp = n_pop = n_exp = n_pp = 0;
    nbest_max = *((int32 *) cmd_ln_access ("-nbest"));
    besthyp = besttscr = (int32)0x80000000;
    worsthyp = (int32)0x7fffffff;
    
    while ((n_hyp < nbest_max) && astar->heap_root) {
	/* Extract top node from heap */
	top = astar->heap_root->ppath;
	astar->heap_root = aheap_pop (astar->heap_root);
	
	n_pop++;
	
	if (top->pruned)
	    continue;
	/* j'ai les plus grand doute sur final.ascr car final is after  end ????? */	
	if (top->dagnode == dag->end) {	/* Complete hypotheses; output */
	  nbest_hyp_write (fp, top, top->pscr + dag->final.ascr, dag->nfrm,astar);
	    n_hyp++;
	    if (besthyp < top->pscr)
	      {besthyp = top->pscr;
		lebest=top;
	      }
	    if (worsthyp > top->pscr)
		worsthyp = top->pscr;
	    
	    continue;
	}
	/* passage en quadri doit se faire ici Paul */
	/* Find two word (trigram) history beginning at this node */
	pp = (dict_filler_word (dict,top->dagnode->wid)) ? top->lmhist : top;
	bwm1=BAD_S3WID;
	if (pp) {
	    bw1 = dict_basewid(dict, pp->dagnode->wid);
	    pp = pp->lmhist;
	    bw0 = pp ? dict_basewid(dict, pp->dagnode->wid) : BAD_S3WID;
	    if (pp) {
	      pp=pp->lmhist;
	      bwm1=pp ? dict_basewid(dict, pp->dagnode->wid) : BAD_S3WID;
	    }
	} else
	    bw0 = bw1 = BAD_S3WID;
	lwid[0] = (bwm1 == BAD_S3WID) ? BAD_LMWID(lm) : lm->dict2lmwid[bwm1];
        lwid[1] = (bw0 == BAD_S3WID) ? BAD_LMWID(lm) : lm->dict2lmwid[bw0];
        lwid[2] = (bw1 == BAD_S3WID) ? BAD_LMWID(lm) : lm->dict2lmwid[bw1];


	/* Expand to successors of top (i.e. via each link leaving top) */
	d = top->dagnode;
	for (l = d->succlist; l; l = l->next) {
	  assert (l->node->reachable && (! l->bypass)); /* je n'utilise pas les bypass */ 

	    /* Obtain LM score for link */
	    bw2 = dict_basewid (dict, l->node->wid);
	    if (dict_filler_word (dict,bw2)) lscr= fillpen(fpen, bw2) ;
	    else { 
	      lwid[3] =lm->dict2lmwid[bw2];
	      lscr=lwf * lm_ng_score(lm, 4, lwid, bw2);
	    }
	    if (astar->dag->lmop++ > astar->dag->maxlmop) {
		E_ERROR("%s: Max LM ops (%d) exceeded\n", uttid, astar->dag->maxlmop);
		break;
	    }
	    
	    /* Obtain partial path score and hypothesized total utt score */
	    pscr = top->pscr + l->ascr + lscr;
	    tscr = pscr + l->hscr;

	    if (ppathdebug) {
		printf ("pscr= %11d, hscr= %11d, tscr= %11d, lscr=%7d, sf= %5d, %s%s %d\n",
			pscr, l->hscr, tscr,lscr, l->node->sf, dict_wordstr(dict, l->node->wid),
			(tscr-astar->beam >= besttscr) ? "" : " (pruned)",besttscr);
	    }
	    
	    /* Insert extended path if within beam of best so far */
	    if (tscr -astar->beam >= besttscr) {
	      ppath_insert (astar,top, l, pscr, tscr, lscr,fp);
		if (astar->n_ppath > astar->maxppath) {
		    E_ERROR("%s: Max PPATH limit (%d) exceeded\n", uttid, astar->maxppath);
		    break;
		}
		
		if (tscr > besttscr)
		    besttscr = tscr;
	    }
	}
	if (l)	/* Above loop was aborted */
	    break;
	
	n_exp++;
    }
    imprimer(ctmfp,lebest, dag->final.ascr,astar,lebest->dagnode->sf);

    fprintf (stderr, "%s End; best %d worst %d diff %d beam %d\n",uttid,
	     besthyp + dag->final.ascr, worsthyp + dag->final.ascr, worsthyp - besthyp, astar->beam);
    if (fp) fprintf(fp,"start=0\tend=%d\n",final);
    if (fp) fprintf(fp,"N=%d\tL=%d\n",nodeCount,edgeCount);
    fprintf(stderr,"N=%d\tL=%d\tF=%d\n",nodeCount,edgeCount,final);
    if (fp) fclose_comp (fp, ispipe);



    if (n_hyp <= 0) {
	unlink (filename);
	E_ERROR("%s: A* search failed\n", uttid);
    }
    
    /* Free partial path nodes and any unprocessed heap */
    while (astar->heap_root)
	astar->heap_root = aheap_pop(astar->heap_root);

    n_pp = ppath_free (astar);
    ckd_free(astar->heap_root);
    ckd_free(astar->hash_list);
    if (0) {

    lm_cache_stats_dump(astar->lm);
    lm_cache_reset(astar->lm);
    }
    ckd_free(astar);
    E_INFO ("CTR(%s): %5d frm %4d hyp %6d pop %6d exp %8d pp\n",
	    uttid, dag->nfrm, n_hyp, n_pop, n_exp, n_pp);
}

