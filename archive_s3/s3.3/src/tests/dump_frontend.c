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


/********************************************************************
 *
 * This program dumps all the intermediate data between the frontend
 * processors: the audio frames, the preemphasized audio,
 * the windowed audio data (after applying Hamming window),
 * the spectrum, the melspectrum, the melcepstrum, and the features.
 * At the end, the amount of time used by each frontend processor
 * is displayed.
 *
 * This program is basically a replica of
 *
 * src/programs/main_live_pretend.c
 *
 * but with dumping and timing code inserted instead. Therefore, its
 * usage is basically the same:
 *
 * dumpfrontend <control file> <audio files directory> <ARGS file>
 * <dump file>
 *
 * control file - the usual .ctl file where list the audio files
 * audio files directory - where all the audio files are
 * ARGS file - file containing Sphinx 3 command line arguments
 * dump file - the file to dump the results
 *
 ********************************************************************/

#include <stdio.h>
#include <libutil/libutil.h>
#include <libs3decoder/new_fe.h>
#include "live_dump.h"
#include "fe_dump.h"
#include "metrics.h"

#define MAXSAMPLES 	1000000

int main (int argc, char *argv[])
{
    short *samps;
    int  i, buflen, endutt, blksize, nhypwds, nsamp;
    char   *argsfile, *ctlfile, *indir, *dumpfile;
    char   filename[512], cepfile[512];
    partialhyp_t *parthyp;
    FILE *fp, *sfp;
    
    
    if (argc != 5) {
        E_FATAL("\nUSAGE: %s <ctlfile> <inrawdir> <argsfile> <frontend_dumpfile>\n",argv[0]);
    }
    ctlfile = argv[1]; indir = argv[2]; argsfile = argv[3]; dumpfile = argv[4];
    
    /* open dump file */
    if ((fe_dumpfile = fopen(dumpfile, "w")) == NULL) {
        E_FATAL("Unable to open %s\n", dumpfile);
        exit(1);
    }
    
    samps = (short *) calloc(MAXSAMPLES,sizeof(short));
    blksize = 2000;
    
    if ((fp = fopen(ctlfile,"r")) == NULL)
	E_FATAL("Unable to read %s\n",ctlfile);
    
    live_initialize_decoder(argsfile);
    
    while (fscanf(fp,"%s",filename) != EOF) {

        sprintf(cepfile,"%s/%s.raw",indir,filename);

        if ((sfp = fopen(cepfile,"rb")) == NULL)
            E_FATAL("Unable to read %s\n",cepfile);

        nsamp = fread(samps, sizeof(short), MAXSAMPLES, sfp);

        fprintf(stdout,
                "%d samples in file %s.\nWill be decoded in blocks of %d\n",
                nsamp,cepfile,blksize);

        fflush(stdout); fclose(sfp);
        
        for (i=0;i<nsamp;i+=blksize){
            buflen = i+blksize < nsamp ? blksize : nsamp-i;
            endutt = i+blksize <= nsamp-1 ? 0 : 1;

            /* run the frontend */
            nhypwds = live_fe_process_block(samps+i,buflen,endutt,&parthyp);
        }
    }
    
    metricsPrint();
    
    return 0;
}
