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
 * lmset.c -- Language model set : an array of LM which used in Sphinx's application
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
 * Revision 1.4  2006/04/05  20:27:33  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.3  2006/02/24 13:38:08  arthchan2003
 * Added lm_read, it is a simple version of lm_read_advance.
 *
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert lm to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.2  2005/10/17 04:49:13  arthchan2003
 * Free resource of lm_t and lmset_t correctly.
 *
 * Revision 1.1.2.1  2005/07/17 05:23:25  arthchan2003
 * added lm_3g_dmp.c and lmset.c, split it out from lm.c to avoid overcrowding situation in it.
 *
 *
 */

#include <lm.h>
#include <wid.h>

static int32 lm_build_lmclass_info(lm_t *lm,float64 lw, float64 uw, float64 wip,int32 n_lmclass_used,lmclass_t *lmclass)
{
  int i;
  if(n_lmclass_used >0){
    lm->lmclass=(lmclass_t*) ckd_calloc(n_lmclass_used,sizeof(lmclass_t));
    for(i=0; i<n_lmclass_used ;i++)
      lm->lmclass[i]=lmclass[i];
  }else
    lm->lmclass= NULL;
  lm->n_lmclass = n_lmclass_used;

  lm->inclass_ugscore = (int32*)ckd_calloc(lm->dict_size,sizeof(int32));

  E_INFO("LM->inclass_ugscore size %d\n",lm->dict_size);
  E_INFO("Number of class used %d\n",n_lmclass_used);
  return 1;
}

lmset_t* lmset_init(char* lmfile, 
		    char* lmctlfile,
		    char* ctl_lm,
		    char* lmname,
		    char* lmdumpdir,
		    float32 lw,
		    float32 wip,
		    float32 uw,
		    dict_t *dict
		    )
{
  lmset_t* lms;
  lms=NULL;

  if(lmfile && lmctlfile)
    E_FATAL("Please only specify either -lm or -lmctlfn\n");

  if(!lmfile && !lmctlfile)
    E_FATAL("Please specify either one of -lm or -lmctlfn\n");

  if (lmfile) { /* Data structure are shared. But it is still a sore
		   point to have two interfaces for -lm and
		   -lmctlfile*/
    if(lmname!=NULL)
      lms=lmset_read_lm(lmfile,dict,lmname,lw,wip,uw,lmdumpdir);
    else
      lms=lmset_read_lm(lmfile,dict,"default",lw,wip,uw,lmdumpdir);
    if(lms==NULL)
      E_FATAL("lmset_read_lm(%s,%e,%e,%e) failed\n:",lmctlfile,lw,wip,uw);
    
  }else if (lmctlfile) {
    E_INFO("Reading LM ctl file\n");
    lms=lmset_read_ctl(lmctlfile,dict,lw,wip,uw,lmdumpdir);
    if(lms==NULL)
      E_FATAL("lmset_read_ctl(%s,%e,%e,%e) failed\n:",lmctlfile,lw,wip,uw);
  }else{
    E_FATAL("You must specify either -lm or -lmctlfn\n");
  }

  if (lms && ctl_lm == NULL) {
    char *name;
	
    if (lmname == NULL)
      name = lms->lmarray[0]->name;
    else
      name = lmname;
	
    /* Set the default LM */
    if (name)
      lmset_set_curlm_wname(lms,name);
    
    /* If this failed, then give up. */
    if (lms->cur_lm == NULL)
      E_FATAL("Failed to set default LM\n");
  }

  return lms;
}


lm_t* lmset_get_lm_widx(lmset_t *lms, int32 lmidx)
{
  assert(lms->lmarray[lmidx] && lmidx < lms->n_lm);

  return lms->lmarray[lmidx];
}


lm_t* lmset_get_lm_wname(lmset_t *lms, const char *lmname)
{
  int32 idx;

  idx=lmset_name_to_idx(lms,lmname);
  if(idx==LM_NOT_FOUND){
    E_WARN("In lmset_get_lm_wname: LM name %s couldn't be found, fall back to the default (the first) LM\n");
    idx=0;
  }
  return lmset_get_lm_widx(lms,idx);
}

void lmset_set_curlm_widx(lmset_t *lms, int32 lmidx)
{
  assert(lms->lmarray[lmidx] && lmidx < lms->n_lm);
  lms->cur_lm=lms->lmarray[lmidx];
  lms->cur_lm_idx=lmidx;
}

void lmset_set_curlm_wname(lmset_t *lms, const char *lmname)
{
  int32 idx;

  idx=lmset_name_to_idx(lms,lmname);
  if(idx==LM_NOT_FOUND){
    E_WARN("In lm_set_curlm_wname: LM name %s couldn't be found, fall back to the default (the first) LM\n");
    idx=0;
  }
  lmset_set_curlm_widx(lms,idx);
}

int32 lmset_name_to_idx(lmset_t *lms,const char *lmname)
{
  int32 i;
  for(i=0;i<lms->n_lm ;i++){
    if(!strcmp(lmname,lms->lmarray[i]->name)){
      return i;
    }
  }
  return LM_NOT_FOUND;
}

char* lmset_idx_to_name(lmset_t *lms,int32 lmidx)
{
  assert(lms->lmarray[lmidx]&& lmidx < lms->n_lm);
  return lms->lmarray[lmidx]->name;
}


void lmset_add_lm(lmset_t *lms,  
		  lm_t *lm,
		  const char* lmname
		  )
{  
  if(lms->n_lm == lms->n_alloc_lm){
    lms->lmarray= (lm_t **) ckd_realloc(lms->lmarray,(lms->n_alloc_lm+LM_ALLOC_BLOCK)*sizeof(lm_t*));
    lms->n_alloc_lm+=LM_ALLOC_BLOCK;
  }

  lms->lmarray[lms->n_lm]=lm;
  lms->n_lm+=1;
}

void lmset_delete_lm(lmset_t *lms,  
		  const char* lmname
		  )

{
  int32 idx;
  int32 i;
  idx=lmset_name_to_idx(lms,lmname);
  
  if(idx==LM_NOT_FOUND){
    E_WARN("In lmset_delete_lm, lmname %s is not found in the lmset\n",lmname);
  }
  
  for(i=idx;i<lms->n_lm-1;i++){
    lms->lmarray[i]=lms->lmarray[i+1];
  }
  lms->n_lm-=1;
}

void lmset_free(lmset_t *lms)
{
  int i;
  for(i=0;i<lms->n_lm;i++){
    ckd_free((void*) lms->lmarray[i]->name);
    lm_free(lms->lmarray[i]);
  }
  ckd_free(lms->lmarray);
  ckd_free((void*) lms);

}

lmset_t* lmset_read_lm(const char *lmfile,dict_t *dict, const char *lmname,float64 lw, float64 wip, float64 uw, const char *lmdumpdir)
{
  lmset_t *lms;

  lms=(lmset_t *) ckd_calloc(1,sizeof(lmset_t));
  lms->n_lm=1;
  lms->n_alloc_lm=1;

  /* Only allocate one single LM.  This assumes no class definition would be defined. 
   */
  lms->lmarray = (lm_t **) ckd_calloc(1,sizeof(lm_t*));
  /* 
     No need to check whether lmname exists here.
   */
  if ((lms->lmarray[0] = lm_read_advance (lmfile,lmname,lw, wip, uw, dict_size(dict),NULL,1))== NULL)
    E_FATAL("lm_read_advance(%s, %e, %e, %e %d [Arbitrary Fmt], Weighted Apply) failed\n", lmfile, lw, wip, uw, dict_size(dict));

  if(dict!=NULL) {
    assert(lms->lmarray[0]);
    if ((lms->lmarray[0]->dict2lmwid = wid_dict_lm_map (dict, lms->lmarray[0],lw)) == NULL)
      E_FATAL("Dict/LM word-id mapping failed for LM index %d, named %s\n",0,lmset_idx_to_name(lms,0));

  }else{
    E_FATAL("Dict is specified to be NULL (dict_init is not called before lmset_read_lm?), dict2lmwid is not built inside lmset_read_lm\n");
  }

  return lms;
}



/*
 * I attached the comment in Sphinx 2 here.  It specifies the restriction of the Darpa file format. 
 */

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
 */

lmset_t* lmset_read_ctl(const char *ctlfile,
			dict_t* dict,
			float64 lw, 
			float64 wip, 
			float64 uw,
			char *lmdumpdir)
{
  FILE *ctlfp;
  FILE *tmp;
  char lmfile[4096], lmname[4096], str[4096];

  lmclass_set_t lmclass_set;
  lmclass_t *lmclass, cl;
  int32 n_lmclass, n_lmclass_used;
  int32 i;
  lm_t *lm;
  lmset_t *lms=NULL;
  tmp=NULL;

  lmclass_set = lmclass_newset();
	    
  
  lms=(lmset_t *) ckd_calloc(1,sizeof(lmset_t));
  lms->n_lm=0;
  lms->n_alloc_lm=0;

  E_INFO("Reading LM control file '%s'\n",ctlfile);
	    
  ctlfp = myfopen (ctlfile, "r");

  if (fscanf (ctlfp, "%s", str) == 1) {
    if (strcmp (str, "{") == 0) {
      /* Load LMclass files */
      while ((fscanf (ctlfp, "%s", str) == 1) && (strcmp (str, "}") != 0))
	lmclass_set = lmclass_loadfile (lmclass_set, str);
		    
      if (strcmp (str, "}") != 0)
	E_FATAL("Unexpected EOF(%s)\n", ctlfile);
		    
      if (fscanf (ctlfp, "%s", str) != 1)
	str[0] = '\0';
    }
  } else
    str[0] = '\0';
	
#if 0
  tmp=myfopen("./tmp","w");
  lmclass_set_dump(lmclass_set,tmp);
  fclose(tmp);		   
#endif

  /* Fill in dictionary word id information for each LMclass word */
  for (cl = lmclass_firstclass(lmclass_set);
       lmclass_isclass(cl);
       cl = lmclass_nextclass(lmclass_set, cl)) {
    
    /*
      For every words in the class, set the dictwid correctly 
      The following piece of code replace s2's kb_init_lmclass_dictwid (cl);
      doesn't do any checking even the id is a bad dict id. 
      This only sets the information in the lmclass_set, but not 
      lm-2-dict or dict-2-lm map.  In Sphinx 3, they are done in 
      wid_dict_lm_map in wid.c.
     */
    
    lmclass_word_t w;
    int32 wid;
    for (w = lmclass_firstword(cl); lmclass_isword(w); w = lmclass_nextword(cl, w)) {
      wid = dict_wordid (dict,lmclass_getword(w));
#if 0
      E_INFO("In class %s, Word %s, wid %d\n",cl->name,lmclass_getword(w),wid);
#endif
      lmclass_set_dictwid (w, wid);
    }
  }

  /* At this point if str[0] != '\0', we have an LM filename */

  n_lmclass = lmclass_get_nclass(lmclass_set);
  lmclass = (lmclass_t *) ckd_calloc (n_lmclass, sizeof(lmclass_t));

  E_INFO("Number of LM class specified %d in file %s\n",n_lmclass,ctlfile);

  /* Read in one LM at a time */
  while (str[0] != '\0') {
    strcpy (lmfile, str);
    if (fscanf (ctlfp, "%s", lmname) != 1)
      E_FATAL("LMname missing after LMFileName '%s'\n", lmfile);
    
    n_lmclass_used = 0;
		
    if (fscanf (ctlfp, "%s", str) == 1) {
      if (strcmp (str, "{") == 0) {
	while ((fscanf (ctlfp, "%s", str) == 1) &&
	       (strcmp (str, "}") != 0)) {
	  if (n_lmclass_used >= n_lmclass){
	    E_FATAL("Too many LM classes specified for '%s'\n",
		    lmfile);
	  }

	  lmclass[n_lmclass_used] = lmclass_get_lmclass (lmclass_set,
							 str);
	  if (! (lmclass_isclass(lmclass[n_lmclass_used])))
	    E_FATAL("LM class '%s' not found\n", str);
	  n_lmclass_used++;
	}
	if (strcmp (str, "}") != 0)
	  E_FATAL("Unexpected EOF(%s)\n", ctlfile);
	if (fscanf (ctlfp, "%s", str) != 1)
	  str[0] = '\0';
      }
    } else
      str[0] = '\0';
      
    lm = (lm_t*) lm_read_advance (lmfile, lmname, lw, wip, uw, dict_size(dict),NULL,1);
    

    if(n_lmclass_used>0) {
      E_INFO("Did I enter here?\n");
      lm_build_lmclass_info(lm,lw,uw,wip,n_lmclass_used,lmclass);
    }

    if(lms->n_lm == lms->n_alloc_lm){
      lms->lmarray= (lm_t **) ckd_realloc(lms->lmarray,(lms->n_alloc_lm+LM_ALLOC_BLOCK)*sizeof(lm_t*));
      lms->n_alloc_lm+=LM_ALLOC_BLOCK;
    }

    lms->lmarray[lms->n_lm]=lm;
    lms->n_lm+=1;
    E_INFO("%d %d\n",lms->n_alloc_lm, lms->n_lm);
  }
  
  assert(lms);
  assert(lms->lmarray);
  E_INFO("No. of LM set allocated %d, no. of LM %d \n",lms->n_alloc_lm,lms->n_lm);


  if(dict!=NULL) {
    for(i=0;i<lms->n_lm;i++){
      assert(lms->lmarray[i]);
      assert(dict);
      if ((lms->lmarray[i]->dict2lmwid = wid_dict_lm_map (dict, lms->lmarray[i],lw)) == NULL)
	E_FATAL("Dict/LM word-id mapping failed for LM index %d, named %s\n",i,lmset_idx_to_name(lms,i));
    }
  }else{
    E_FATAL("Dict is specified to be NULL (dict_init is not called before lmset_read_lm?), dict2lmwid is not built inside lmset_read_lm\n");
  }


  fclose (ctlfp);
  return lms;
}
