/*
 * profile.h -- For timing and event counting.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 11-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ptmr_init().
 * 
 * 19-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from earlier Sphinx-3 version.
 */


#ifndef _LIBUTIL_PROFILE_H_
#define _LIBUTIL_PROFILE_H_


#include "prim_type.h"


/*
 * Generic event counter for profiling.  User is responsible for allocating an array
 * of the desired number.  There should be a sentinel with name = NULL.
 */
typedef struct {
    const char *name;		/* Counter print name; NULL terminates array of counters.
				   Used by pctr_print_all */
    int32 count;		/* Counter value */
} pctr_t;


void pctr_reset (pctr_t *ctr);
void pctr_reset_all (pctr_t *ctr);
void pctr_print_all (FILE *fp, pctr_t *ctr);


/*
 * Generic timer structures and functions for coarse-grained performance measurements
 * using standard system calls.
 */
typedef struct {
    const char *name;		/* Timer print name; NULL terminates an array of timers.
				   Used by ptmr_print_all */
    float64 t_cpu;		/* CPU time accumulated since most recent reset op */
    float64 t_elapsed;		/* Elapsed time accumulated since most recent reset */
    float64 t_tot_cpu;		/* Total CPU time since creation */
    float64 t_tot_elapsed;	/* Total elapsed time since creation */
    float64 start_cpu;		/* ---- FOR INTERNAL USE ONLY ---- */
    float64 start_elapsed;	/* ---- FOR INTERNAL USE ONLY ---- */
} ptmr_t;


/* Start timing using tmr */
void ptmr_start (ptmr_t *tmr);

/* Stop timing and accumulate tmr->{t_cpu, t_elapsed, t_tot_cpu, t_tot_elapsed} */
void ptmr_stop (ptmr_t *tmr);

/* Reset tmr->{t_cpu, t_elapsed} to 0.0 */
void ptmr_reset (ptmr_t *tmr);

/* Reset tmr->{t_cpu, t_elapsed, t_tot_cpu, t_tot_elapsed} to 0.0 */
void ptmr_init (ptmr_t *tmr);


/*
 * Reset t_cpu, t_elapsed of all timer modules in array tmr[] to 0.0.
 * The array should be terminated with a sentinel with .name = NULL.
 */
void ptmr_reset_all (ptmr_t *tmr);

/*
 * Print t_cpu for all timer modules in tmr[], normalized by norm (i.e., t_cpu/norm).
 * The array should be terminated with a sentinel with .name = NULL.
 */
void ptmr_print_all (FILE *fp, ptmr_t *tmr, float64 norm);


/*
 * Return the processor clock speed (in MHz); only available on some machines (Alphas).
 * The dummy argument can be any integer value.
 */
int32 host_pclk (int32 dummy);


/*
 * Check the native byte-ordering of the machine by writing a magic number to a
 * temporary file and reading it back.
 * Return value: 0 if BIG-ENDIAN, 1 if LITTLE-ENDIAN, -1 if error.
 */
int32 host_endian ( void );


#endif
