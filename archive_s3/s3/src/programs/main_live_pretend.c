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
/********************************************************************
 * Example program to show usage of the live mode routines
 * The decoder is initialized with live_initialize_decoder()
 * Blocks of samples are decoded by live_utt_decode_block()
 * To compile an excutable compile using
 * $(CC) -I. -Isrc -Llibutil/linux -Lsrc/linux main_live_example.c -lutil -ldecoder -lm
 * from the current directory 
 * Note the include directories (-I*) and the library directories (-L*)
 *
 ********************************************************************/

#include <stdio.h>
#include <libutil/libutil.h>
#include "live.h"

#define MAXSAMPLES 	1000000

int main (int argc, char *argv[])
{
    short *samps;
    int  i, j, buflen, endutt, blksize, nhypwds, nsamp;
    char   *argsfile, *ctlfile, *indir;
    char   filename[512], cepfile[512];
    partialhyp_t *parthyp;
    FILE *fp, *sfp;


    if (argc != 4)
	    E_FATAL("\nUSAGE: %s <ctlfile> <inrawdir> <argsfile>\n",argv[0]);
    ctlfile = argv[1]; indir = argv[2]; argsfile = argv[3];

	samps = (short *) calloc(MAXSAMPLES,sizeof(short));
    blksize = 2000;

    if ((fp = fopen(ctlfile,"r")) == NULL)
	E_FATAL("Unable to read %s\n",ctlfile);

    live_initialize_decoder(argsfile);

    while (fscanf(fp,"%s",filename) != EOF){
	sprintf(cepfile,"%s/%s.raw",indir,filename);
	if ((sfp = fopen(cepfile,"rb")) == NULL)
	    E_FATAL("Unable to read %s\n",cepfile);
		nsamp = fread(samps, sizeof(short), MAXSAMPLES, sfp);
        fprintf(stdout,"%d samples in file %s.\nWill be decoded in blocks of %d\n",nsamp,cepfile,blksize);
        fflush(stdout); fclose(sfp);

        for (i=0;i<nsamp;i+=blksize){
	    buflen = i+blksize < nsamp ? blksize : nsamp-i;
	    endutt = i+blksize <= nsamp-1 ? 0 : 1;
	    nhypwds = live_utt_decode_block(samps+i,buflen,endutt,&parthyp);

	    E_INFO("PARTIAL HYP:");
	    if (nhypwds > 0)
                for (j=0; j < nhypwds; j++) fprintf(stderr," %s",parthyp[j].word);
	    fprintf(stderr,"\n");
        }
    }
    return 0;
}
