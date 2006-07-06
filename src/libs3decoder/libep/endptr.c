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
 * HISTORY
 * $Log$
 * Revision 1.8  2006/03/03  20:02:38  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 * 
 * Revision 1.7  2006/02/23 04:05:21  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: fixed dox-doc.
 *
 *
 * Revision 1.5.4.1  2005/07/05 06:46:23  arthchan2003
 * 1, Merged from HEAD.  2, fixed dox-doc.
 *
 * Revision 1.6  2005/07/02 04:24:45  egouvea
 * Changed some hardwired constants to user defined parameters in the end pointer. Tested with make test-ep.
 *
 * Revision 1.5  2005/06/21 21:07:28  arthchan2003
 * Added  keyword.
 *
 * Revision 1.2  2005/06/15 06:48:54  archan
 * Sphinx3 to s3.generic: 1, updated the endptr and classify 's code, 2, also added
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "endptr.h"

endpointer_t *
endpointer_initialize(fe_t * FE,
                      float pad_t_before,
                      float pad_t_after,
                      float utt_t_start,
                      float utt_t_end, float utt_t_cancel)
{

    endpointer_t *ENDPTR =
        (endpointer_t *) calloc(1, sizeof(endpointer_t));

    /* set the fixed specifications */

    ENDPTR->PAD_F_BEFORE =
        (int) (pad_t_before * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
    ENDPTR->PAD_F_AFTER =
        (int) (pad_t_after * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
    ENDPTR->UTT_F_START =
        (int) (utt_t_start * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
    ENDPTR->UTT_F_END =
        (int) (utt_t_end * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);
    ENDPTR->UTT_F_CANCEL =
        (int) (utt_t_cancel * FE->SAMPLING_RATE / FE->FRAME_SHIFT + 0.5);

    /* set the rest specifications */

    ENDPTR->status = STAT_OTHER;
    ENDPTR->leader = ENDPTR->spbegin = ENDPTR->spend = ENDPTR->trailer = 0;

    ENDPTR->utt_counter = 0;

    return (ENDPTR);
}

void
endpointer_free(endpointer_t * ENDPTR)
{
    if (ENDPTR)
        ckd_free(ENDPTR);
}

void
endpointer_update_stat(endpointer_t * ENDPTR, fe_t * FE, class_t * CLASSW,
                       int class)
{

    switch (ENDPTR->status) {
    case STAT_OTHER:
        /* we are in the non-speech region and we received a speech frame */
        if (class == CLASS_O) {
            ENDPTR->status = STAT_BEGIN;
            ENDPTR->spbegin = FE->FRAME_COUNTER - CLASSW->classlatency;
            ENDPTR->leader = ENDPTR->spbegin - ENDPTR->PAD_F_BEFORE;
            ENDPTR->utt_counter++;

            printf("Utt_Start#%d, Leader: %6.5f,  Begin: %6.5f\n",
                   ENDPTR->utt_counter, endptr_frame2secs_beg(FE,
                                                              ENDPTR->
                                                              leader),
                   endptr_frame2secs_beg(FE, ENDPTR->spbegin));
            fflush(stdout);
        }
        break;

    case STAT_BEGIN:
        /* we are in the speech region but still not enough frames to annouce
           start of utterance   */
        if ((class == CLASS_O) && ((FE->FRAME_COUNTER - ENDPTR->spbegin) <
                                   ENDPTR->UTT_F_START)) {
            /* do nothing for now   */
        }

        /* we are ready to start the utterance */
        else if (class == CLASS_O) {
            ENDPTR->status = STAT_SPEECH;
        }

        /* we were beginning to get speech and suddenly we get a non-speech frame */
        else {
            ENDPTR->status = STAT_CANCEL;
            ENDPTR->spend = FE->FRAME_COUNTER - 1 - CLASSW->classlatency;
        }
        break;

    case STAT_SPEECH:
        if (class != CLASS_O) {
            ENDPTR->status = STAT_END;
            ENDPTR->spend = FE->FRAME_COUNTER - 1 - CLASSW->classlatency;
            ENDPTR->trailer = ENDPTR->spend + ENDPTR->PAD_F_AFTER;

        }
        break;
    case STAT_END:
        /* we thought we are ending the utterance and we get a speech frame */
        if (class == CLASS_O) {
            ENDPTR->status = STAT_SPEECH;
        }

        /* still not enough non-speech frames to end the utterance      */
        else if ((FE->FRAME_COUNTER - ENDPTR->spend) < ENDPTR->UTT_F_END) {
            /* do nothing bfor now */
        }

        /* enough non-speech frames to end utterance */
        else {
            ENDPTR->status = STAT_OTHER;
            printf("Utt_End#%d, End: %6.5f,  Trailer: %6.5f\n",
                   ENDPTR->utt_counter, endptr_frame2secs_end(FE,
                                                              ENDPTR->
                                                              spend),
                   endptr_frame2secs_end(FE, ENDPTR->trailer));
            /*     printf("Utt_End End: %d,  Trailer: %d\n",ENDPTR->spend, ENDPTR->trailer); */
            fflush(stdout);
        }
        break;

    case STAT_CANCEL:
        /* we wanted to cancel and boom we get another speech frame!    */
        if (class == CLASS_O) {
            ENDPTR->status = STAT_BEGIN;
        }

        /* still not enough non-speech frames to cancel the utterance */
        else if ((FE->FRAME_COUNTER - ENDPTR->spend) <
                 ENDPTR->UTT_F_CANCEL) {
            /* do nothing for now */
        }

        /* enough non-speech frames to end utterance */
        else {
            ENDPTR->status = STAT_OTHER;
            printf("Utt_Cancel#%d End: %6.5f\n", ENDPTR->utt_counter,
                   endptr_frame2secs_end(FE, ENDPTR->spend));
            /*      printf("Utt_Cancel End: %d\n",ENDPTR->spend); */
            fflush(stdout);

            ENDPTR->utt_counter--;
        }
        break;

    }

}

float
endptr_frame2secs_beg(fe_t * FE, int frame)
{
    return ((float) (frame - 1) * FE->FRAME_SHIFT / FE->SAMPLING_RATE);

}

float
endptr_frame2secs_end(fe_t * FE, int frame)
{
    return ((float) ((frame - 1) * FE->FRAME_SHIFT + FE->FRAME_SIZE) /
            FE->SAMPLING_RATE);

}
