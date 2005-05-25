/* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights
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

/*************************************************
 * CMU CALO Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * 11-Jul-2004  Ziad Al Bawab (ziada@cs.cmu.edu) at Carnegie Mellon University
 * Created
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 

#include "endptr.h"

endpointer_t * endpointer_initialize(fe_t *FE)
{

        endpointer_t *ENDPTR = (endpointer_t *) calloc(1,sizeof(endpointer_t));

        /* set the fixed specifications */

        ENDPTR->PAD_F_BEFORE = (int)(PAD_T_BEFORE * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
        ENDPTR->PAD_F_AFTER  = (int)(PAD_T_AFTER * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
        ENDPTR->UTT_F_START  = (int)(UTT_T_START * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
        ENDPTR->UTT_F_END    = (int)(UTT_T_END * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
        ENDPTR->UTT_F_CANCEL = (int)(UTT_T_CANCEL * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);

        /* set the rest specifications */

        ENDPTR->status = STAT_OTHER;
        ENDPTR->leader = ENDPTR->spbegin = ENDPTR->spend = ENDPTR->trailer = 0;

        ENDPTR->utt_counter = 0;        

        return (ENDPTR);
}

void endpointer_free(endpointer_t *ENDPTR)
{
        if(ENDPTR)
                ckd_free(ENDPTR);
}

void endpointer_update_stat (endpointer_t *ENDPTR, fe_t *FE, class_t *CLASSW, int class)
{
  
        switch (ENDPTR->status)
        {
        case STAT_OTHER:
                // we are in the non-speech region and we received a speech frame
                if (class == CLASS_O)
                {
                        ENDPTR->status = STAT_BEGIN;
                        ENDPTR->spbegin = FE->FRAME_COUNTER - CLASSW->classlatency;
                        ENDPTR->leader = ENDPTR->spbegin - ENDPTR->PAD_F_BEFORE;
                        ENDPTR->utt_counter++;

                        printf("Utt_Start#%d, Leader: %6.5f,  Begin: %6.5f\n", ENDPTR->utt_counter,endptr_frame2secs_beg( FE,ENDPTR->leader), endptr_frame2secs_beg(FE, ENDPTR->spbegin) ); 
                        fflush(stdout);
                }
                break;

        case STAT_BEGIN:
                // we are in the speech region but still not enough frames to annouce
                // start of utterance   
                if ((class == CLASS_O) && ((FE->FRAME_COUNTER - ENDPTR->spbegin) < 
                                           ENDPTR->UTT_F_START))
                {
                        // do nothing for now   
                }

                // we are ready to start the utterance
                else if (class == CLASS_O)
                {
                        ENDPTR->status = STAT_SPEECH;
                }

                // we were beginning to get speech and suddenly we get a non-speech frame
                else
                {
                        ENDPTR->status = STAT_CANCEL;
                        ENDPTR->spend = FE->FRAME_COUNTER - 1 - CLASSW->classlatency;
                }
                break;

        case STAT_SPEECH:
                if (class != CLASS_O)
                {
                        ENDPTR->status = STAT_END; 
                        ENDPTR->spend = FE->FRAME_COUNTER - 1 - CLASSW->classlatency;
                        ENDPTR->trailer = ENDPTR->spend + ENDPTR->PAD_F_AFTER;

                }
                break;
        case STAT_END:
                // we thought we are ending the utterance and we get a speech frame
                if (class == CLASS_O)
                {
                        ENDPTR->status = STAT_SPEECH;   
                }

                // still not enough non-speech frames to end the utterance      
                else if ((FE->FRAME_COUNTER - ENDPTR->spend) < ENDPTR->UTT_F_END)
                {                       
                        // do nothing for now
                }

                // enough non-speech frames to end utterance
                else
                {
                        ENDPTR->status = STAT_OTHER;
                        printf("Utt_End#%d, End: %6.5f,  Trailer: %6.5f\n", ENDPTR->utt_counter,endptr_frame2secs_end(FE, ENDPTR->spend), endptr_frame2secs_end(FE, ENDPTR->trailer));
                        //      printf("Utt_End End: %d,  Trailer: %d\n",ENDPTR->spend, ENDPTR->trailer);
                        fflush(stdout);
                }
                break;

        case STAT_CANCEL:
                // we wanted to cancel and boom we get another speech frame!    
                if (class == CLASS_O)
                {
                        ENDPTR->status = STAT_BEGIN;    
                }

                // still not enough non-speech frames to cancel the utterance
                else if ((FE->FRAME_COUNTER - ENDPTR->spend) < ENDPTR->UTT_F_CANCEL)
                {                        
                        // do nothing for now 
                }

                // enough non-speech frames to end utterance
                else
                {
                        ENDPTR->status = STAT_OTHER;
                        printf("Utt_Cancel#%d End: %6.5f\n", ENDPTR->utt_counter,endptr_frame2secs_end(FE, ENDPTR->spend));
                        //      printf("Utt_Cancel End: %d\n",ENDPTR->spend);
                        fflush(stdout);
                                                
                        ENDPTR->utt_counter--;
                }
                break;

        }

}

float endptr_frame2secs_beg (fe_t *FE, int frame)
{
        return ( (float)(frame - 1) * FE->FRAME_SHIFT / FE->SAMPLING_RATE);

}

float endptr_frame2secs_end (fe_t *FE, int frame)
{
        return ( (float)((frame - 1) * FE->FRAME_SHIFT + FE->FRAME_SIZE) / FE->SAMPLING_RATE);

}
