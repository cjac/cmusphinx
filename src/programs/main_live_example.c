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
#include <libutil/libutil.h>
#include <stdio.h>
#include "live.h"
#include "ad.h"

#define MAXSAMPLES 	10000
#define LISTENTIME	5.0
#define MAX_RECORD      48000

static int32 last_fr;		/* Last frame for which partial result was reported */
static ad_rec_t *ad;

/* Determine if the user has indicated end of utterance (keyboard hit at end of utt) */

static void ui_ready ( void ){
#if (WIN32)
    printf ("\nSystem will listen for ~ %.1f sec of speech\n", LISTENTIME);
    printf ("Hit <cr> before speaking: ");
#else
    printf ("\nHit <cr> BEFORE and AFTER speaking: ");
#endif
    fflush (stdout);
}


/* Main utterance processing loop: decode each utt */
static void utterance_loop()
{
    char line[1024];
    int16 adbuf[4096];
    int32 k;
    int32 ns;		/* #Samples read from audio in this utterance */
    int32 hwm;		/* High Water Mark: to know when to report partial result */
    int32 recording;

    int  j,nhypwds;
    partialhyp_t *parthyp;
    
    for (;;) {		/* Loop for each new utterance */
	ui_ready ();

	fgets (line, sizeof(line), stdin);
	if ((line[0] == 'q') || (line[0] == 'Q'))
	    return;
	
	ad_start_rec(ad);	/* Start A/D recording for this utterance */
	recording = 1;

	ns = 0;
	hwm = 4000;	/* Next partial result reported after 4000 samples */
	last_fr = -1;	/* Frame count at last partial result reported */

	
	/* Send audio data to decoder until end of utterance */
	for (;;) {
	    /*
	     * Read audio data (NON-BLOCKING).  Use your favourite substitute here.
	     * NOTE: In our implementation, ad_read returns -1 upon end of utterance.
	     */
	    if ((k = ad_read (ad, adbuf, 4096)) < 0)
		break;

	    // For now, record until MAX_RECORD and then shut off
	    if (ns + k > MAX_RECORD) {
	      ad_close (ad);
	      nhypwds = live_utt_decode_block(adbuf,k,1,&parthyp);
	      E_INFO("\n\nFINAL HYP:");
	      if (nhypwds > 0)
		for (j=0; j < nhypwds; j++) printf(" %s",parthyp[j].word);
	      printf("\n");
	      break;
	    }  else
	      nhypwds = live_utt_decode_block(adbuf,k,0,&parthyp);

	    /* Send whatever data was read above to decoder */
	    // uttproc_rawdata (adbuf, k, 0);
	    ns += k;

	    /* Time to report partial result? (every 4000 samples or 1/4 sec) */
	    if (ns > hwm) {
		hwm = ns+4000;
		E_INFO("PARTIAL HYP:");
		if (nhypwds > 0)
		  for (j=0; j < nhypwds; j++) printf(" %s",parthyp[j].word);
		printf("\n");
	    }
	}
    }
}


int main (int argc, char *argv[])
{
  short samps[MAXSAMPLES];
  int  i, /*j,*/ buflen, endutt, blksize, nhypwds, nsamp;
    char   *argsfile, *ctlfile, *indir;
    char   filename[512], cepfile[512];
    partialhyp_t *parthyp;
    FILE *fp, *sfp;


    argsfile = argv[1];
    live_initialize_decoder(argsfile);

    if (0) {
      if (argc != 4)
	E_FATAL("\nUSAGE: %s <ctlfile> <infeatdir> <argsfile>\n",argv[0]);
      ctlfile = argv[1]; indir = argv[2]; argsfile = argv[3];
      
      blksize = 2000;
      
      if ((fp = fopen(ctlfile,"r")) == NULL)
	E_FATAL("Unable to read %s\n",ctlfile);
      
      while (fscanf(fp,"%s",filename) != EOF){
	sprintf(cepfile,"%s/%s.raw",indir,filename);
	if ((sfp = fopen(cepfile,"r")) == NULL)
	  E_FATAL("Unable to read %s\n",cepfile);
	nsamp = fread(samps, sizeof(short), MAXSAMPLES, sfp);
        E_INFO("%d samples in file. Will be decoded in blocks of %d\n",nsamp,blksize);
        fclose(sfp);
	
        for (i=0;i<nsamp;i+=blksize){
	  buflen = i+blksize < nsamp ? blksize : nsamp-i;
	  endutt = i+blksize <= nsamp-1 ? 0 : 1;
	  nhypwds = live_utt_decode_block(samps+i,buflen,endutt,&parthyp);
	  
	  /*	  E_INFO("PARTIAL HYP:");
	  if (nhypwds > 0)
	    for (j=0; j < nhypwds; j++) printf(" %s",parthyp[j].word);
	  printf("\n");
	  */
        }
      }
    } else {    
      // RAH
      const int SAMPLE_RATE = 8000;
      if ((ad = ad_open_sps(SAMPLE_RATE)) == NULL)
	E_FATAL("ad_open_sps failed\n");

      utterance_loop();
    }
    exit(0);
}
