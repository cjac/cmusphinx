/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/* srch_time_switch_tree.h
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.3  2006/02/23 16:46:50  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH:
 * 1, Used the corresponding lextree interface.
 * 2, 2nd-stage search logic are all commented.
 *
 * Revision 1.2.4.3  2006/01/16 20:14:02  arthchan2003
 * Remove the unlink silences part because that could affect the performance of the 1st pass search when -bestpath is specified.
 *
 * Revision 1.2.4.2  2005/07/07 02:38:35  arthchan2003
 * 1, Remove -lminsearch, 2 Remove rescoring interface in the header.
 *
 * Revision 1.2.4.1  2005/07/04 07:20:48  arthchan2003
 * 1, Ignored -lmsearch, 2, cleaned up memory, 3 added documentation of TST search.
 *
 * Revision 1.2  2005/06/22 02:45:52  arthchan2003
 * Log. Implementation of word-switching tree. Currently only work for a
 * very small test case and it's deliberately fend-off from users. Detail
 * omitted.
 *
 * Revision 1.8  2005/05/11 00:18:46  archan
 * Add comments on srch.h and srch_time_switch_tree.h and srch_debug.h on how things work. A very detail comment is added in srch.h to describe how generally srch_t is interacting with other parts of the code.
 *
 * 
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. Time switching tree implementation. 
 */


#ifndef _SRCH_TST_H_
#define _SRCH_TST_H_

extern struct srch_funcs_s srch_TST_funcs;

#endif






