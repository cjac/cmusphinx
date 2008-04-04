#include "grammar.h"

#define	PATH_TREE_DEPTH	50	/* max call-nets in single rewrite rule */
#define HIST_LEN	5

typedef unsigned short	Id;

typedef struct framedef {
	Id	n_slot;		/* number of slots in form */
	Id	*slot;		/* net numbers for slots */
	char	**prompt;	/* propmt for slot */
} FrameDef;

typedef struct gram
{
    FrameDef	*frame_def;		/* nets used in each form */
    char	**frame_name;		/* frame names */
    int		num_frames;		/* number of frames read in */
    char	**labels;		/* names of nets */
    Gnode	**Nets;			/* pointers to heads of nets */
    int		num_nets;		/* number of nets read in */
    char	**wrds;			/* pointers to strings for words */
    int		num_words;		/* number of words in lexicon */
    int		*node_counts;		/* number of nodes in each net */
    char	*leaf;			/* concept leaf flags */
    unsigned char	*priorities;	/* priorities for nets */
    int		max_pri;		/* max net priority level */
    char	*sym_buf;		/* strings for words and names */
} Gram;


/* cells of chart */
typedef struct edge {
    Id			net;
    Id			sw;
    Id			ew;
    Id			score;
    char		nchld;
    struct edge		**chld;
    struct edge		*link;
} Edge;


/* temporary structure used in matching nets to create edges */
typedef struct buf {
	Gnode	*state;		/* current state */
	Id	net;		/* number of top-level net */
	Id	sw;		/* number of top-level net */
	Id	ew;		/* number of top-level net */
	Id	score;		/* number of top-level net */
	char	nchld;
	Edge	*chld[PATH_TREE_DEPTH];	/* pointers to sub-trees (children) */
	int	word_pos;	/* extended flag */
}Path;



/* structure for linking edges into chart */
typedef struct edge_link {
	int	sw;
	struct edge_link *link;
	Edge *edge;
}EdgeLink;


typedef struct frame_id {
	unsigned short id;
	unsigned short count;
} Fid;

typedef struct seq_node {
	Edge *edge;
	unsigned short	n_frames;	/* frame count for path */
	unsigned short	n_act;	/* number of active frames for path */
	Fid	*frame_id;
	struct seq_node *back_link;
	struct seq_node *link;
	unsigned short *pri;
} SeqNode;

typedef struct seq_cell {
	Id score;
	Id n_slots;
	Id n_frames;
	SeqNode *link;
} SeqCell;

typedef struct frame_node {
	int n_frames;
	int frame;
	SeqNode *slot;
	struct frame_node *bp;
	struct frame_node *link;
} FrameNode;



/* functions */
void exit();
void parse();
void read_script();
void copy_chart();
void breakpt_reset();
int add_to_seq();
int cmp_pri();
void expand_seq();
void map_to_frames();
int net_in_frame();
int check_chart();
int append_slot();
int score_phrase();
void print_seq();
void score_pri();
void clear_chart();
int pconf();
int fpconf();
void pusage();
void strip_underscore();
int expand_path();
int check_final();
void read_nets();
void read_frames();
int find_net();
void quit();
int find_frame();
void config();
void init_parse();
void print_profile();
void reset();
void print_parse();
void start_session();
