/*
 * global constants and map of locations in sm for hmm sap rb - Nov 87 
 */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEBUG	1

#define WRS	<
#define BTR	>
#define WRE	<=

#define BESTSCORE	(int32) 0
#define WORSTSCORE	min_log
#define NOSCORE	(int32) 0x80000000	/* largest negative, worse than worst */
#define LASTWORDSTATE	0x80000000	/* both defined in c code */
#define BOUNDARYERROR	0x80000000	
#define MEMSIZEERROR	-200


#define EMPTY	0		/* marks an empty entry in the beam table */
#define	ENDOFLIST	0	/* end of history tree */
#define HON_WORD_END	1	/* marks last phoneme of word in sib */

#define MAXNUMOFNETSTATES	17000
#define	NUMOFHMMS		7600
#define MAXSTATESPERHMM		6
#define NUMOFCODEENTRIES	256
#define NUMDISTRTYPES	5 
/* values to change if BOUNDARYERROR (0x80000000) */
#define NUMOFHISTORYWORDS	1500000	 /* 1600000 */
#define BEAMSIZE	160000  /* 26000 */
#define FINALBEAM	3000	/* 500 */
#define ENDBEAMSIZE	6000	/* 6000 */
/* end of boundary paramters */
#define NUMOFDISTRBUF	4	/* distribution buffers, minimum 2 */
#define MAX_SENT_LENGTH 	1500	/* 15 seconds */
#define MAX_WORDS_PER_SENT	200
#define NUMOFWORDS	1017
#define ENDOFBEAM	-1


/****************************************/
#define USERSMBASE			0

#define INFOSIZE		(sizeof(INFO))

#define INFOBASE		(USERSMBASE)

#define BSCORESIZE		(MAX_SENT_LENGTH * sizeof(BSCORE))

#define BSCOREBASE		(INFOBASE + INFOSIZE)

#define LASTWORDSIZE		(sizeof (LASTWORD) * NUMOFWORDS)

#define LASTWORDBASE		(BSCOREBASE + BSCORESIZE)

#define SCODESIZE		 (sizeof(INPUT_CODES)* MAX_SENT_LENGTH)

#define CODEBASE		(LASTWORDBASE + LASTWORDSIZE)

#define HISTORYSIZE		(sizeof(HISTORY) * NUMOFHISTORYWORDS)

#define HISTORYBASE		(CODEBASE + SCODESIZE)

#define TOTAL_OMATRIX	6240
#define PHONLIKSIZE 	(sizeof(int32) * TOTAL_OMATRIX * NUMOFDISTRBUF)

#define	PHONLIKBASE		(HISTORYBASE + HISTORYSIZE)
 
#define BEAM_ENTRYSIZE		7000000  /* 7000000 */

/*#define BEAM_ENTRYBASE		(HISTORYBASE + HISTORYSIZE) */
#define BEAM_ENTRYBASE		(PHONLIKBASE + PHONLIKSIZE)

#define REQBASE		(BEAM_ENTRYBASE + BEAM_ENTRYSIZE)

#define REQSIZE		(sizeof(REQINFO))

#define ACKBASE		(REQBASE + REQSIZE)

#define ACKSIZE		(sizeof (RETINFO))

#define BPRETBASE	(ACKBASE + ACKSIZE)

#define BPRETSIZE	(FINALBEAM * sizeof(BEAM_POINTERP))

#define SMSIZE	(BPRETBASE + BPRETSIZE - USERSMBASE)
