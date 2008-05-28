#include "globals_gram.h"

/* flags */
extern int	verbose,
		extract,
		MAX_PARSES,
		USE_HISTORY,
		PROFILE,
		IGNORE_OOV,
		ALL_PARSES,
		BIGRAM_PRUNE,
		MAX_PATHS;	/* if set, limit max paths expanded */

/* file and dir names */
extern char	*dir,
		*config_file;

/* lists of nets used in parse*/
extern int	*cur_nets,	/* set of nets to be used in this parse */
		num_active,	/* length of cur_nets */
		num_nets,	/* number of nets read in */
		num_nets,
		num_frames,
		max_pri;
extern Gram	*gram;			/* grammar */

extern int	start_token, end_token;

/* parser structures */
extern int	*script,	/* array of word nums for input line */
		script_len;	/* number of words in script */
extern int	*matched_script;	/* words included in parse */
extern char	fun_wrds[];	/* flags indicating function words */
extern int	num_parses;

/* buffers */
extern int	EdgeBufSize,	/* max number of paths in beam */
		ChartBufSize,	/* max number of paths in beam */
		PeBufSize,	/* number of Val slots for trees  */
		InputBufSize,	/* max words in line of input */
		SlotSeqLen,	/* max number of slots in a sequence */
		ParseBufSize,	/* buffer for parses */
		SeqBufSize,	/* buffer for sequence nodes */
		FrameBufSize;	/* buffer for frame nodes */

/* parser buffers */
extern EdgeLink	**chart;	/* chart of matched nets */
extern EdgeLink	*edge_link_buf,	/* buffer of nodes to link edges in lists */
		*edge_link_end,
		*edge_link_ptr;
extern Edge 	*edge_buf,	/* buffer of chart edges */
		*edge_buf_end, 
		*edge_ptr;	/* ptr to next free edge in buf */
extern Edge	**pe_buf,	/* buffer for trees pointed to by edges */
		**pe_buf_end,
		**pe_buf_ptr;

extern char	*print_line;	/* buffer for output */

