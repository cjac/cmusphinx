
/* 01.18.01 - RAH, allow for C++ compiles */
#ifdef __cplusplus
extern "C" {
#endif

#include <libutil/libutil.h>
#include "feat.h"


typedef struct {
    int32 ascr;
    int32 lscr;
    char  *word;
    int32 sf;
    int32 ef;
} partialhyp_t;

void live_initialize_decoder (char *argsfile);

int32 live_get_partialhyp(int32 endutt);

/* Routine to decode a block of incoming samples. A partial hypothesis
 * for the utterance upto the current block of samples is returned.
 * The calling routine has to inform the routine if the block of samples
 * being passed is the final block of samples for an utterance by
 * setting live_endutt to 1. On receipt of a live_endutt flag the routine
 * automatically assumes that the next block of samples is the beginning
 * of a new utterance
 */

int32 live_utt_decode_block (int16 *samples,    /* Incoming samples */
			     int32 nsamples,    /* No. of incoming samples */
                      	     int32 live_endutt, /* End of utterance flag */
			     partialhyp_t **ohyp);

int32 live_free_memory ();
  
#ifdef __cplusplus
}
#endif
