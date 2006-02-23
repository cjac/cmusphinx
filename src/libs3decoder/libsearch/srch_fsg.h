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

/* srch_fsg.h
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2006/02/23  05:11:38  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Adaptation of Sphinx 2's FSG search into Sphinx 3
 * 
 * Revision 1.1.2.7  2006/01/16 20:12:42  arthchan2003
 * Interfaces for the 2nd-stage search. Now commented.
 *
 * Revision 1.1.2.6  2005/09/18 01:45:43  arthchan2003
 * Clean up the interfaces for srch_fsg.[ch]
 *
 * Revision 1.1.2.5  2005/07/26 02:20:39  arthchan2003
 * merged hyp_t with srch_hyp_t.
 *
 * Revision 1.1.2.4  2005/07/24 01:41:18  arthchan2003
 * Filled in all function pointers for mode 2
 *
 * Revision 1.1.2.3  2005/07/20 21:18:30  arthchan2003
 * FSG can now be read, srch_fsg_init can now be initialized, psubtree can be built. Sounds like it is time to plug in other function pointers.
 *
 * Revision 1.1.2.2  2005/06/28 07:01:21  arthchan2003
 * General fix of fsg routines to make a prototype of fsg_init and fsg_read. Not completed.  The number of empty functions in fsg_search is now decreased from 35 to 30.
 *
 * Revision 1.1.2.1  2005/06/27 05:27:09  arthchan2003
 * Fixed added srch_fsg.[ch] and replace it by srch_fsm.c
 *
 */

#include <s3types.h>

#include "kb.h"
#include "word_fsg.h"
#include "fsg_search.h"
#include "cmd_ln.h"


int srch_FSG_init(kb_t *kb, /**< The KB */
		  void* srch_struct /**< The pointer to a search structure */
		  );

word_fsg_t* srch_FSG_read_fsgfile(void* srch_struct,const char* fsgname);

int srch_FSG_uninit(void* srch_struct);
int srch_FSG_begin(void* srch_struct);
int srch_FSG_end(void* srch_struct);

glist_t srch_FSG_gen_hyp(void* srch_struct);
int srch_FSG_dump_vithist(void* srch_struct);

int srch_FSG_set_lm(void* srch_struct, const char* lmname);
int srch_FSG_add_lm(void* srch, lm_t *lm, const char *lmname);
int srch_FSG_delete_lm(void* srch, const char *lmname);

int srch_FSG_srch_one_frame_lv2(void* srch_struct);

int srch_FSG_shift_one_cache_frame(void *srch_struct,int32 win_efv);
int srch_FSG_select_active_gmm(void *srch_struct);

int srch_FSG_windup(void* srch_struct, int32 frmno);

