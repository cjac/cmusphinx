/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * search.h -- All exported search-related functions and data structures.
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
 * $Log$
 * Revision 1.4  2005/06/21  23:34:39  arthchan2003
 * Remove all dag functions. Eventually I may just want to delete the whole file as well.
 * 
 * Revision 1.2  2005/06/03 05:46:19  archan
 * Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:00  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.3  2004/12/06 10:52:01  arthchan2003
 * Enable doxygen documentation in libs3decoder
 *
 * Revision 1.2  2004/08/31 08:43:47  arthchan2003
 * Fixing _cpluscplus directive
 *
 * Revision 1.1  2004/08/09 00:17:12  arthchan2003
 * Incorporating s3.0 align, at this point, there are still some small problems in align but they don't hurt. For example, the score doesn't match with s3.0 and the output will have problem if files are piped to /dev/null/. I think we can go for it.
 *
 * Revision 1.2  2002/12/03 23:02:44  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 *
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *  		Added onlynodes argument to dag_dump().
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed fwd_sen_active to flag active senones instead of building a list
 * 		of them.
 *  
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_search().
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added function fwd_sen_active() to obtain list of active senones in
 * 		current frame.
 * 
 * 04-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBFBS_SEARCH_H_
#define _LIBFBS_SEARCH_H_

/** \file search.h
   \brief The temporary header file for sphinx 3 functions. 
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct srch_hyp_s {
    char     *word;		/* READ-ONLY item!! */
    s3wid_t   wid;
    s3frmid_t sf;
    s3frmid_t ef;
    int32     ascr;
    int32     lscr;
    int32     pscr;
    struct srch_hyp_s *next;
} srch_hyp_t;

#ifdef __cplusplus
}
#endif

#endif
