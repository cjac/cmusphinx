/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
/*
 * cviterbi.h
 * 
 */


#define HMM_EXT		"bhmm4"
#define CONVERGE	1
#define MIN_PROB	0.000001
#define SMOOTH_WEIGHT	0.0002
/* #define MAX_ALPHABET	256	Max Size of the alphabet */
#define MAX_STATES	8	/* Max number of states in the HMM */
#define MAX_ARCS	15	/* Max number of arcs in the HMM */
#define NULL_TRANSITION -1
#define Abs(x) (((x) > 0) ? (x) : -(x))

/* Transition describes a transition in HMM */

struct transition
{
  int32 destination,		/* Destination state of the arc */
      origin,
      out_prob_index,		/* Index to Output_Prob. -1 -> null trans/ */
      trans_prob;		/* Transitional probability of the arc */
};


/* State describes a state in the HMM */

struct state
{
  char  label,				/* The label of this state */
	num_from, num_to,
	is_initial, is_final,
	trans_from[MAX_STATES],	/* All transitions from it */
	trans_to[MAX_STATES];	/* All transitions to it */
};

struct hmm
{
  int32 num_states, num_omatrix, num_arcs, num_initial, num_final;
  int32 *output_prob, *output_prob2, *output_prob3, *output_prob4;
  struct state *states;
  struct transition *transitions;
};

struct dp_cell
{
  int32 value;
  int16 back_trace;
};

