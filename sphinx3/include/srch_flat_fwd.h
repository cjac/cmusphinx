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

/*
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:16:14  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Add wrapper of flat_fwd.c
 *
 * Revision 1.1.2.5  2006/01/16 20:11:23  arthchan2003
 * Interfaces for 2nd stage search, now commented.
 *
 * Revision 1.1.2.4  2005/11/17 06:42:15  arthchan2003
 * Added back crossword triphone traversing timing for search. Also. for consistency with srch.c.  Some dummy code of IBM lattice conversion was added. They are now bypassed because it is not fully function.
 *
 * Revision 1.1.2.3  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.1.2.2  2005/09/18 01:45:19  arthchan2003
 * Filled in all implementation in srch_flat_fwd.[ch], like the FSG mode, it takes care of reporting itselft.
 *
 * Revision 1.1.2.1  2005/07/24 01:40:37  arthchan2003
 * (Incomplete) The implementation of flat-lexicon decoding.
 *
 *
 *
 */

/* \file srch_flat_fwd.h
 * 
 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 *
 */

#ifndef SRCH_FLT_FWD
#define SRCH_FLT_FWD

extern struct srch_funcs_s srch_FLAT_FWD_funcs;

#endif /* SRCH_FLT_FWD*/
