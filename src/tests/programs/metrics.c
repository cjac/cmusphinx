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

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "metrics.h"
#include "ckd_alloc.h"

#define MAX_DURATION_SLOTS 32



static NamedDuration namedDuration[MAX_DURATION_SLOTS];


/**
 * Given a name, find the associated NamedDuration. If none is found
 * create it, if there are no more slots, return NULL
 *
 * @param name the name to look for
 * @return the NamedDuration or NULL
 */
static NamedDuration *
findDuration(const char *name)
{
    int i;

    for (i = 0; i < MAX_DURATION_SLOTS && namedDuration[i].name; i++) {
        if (strcmp(namedDuration[i].name, name) == 0) {
            return &namedDuration[i];
        }
    }

    if (i < MAX_DURATION_SLOTS) {
        /*   namedDuration[i].name = strdup(name); */
        namedDuration[i].name = ckd_salloc(name);
        strcpy((char *) namedDuration[i].name, name);
        return &namedDuration[i];
    }
    return NULL;
}

/**
 * Starts timing the given operation
 *    @param name the name of the timer to start
 */
void
metricsStart(const char *name)
{
    struct timeval tv;
    NamedDuration *namedDuration;

    gettimeofday(&tv, NULL);
    namedDuration = findDuration(name);

    if (namedDuration == NULL) {
        return;
    }


    namedDuration->start
        = (((double) (tv.tv_sec)) + ((double) tv.tv_usec / 1000000.0));
}

/**
 * Stops timing the given operation
 * @param name the name of the timer to stop
 */
void
metricsStop(const char *name)
{
    double time_current;
    struct timeval tv;
    NamedDuration *namedDuration;

    gettimeofday(&tv, NULL);
    namedDuration = findDuration(name);

    if (namedDuration == NULL) {
        return;
    }

    time_current =
        (((double) (tv.tv_sec)) + ((double) tv.tv_usec / 1000000.0));

    /*
       printf(" %s %f %f %f\n", name, namedDuration->start, time_current,
       time_current - namedDuration->start);
     */
    namedDuration->duration +=
        (float) (time_current - namedDuration->start);
    namedDuration->count++;
}


/**
 * Resets the given timer.
 *
 * @param name the name of the timer to reset
 */
void
metricsReset(const char *name)
{
    int i;

    for (i = 0; i < MAX_DURATION_SLOTS && namedDuration[i].name; i++) {
        if (strcmp(namedDuration[i].name, name) == 0) {
            namedDuration[i].duration = 0.0;
            namedDuration[i].count = 0;
            break;
        }
    }
}


/**
 * Returns the duration of the given timer.
 */
double
metricsDuration(const char *name)
{
    NamedDuration *duration = findDuration(name);
    if (duration != NULL) {
        return duration->duration;
    }
    else {
        return 0.0;
    }
}


/**
 * Prints the processing time of each step in the speech synthesis process.
 */
void
metricsPrint(void)
{
    int i;
    float total = 0.0;

    printf("%20.20s %9s %6s %9s\n", "Function", "Total", "Count",
           "Average");
    for (i = 0; i < MAX_DURATION_SLOTS && namedDuration[i].count; i++) {
        printf("%20.20s %9.4f %6.6d %9.4f\n",
               namedDuration[i].name,
               namedDuration[i].duration,
               namedDuration[i].count,
               namedDuration[i].duration / namedDuration[i].count);
        total += namedDuration[i].duration;
    }
    printf("Total Times %f seconds\n", total);
}
