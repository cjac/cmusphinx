/* SEARCH_CONST.H
 * 10-23-93 M K Ravishankar (rkm@cs.cmu.edu)
 * (int) cast added to WORST_SCORE to overcome Alpha compiler bug.
 */

#ifndef _SEARCH_CONST_H_
#define _SEARCH_CONST_H_

#define NUM_CODE_BOOKS		4	/* Number of codebooks used */

#define MAX_FRAMES		8000	/* Max Frames in an utterance */

#define WORST_SCORE		((int)0xE0000000)
	/* Large negative number. This number must be small enough so that
	 * 4 times WORST_SCORE will not overflow. The reason for this is
	 * that the search doesn't check the scores in a model before
	 * evaluating the model and it may require as many was 4 plies
	 * before the new 'good' score can wipe out the initial WORST_SCORE
	 * initialization.
	 */

#define HMM_5_STATE		1
	/* Set to TRUE is the is a 5 state HMM, otherwise
	 * this is a 3 state HMM
	 */

#if HMM_5_STATE
#define HMM_LAST_STATE	5
#else
#define HMM_LAST_STATE	3
#endif

#endif /* _SEARCH_CONST_H_ */

