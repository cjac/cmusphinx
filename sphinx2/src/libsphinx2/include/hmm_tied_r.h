/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 *
 * $Log$
 * Revision 1.8  2004/12/10  16:48:58  rkm
 * Added continuous density acoustic model handling
 * 
 * 
 */


#ifndef __S2_HMM_TIED_R_H__
#define __S2_HMM_TIED_R_H__	1


int32 hmm_num_sseq(void);
int32 hmm_pid2sid (int32 pid);

void hmm_tied_read_bin (char const *dir_list,
			char const *file,
			SMD *smd,
			double transSmooth,
			int32 numAlphaExpected,
			int norm,
			double arcWeight);

void hmm_tied_read_big_bin (char const  *dir_list,
			    char const  *file,
			    SMD   *smds,
			    double transSmooth,
			    int32  numAlphaExpected,
			    int    norm,
			    double arcWeight);

void read_dists (char const *distDir,
		 char const *Code_Ext0, char const *Code_Ext1,
		 char const *Code_Ext2, char const *Code_Ext3,
		 int32 numAlphabet,
		 double SmoothMin,
		 int32 useCiDistsOnly);

void readDistsOnly (
	char const *distDir,
	char const *Code_Ext0, char const *Code_Ext1,
	char const *Code_Ext2, char const *Code_Ext3,
	int32 numAlphabet,
	int32 useCiDistsOnly);

void read_map (char const *map_file, int32 compress);
void remap (SMD *smdV);

int32 senid2pid (int32 senid);
int32 *hmm_get_psen (void);


#endif
