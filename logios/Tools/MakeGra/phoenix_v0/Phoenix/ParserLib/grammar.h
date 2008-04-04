#include "lengths.h"

/* grammar state structure */
typedef struct gnode
{
	short	n_suc;		/* number of succs */
	short	final;		/* true if final state */
	struct gsucc	*succ;	/* arcs out */
} Gnode;

/* grammar arc structure */
typedef struct gsucc
{
	int	tok;		/* word number, call_netber or 0 */
	int	call_net;	/* 0 or number of net called */
	Gnode	*state;		/* ptr to successor state */
} Gsucc;

typedef struct suc_link
{
	Gsucc	succ;
	struct suc_link *link;
	int nt;
} SucLink;

struct state_set {
	int state;
	char used;
	struct state_set *next;
};
typedef struct state_set set;

typedef struct {
	int tok;
	SucLink *arc;
	char	rw;	/* has been rewritten flag */
} non_term;

