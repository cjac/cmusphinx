
/*
 * ad.c -- Wraps a "sphinx-II standard" audio interface around the basic audio
 * 		utilities.
 * 
 * HISTORY
 * 
 * 11-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Modified to conform to new A/D API.
 * 
 * 12-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Dummy template created.
 */

#include <stdio.h>
#include <string.h>
#include <dmedia/audio.h>

#include "s3types.h"
#include "ad.h"

#define QUIT(x)		{fprintf x; exit(-1);}


ad_rec_t *ad_open_sps (int32 samples_per_sec)
{
  // fprintf(stderr, "A/D library not implemented\n");
    ad_rec_t *handle;
    ALpv          pv; 
    int device  = AL_DEFAULT_INPUT;
    ALconfig portconfig = alNewConfig(); 
    ALport port; 
    int32 sampleRate;
    long long gainValue = alDoubleToFixed(8.5); 
    
    pv.param = AL_GAIN; 
    pv.sizeIn = 1; 
    pv.value.ptr = &gainValue; 
    
    if (alSetParams(device, &pv, 1)<0) {
      fprintf(stderr, "setparams failed: %s\n",alGetErrorString(oserror()));
      return NULL; 
    }
    

    pv.param = AL_RATE;
    pv.value.ll = alDoubleToFixed(samples_per_sec);
    
    if (alSetParams(device, &pv, 1)<0) {
      fprintf(stderr, "setparams failed: %s\n",alGetErrorString(oserror()));
      return NULL; 
    }
    
    if (pv.sizeOut < 0) {
      /*
       * Not all devices will allow setting of AL_RATE (for example, digital 
       * inputs run only at the frequency of the external device).  Check
       * to see if the rate was accepted.
       */
      fprintf(stderr, "AL_RATE was not accepted on the given resource\n");
      return NULL; 
    }
    
    if (alGetParams(device, &pv, 1)<0) {
        fprintf(stderr, "getparams failed: %s\n",alGetErrorString(oserror()));
     }
    
    sampleRate = (int32)alFixedToDouble(pv.value.ll);
#if 0
    printf("sample rate is set to %d\n", sampleRate);
#endif


    if (alSetChannels(portconfig, 1) < 0) {
      fprintf(stderr, "alSetChannels failed: %s\n",alGetErrorString(oserror()));
      return NULL; 
    }

    port = alOpenPort(" Sphinx-II input port", "r", portconfig); 

    if (!port) {
      fprintf(stderr, "alOpenPort failed: %s\n", alGetErrorString(oserror()));
      return NULL; 
    }
    if ((handle = (ad_rec_t *) calloc (1, sizeof(ad_rec_t))) == NULL) {
      fprintf(stderr, "calloc(%d) failed\n", sizeof(ad_rec_t));
      abort();
    }

    handle->audio = port; 
    handle->recording = 0;
    handle->sps = sampleRate;
    handle->bps = sizeof(int16);

    alFreeConfig(portconfig); 

    return handle;
}


ad_rec_t *ad_open ( void )
{
    return ad_open_sps(DEFAULT_SAMPLES_PER_SEC);
}


int32 ad_start_rec (ad_rec_t *r)
{
    return 0;
}


int32 ad_stop_rec (ad_rec_t *r)
{
  /* how do we start/stop recording on an sgi? */
    return 0;
}


int32 ad_read (ad_rec_t *r, int16 *buf, int32 max)
{
  int32 length = alGetFilled(r->audio);
  
  if (length > max) {
    alReadFrames(r->audio, buf, max);
    return max; 
  } else {
    alReadFrames(r->audio, buf, length); 
    return length;
  }
}


int32 ad_close (ad_rec_t *r)
{
  if (r->audio) 
    alClosePort(r->audio); 
  free(r); 
  return 0;
}
