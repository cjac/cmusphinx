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
