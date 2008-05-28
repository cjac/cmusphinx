/* parse function
   Take word string as argument
   Generate lattice of parses in structure "parses"
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <malloc.h>
#include <sys/time.h>

#include "parse.h"
#include "functions.h"
#include "pconf.h"

#define NO_SCORE 0xFFFF

Gram *read_grammar();
void extend_map();

/* ********* Command Line Parameters ************** */

/* flags */
int	verbose		=1,
	extract		=0,
	MAX_PARSES	=10,
	USE_HISTORY	=0,
	PROFILE		=0,
	IGNORE_OOV	=1,
	ALL_PARSES	=1,
	BIGRAM_PRUNE	=0,
	MAX_PATHS	=0;	/* if set, limit max paths expanded */


/* files */
char	*grammar_file = NULL,
	*dir = NULL,
	*config_file = NULL,
	*function_wrd_file = NULL;

/* default buffer sizes */
int	EdgeBufSize=	1000,	/* max number of paths in beam */
	ChartBufSize=	40000,	/* max number of paths in beam */
	PeBufSize=	2000,	/* number of Val slots for trees  */
	InputBufSize=	1000,	/* max words in line of input */
	StringBufSize=	50000,	/* max words in line of input */
	SlotSeqLen=	200,	/* max number of slots in a sequence */
	FrameBufSize=	500,	/* buffer for frame nodes */
	SymBufSize=	50000,	/* buffer size to hold char strings */
	ParseBufSize=	200,	/* buffer for parses */
	SeqBufSize=	500,	/* buffer for sequence nodes */
	PriBufSize=	2000,	/* buffer for sequence nodes */
	FidBufSize=	1000;	/* buffer for frame ids */

char	sym1[] = "<s>",		/* default start of utt symbol */
	sym2[] = "</s>",	/* default end of utt symbol */
	*start_sym= sym1,
	*end_sym= sym2;

/* Command Line structure */

config_t conf[] = {
  {"dir",	"dir containing grammar files (frames, base.dic, etc)",
       "-dir", 	STRING,	(caddr_t) &dir},
  {"grammar",	"file containing compiled grammar nets",
       "-grammar", 	STRING,	(caddr_t) &grammar_file},
  {"start_sym",	"start of utterance grammar symbol",
       "-start_sym", 	STRING,	(caddr_t) &start_sym},
  {"end_sym",	"end of utterance grammar symbol",
       "-end_sym", 	STRING,	(caddr_t) &end_sym},
  {"config_file",	"file containing command line arguments",
       "-config", 	STRING,	(caddr_t) &config_file},
  {"function_wrd_file",	"file containing function words",
       "-function_wrd", STRING,	(caddr_t) &function_wrd_file},
  {"verbose",	"verbosity level",
       "-verbose", 	INT, 	(caddr_t) &verbose},
  {"extract",	"0= print full parse, 1= print extracted elements",
       "-extract",	INT,		(caddr_t) &extract},
  {"MAX_PARSES",	"maximum number of parses to print",
       "-MAX_PARSES",	INT,	(caddr_t) &MAX_PARSES},
  {"USE_HISTORY",	"use history in finding best parse",
       "-USE_HISTORY",	INT,	(caddr_t) &USE_HISTORY},
  {"PROFILE",	"print profile stats",
       "-PROFILE",	INT,		(caddr_t) &PROFILE},
  {"IGNORE_OOV",	"Ignore oov words in parse",
       "-IGNORE_OOV",	INT,	(caddr_t) &IGNORE_OOV},
  {"ALL_PARSES",	"Print all parses to text buffer",
       "-ALL_PARSES",	INT,	(caddr_t) &ALL_PARSES},
  {"BIGRAM_PRUNE",	"1= aggressive pruning in parse search",
       "-BIGRAM_PRUNE",	INT,	(caddr_t) &BIGRAM_PRUNE},
  {"MAX_PATHS", "Maximum provisional edges expanded before pruning", 
       "-MAX_PATHS",	INT,	(caddr_t) &MAX_PATHS},
  {"EdgeBufSize", "Maximum edges in chart", 
       "-EdgeBufSize",	INT,	(caddr_t) &EdgeBufSize},
  {"ChartBufSize", "Maximum edge links in chart", 
       "-ChartBufSize", INT,	(caddr_t) &ChartBufSize},
  {"PeBufSize", "Number of Val structs for trees", 
       "-PeBufSize", INT,	(caddr_t) &PeBufSize},
  {"SymBufSize", "Size in bytes of symbol buffer for dictionary", 
       "-SymBufSize",	INT,	(caddr_t) &SymBufSize},
  {"FrameBufSize", "Maximum number of Frame Nodes in parses", 
       "-FrameBufSize",	INT,	(caddr_t) &FrameBufSize},
  {"FidBufSize", "Maximum size of Frame Table Structure", 
       "-FidBufSize",	INT,	(caddr_t) &FidBufSize},
  {"PriBufSize", "Maximum size of Frame Table Structure", 
       "-PriBufSize",	INT,	(caddr_t) &PriBufSize},
  {"ParseBufSize", "Buffer for set of parses", 
       "-ParseBufSize",	INT,	(caddr_t) &ParseBufSize},
  {"InputBufSize", "Maximum number of words in line of input", 
       "-InputBufSize",	INT,	(caddr_t) &InputBufSize},
  {"StringBufSize", "size of buffer for parse substrings", 
       "-StringBufSize",	INT,	(caddr_t) &StringBufSize},
  {"SeqBufSize", "Maximum size of Seq Table Structure", 
       "-SeqBufSize",	INT,	(caddr_t) &SeqBufSize},
  {"SlotSeqLen", "Maximum number of slots in a sequence", 
       "-SlotSeqLen",	INT,	(caddr_t) &SlotSeqLen},
  {NULL, NULL, NULL, NOTYPE, NULL}
  };


/* file parameters */
char	dict_file[LABEL_LEN] =		"base.dic",
	priority_file[LABEL_LEN] =	"NET_PRIORITIES",
	frames_file[LABEL_LEN] =	"frames";



/*** global variables ***/

int	*active_slots,		/* set of slot level nets used in frames */
	*cur_nets,		/* set of nets to be used in this parse */
	num_active,		/* length of cur_nets */
	num_nets,		/* total number of nets read in */
	num_frames,		/* total number of frames read in */
	max_pri;		/* number of priority levels assigned to nets */

Gram	*gram;			/* grammar */

/* parser structures */
int	*script,		/* array of word nums for input line */
	script_len,		/* number of words in script */
	num_seqs,		/* number of current parses */
	start_token,		/* word number of start symbol */
	end_token;		/* word number of end symbol */

char	fun_wrds[MAX_WRDS];	/* flags indicating function words */


/* parser buffers */
SeqNode		**pbuf, **pbuf_end;	/* lattice of final parses */
EdgeLink	**chart;	/* chart of matched nets */
Edge 		*edge_buf,	/* buffer of chart edges */
		*edge_buf_end, 
		*edge_ptr;	/* ptr to next free edge in buf */
EdgeLink	*edge_link_buf,	/* buffer of nodes to link edges in lists */
		*edge_link_end,
		*edge_link_ptr;
Edge		**pe_buf,	/* buffer for trees pointed to by edges */
		**pe_buf_end,
		**pe_buf_ptr;
Edge		**slot_seq;
SeqCell		*seq_end;
SeqNode		*seq_buf,		/* buffer for sequence path tree */
		*seq_buf_ptr,
		*seq_buf_end;
Fid		*fid_buf,		/* buffer for frame path tree */
		*fid_buf_ptr,
		*fid_buf_end;
unsigned short	*pri_buf, *pri_buf_ptr, *pri_buf_end;
char		*pstr_buf,
		*pstr_buf_ptr,
		*pstr_buf_end;

int	first_concept;		/* used by print_extracts() */

int	fr_seq[100];
SeqNode	*sl_seq[100];
SeqNode *parses[500];
int	num_parses,
	hi,
	history[ HIST_LEN];

/**** Static Variables******* */
FrameNode	*frame_buf,		/* buffer for frame path tree */
		*frame_buf_ptr,
		*frame_buf_end,
		**fr_list1,
		**fr_list2;

int		n_slots,	/* number of slots in current parses */
		n_frames;	/* number of frames in current parses */

/* used for segmenting input */
int	last_end = 0,
	brk_pt = 0;

char	print_line[LINE_LEN],
	*print_line_ptr;

/*** profile variables ***/

struct timeval	start_tp,
		end_tp,
		match_start_tp,
		match_end_tp;
struct timezone	start_tzp,
		end_tzp;

long	msec;

/* forward function declarations */
SeqNode		*get_new_slot();
FrameNode	*get_new_frame();
Edge 		*copy_tree();


void parse(char *line, Gram *gram)
{

    int		word_pos,
		slot_num,
		result,
    		i, j, best_wp;
    SeqNode	*best_seq;

    if( PROFILE ) gettimeofday( &start_tp, &start_tzp );


    /* if cur_nets not set, create from frames in grammar */
    if( !num_active || !cur_nets ) {
        int fn, sn;

    	/* for each frame */
    	for(fn= 0; fn < gram->num_frames; fn++) {
	    /* for each slot in frame */
	    for(sn= 0; sn < gram->frame_def[fn].n_slot; sn++) {
    	    	/* see if already in list */
	    	for(i=0; i < num_active; i++) {
		    if( gram->frame_def[fn].slot[sn] == active_slots[i] ) break;
	    	}
    	    	if( i < num_active ) continue;
    	    	active_slots[num_active++]= gram->frame_def[fn].slot[sn];
	    }
    	}
        cur_nets= active_slots;
    }

    *print_line = 0;
    print_line_ptr = print_line;

    /* convert word strings to numbers and put in script array */
    read_script( line, gram );

    if( PROFILE ) gettimeofday( &match_start_tp, &start_tzp );


    /* to parse, for each word position in input try to match each slot 
       this creates chart and tree of slot seqs */
    for( word_pos= 1; word_pos < script_len; word_pos++) {

	/* segment input by setting brk_pt */
	if(  ((word_pos - brk_pt) > 10) && (last_end < word_pos) ) {
	    copy_chart(brk_pt, word_pos-1);
	    breakpt_reset(gram->num_nets);
	    brk_pt= word_pos-1;
	}

	for(slot_num= 0; slot_num < num_active; slot_num++) {

	    /* match net slot_num starting at word_pos */
	    if ( (result=match_net(cur_nets[slot_num], word_pos, gram)) < 0) {
	    	if ( result == -2 ) break ; /* # of paths exploded */
		else return;
	    }

	    /* add matched slot to slot sequence trees */
	    if( add_to_seq(cur_nets[slot_num], word_pos, gram) < 0) {
                return;
            }
	}

    	if ( result == -2 ) break ; /* # of paths exploded */
    }

    /* copy final set */
    copy_chart(brk_pt, word_pos-1);

    if( PROFILE ) gettimeofday( &match_end_tp, &end_tzp );

    /* find best scoring seq end */
    best_wp= 0;
    for( word_pos= 0; word_pos < script_len; word_pos++) {
	/* compare score */
	if( seq_end[word_pos].score < seq_end[best_wp].score ) continue;
	if( seq_end[word_pos].score > seq_end[best_wp].score ) {
		best_wp= word_pos;
		continue;
	}
	/* compare number of slots */
	if( seq_end[word_pos].n_slots > seq_end[best_wp].n_slots ) continue;
	if( seq_end[word_pos].n_slots < seq_end[best_wp].n_slots ) {
		best_wp= word_pos;
		continue;
	}
	/* compare number of frames */
	if( seq_end[word_pos].n_frames > seq_end[best_wp].n_frames ) continue;
	if( seq_end[word_pos].n_frames < seq_end[best_wp].n_frames ) {
		best_wp= word_pos;
		continue;
	}
    }
    if( !best_wp ) {
	printf("No parse\n\n");
	num_seqs= 0;
	return;
    }

    /* print number of paths ending at each word position */
    if( verbose > 2 ) {
    int count;
    SeqNode *sn;
    int max, max_idx;

	max= max_idx=0;
        printf("number of paths ending at word position\n");
        for( word_pos= 0; word_pos < script_len; word_pos++) {
	    count= 0;
	    for( sn= seq_end[word_pos].link; sn; sn= sn->link) count++;
	    printf(" %d", count);
	    if (count > max) {
		max= count;
		max_idx= word_pos;
	    }
        }
        printf("\n");
        printf("best_wp= %d\n", best_wp);
    }


    n_slots= seq_end[best_wp].n_slots;
    num_seqs= 0;


    /* find least fragmented seq */
    best_seq= 0;
    n_frames= seq_end[best_wp].n_frames;
    {
	SeqNode *sq;

	/* for each sequence */
	for( sq= seq_end[best_wp].link; sq; sq= sq->link) {
	    Fid *fid_ptr;
	    int	count;

	    /* delete more fragmented frame ids */
	    count= sq->n_act;
	    for(fid_ptr= sq->frame_id, j=0; j < sq->n_act; fid_ptr++, j++) {
		if( fid_ptr->count == n_frames ) continue;
		fid_ptr->count= 0;
		count--;
	    }
	    if( count < 1 ) {
		/* delete sequence */
		sq->n_act= 0;
		continue;
	    }

    	    /* find best priority */
	    if( !best_seq ) best_seq= sq;
    	    if( gram->max_pri ) {
	        if( cmp_pri(sq->pri, best_seq->pri, gram->max_pri )> 0 )
			best_seq= sq;
    	    }
    	}
    }
    if( !best_seq ) {
	printf("No parse\n\n");
	return;
    }

    /* copy sequences to parses buffer */
    num_seqs= 0;
    /* copy only best parse */
    if( MAX_PARSES == 1 ) {
	expand_seq(best_seq, n_slots);
    }
    /* copy all parses */
    else {
	SeqNode *sq;

	/* for each sequence */
	for( sq= seq_end[best_wp].link; sq; sq= sq->link) {
	    if( !sq->n_act ) continue;
    	    expand_seq(sq, n_slots);
    	}
    }

    /* add frame ids */
    map_to_frames(pbuf, gram);

    /* update history list */
    if( num_parses >0 ) {
        SeqNode *s;

	for(i=0, s= parses[0];i<n_slots; i++, s++) {
	    if( ++hi > (HIST_LEN-1) ) hi= 0;
	    history[hi]=  s->n_act;
	}
    }

    *print_line_ptr = 0;

    return;
}



void expand_seq(seq, ns)
    SeqNode *seq;
    int ns;
{
    SeqNode *sn, **pn;
    int	col;

        /* add new seq to set of parses */
	pn= pbuf  + (num_seqs*ns) +ns-1;
	if( pn >= pbuf_end ) {
	    fprintf(stderr,
	            "ERROR: overflow ParseBufSize %d\n", ParseBufSize);
	    return;
	}
        for( col= ns-1, sn= seq; col >= 0; col--, sn= sn->back_link) {
	    *pn-- = sn;
	}
	num_seqs++;
}

/* add frame labelling to slot seq, creating alts when ambiguous */
void map_to_frames(SeqNode **seq_start, Gram *gram)
{
    int		i, j, p,
		fc1[500], fc2[500],
		last_set[ HIST_LEN],
		li,
		min1,min2;
    SeqNode 	**seq, *s;

    if( verbose > 3 ) printf("num_seqs= %d\n", num_seqs); fflush(stdout);
    num_parses= 0;
    /* for each slot sequence */
    for(p=0; p <num_seqs; p++) {
	seq= pbuf + (n_slots * p);

	/* generate all frame labellings for seq */
    	for(i=0;i<n_slots; i++) {fr_seq[i]= -1; sl_seq[i]= (SeqNode *)0;}
    	i= 0;
	if( verbose > 3 ) printf("map seq %d\n", p);
    	extend_map(seq, i);
    }

    /* generate fragmentation counts */
    min1= min2= 1000;
    for(p=0; p <num_parses; p++) {
	fc1[p]= fc2[p]=0;
	for(i=0; i < HIST_LEN; i++) { last_set[i]= history[i]; }
	li= hi;
	for(i=0, s= parses[p];i<n_slots; i++, s++) {
	    if(last_set[li] != (int) s->n_act )  fc1[p]++;
	    for(j=0; j<HIST_LEN; j++) if( last_set[j] == s->n_act ) break;
	    if( j == HIST_LEN )  fc2[p]++;
	    if( ++li > (HIST_LEN-1) ) li= 0;
	    last_set[li]=  s->n_act;
	}
	if( fc1[p] < min1 ) {
	    min1= fc1[p];
	    min2= fc2[p];
	}
	else if( fc1[p] == min1 ) {
	    if( fc2[p] < min2 ) min2= fc2[p];
	}
    }

    /* prune more fragmented parses */
    for(p=0; p <num_parses; p++) {
	if( (fc1[p] == min1 ) && ( fc2[p] == min2 )) continue;
	free(parses[p]);
	parses[p]=0;
    }
    for(p=0, i=0; p <num_parses; p++) {
	if( parses[p] ) parses[i++]= parses[p];
    }
    num_parses= i;

    if( verbose > 3 ) {
	for(p=0; p <num_parses; p++) {
	    for(i=0, s= parses[p];i<n_slots; i++, s++) printf("%d ", s->n_act);
	    printf(" %d %d\n",  fc1[p], fc2[p]);
	}
    }
}

void extend_map(SeqNode **seq, int i)
{
    int j;
    SeqNode *s;

   if( i >= n_slots ) {
	if( !(parses[num_parses]= (SeqNode *)malloc(n_slots*sizeof(SeqNode)))){
	    printf("malloc parse failed\n");
	    exit(-1);
	}
	for(i=0, s=parses[num_parses]; i< n_slots; i++, s++) {
 	    *s= *sl_seq[i];
	    s->n_act= fr_seq[i];
	}
	num_parses++;
	return;
   }

    sl_seq[i]= *seq;
    for( j=0; j < gram->num_frames; j++ ) {
	if( !net_in_frame((*seq)->edge->net, j, gram) ) continue;
	fr_seq[i]= j;
	extend_map(seq+1, i+1);
    }
   
}



FrameNode *get_new_frame()
{
    if( frame_buf_ptr == frame_buf_end ) {
	fprintf(stderr, "ERROR: overflow FrameBufSize  %d\n", FrameBufSize);
	return((FrameNode *)0);
    }
    return(frame_buf_ptr++);
}


int net_in_frame(int net, int frame, Gram *gram)
{
    int	j;

    /* see if net applies to frame */
    for(j=0; j < gram->frame_def[frame].n_slot; j++) {
	if( net == gram->frame_def[frame].slot[j] ) break;
    }
    /* if net doesn't apply */
    if( j == gram->frame_def[frame].n_slot) return(0);
    return(1);
}


SeqNode *get_new_slot()
{
    if( seq_buf_ptr == seq_buf_end ) {
	fprintf(stderr, "ERROR: overflow SeqBufSize  %d\n", SeqBufSize);
	return((SeqNode *)0);
    }
    return(seq_buf_ptr++);
}

int add_to_seq(int net, int start_word, Gram *gram)
{
    int wp;
    int ew;
    Edge *edge;

    /* find pointer for edges for net, sw */
    if( check_chart(net, start_word, &edge) <= 0 ) return(0);

    /* find closest end_word position to abut to */
    for(wp= start_word-1; wp && (!seq_end[wp].link); wp--);

    /* add all versions (different end points) for net */
    /* don't add alternate parses for same start end point */
    ew= -1;
    for( ; edge; edge= edge->link) {
	if(ew == edge->ew) continue;
	if( append_slot(net, edge, wp, gram) < 0) return(-1); 
	ew= edge->ew;
    }
    return(0);
}


int append_slot( net, edge, prev_word_pos, gram)
    int net;
    Edge *edge;
    int prev_word_pos;
    Gram *gram;
{
    int phrase_score;
    unsigned short new_score,
		ns,
		sw, ew;
    SeqNode *new_slt, *link;
    Fid new_frame[100],
    	*id;
    int  new_count;
    int nf, prev_nf,
	num_extended,
	i, j;
    int new_nf;
    char	prune;

    /* compute score for extending path with slot */
    phrase_score= score_phrase(edge->sw, edge->ew);
    new_score =  (unsigned short) phrase_score;
    if (prev_word_pos) {
        new_score +=  seq_end[prev_word_pos].score;
        ns= seq_end[prev_word_pos].n_slots +1;
    }
    else ns= 1;
    
    ew= edge->ew;
    sw= edge->sw;
    prune= 0;

    /* used for breakpoint */
    if( ew > last_end ) last_end= ew;

    /* if no sequences already end at word_pos, just add it */
    if( !seq_end[ew].link ) {
	prune= 1;
    }

    /* compare score of words accounted for to existing paths */
    if( new_score < seq_end[ew].score ) return(0);
    if( new_score > seq_end[ew].score ) {
	/* prune old phrases ending at word_pos */
	prune= 1;
    }

    if( !prune ) {
	/* same score, check number of slots */
	if( ns > seq_end[ew].n_slots ) return(0);
	if( ns < seq_end[ew].n_slots ) {
	    /* prune old phrases ending at word_pos */
	    prune= 1;
	}
    }

    /* set number of frames for frame fragmentation */
    if( seq_end[ew].link ) nf= seq_end[ew].n_frames;
    else if( prev_word_pos ) nf= seq_end[prev_word_pos].n_frames +1;
    else nf= 1;
    if( prev_word_pos ) prev_nf= seq_end[prev_word_pos].n_frames;
    else prev_nf= 0;

    /* if new path better than current, prune current */
    if(prune) {
	seq_end[ew].link= (SeqNode *) 0;
        seq_end[ew].score= new_score;
        seq_end[ew].n_slots= ns;
    	if( prev_word_pos ) nf= seq_end[prev_word_pos].n_frames +1;
    	else nf= 1;
    }


    /* if partial paths being extended */
    if (prev_word_pos ) {
      SeqNode	*nl;
      Fid	*ff;
      int	min_f;

      /* for each path being extended */
      for(link= seq_end[prev_word_pos].link; link; link=link->link) {
	new_count= 0;
	min_f= 1000;
	/* try to extend each active frame state of the path */
	for( id= link->frame_id, j=0; j < link->n_act; j++, id++) {
	    if( id->id == gram->num_frames ) continue;
	    if( id->count < min_f) min_f= id->count;
	    if( id->count > (nf+1) ) continue;
	    if( !net_in_frame(net, id->id, gram) ) continue;

	    if( BIGRAM_PRUNE ) {
	    	/* don't add same slot-frame */
	    	for(nl= seq_end[ew].link; nl; nl= nl->link) {
		    if( nl->edge->net != net ) break;
		    for( ff= nl->frame_id, i=0; i < nl->n_act; i++, ff++) {
		        if( ff->id == id->id ) break;
		    }
		    if( i< nl->n_act ) break;
	        }
	        /* if same slot-frame exists */
	        if( nl && (nl->edge->net == net) ) {
		    if( ff->count <= id->count ) continue;
		    /* delete old slot-frame */
		    ff->id= gram->num_frames;
	        }
	    }

	    new_frame[new_count].count= id->count;
	    new_frame[new_count++].id= id->id;

	    if( verbose > 3 ) {
    		printf("append slot %s to frame %d ", gram->labels[net],id->id);
    		printf(" word %d  n_slots %d  n_frames %d\n",ew, ns, id->count);
	    }
	    if( id->count < nf ) nf= id->count;
	}

	/* add frames (containing slot) that were not active */
	if( (min_f +1) <= (nf+1) ) {
	    num_extended= new_count;
	    for(i=0; i < gram->num_frames; i++ ) {
	        if( !net_in_frame(net, i, gram) ) continue;
	        /* don't add if extended */
	        for(j=0; j < num_extended; j++) 
		    {if( new_frame[j].id == i ) break; }
	        if( j < num_extended ) continue;

	        if( BIGRAM_PRUNE ) {
	    	    /* don't add same slot-frame */
	    	    for(nl= seq_end[ew].link; nl; nl= nl->link) {
		        if( nl->edge->net != net ) break;
		        for( ff= nl->frame_id, j=0; j < nl->n_act; j++, ff++) {
		    	    if( ff->id == i ) break;
		        }
		        if( j< nl->n_act ) break;
	            }
	            /* if same slot-frame exists */
	            if( nl && (nl->edge->net == net) ) {
		        if( ff->count <= min_f+1 ) continue;
		        /* delete old slot-frame */
		        ff->id= gram->num_frames;
	            }
		}

	        new_frame[new_count].count= min_f +1;
	        new_frame[new_count++].id= i;
	    	if( (min_f+1) < nf ) nf= min_f+1;
if( verbose > 3 )
    printf("start slot %s frame %d word %d  n_slots %d  n_frames %d\n",
    	gram->labels[net], i, ew, ns, min_f+1);
	    }
	}
	/* if no new frames, don't add */
	if( !new_count ) continue;

	/* fill in new node */
    	if( !(new_slt= get_new_slot()) ) {
	   print_seq(seq_end[edge->ew].link, gram->labels);
	   return(-1);
	}
    	new_slt->edge= edge;
	new_slt->frame_id= fid_buf_ptr;
	new_slt->n_act= new_count;
	if( fid_buf_ptr + new_count >= fid_buf_end ) {
	    fprintf(stderr, "ERROR: overflow FidBufSize  %d\n", FidBufSize);
	    return(-1);
	}
	for(i=0; i<new_count; i++) {
	    *fid_buf_ptr++ = new_frame[i];
	}

    	new_slt->back_link= link;
	new_slt->n_frames= new_nf;

	/* link in node */
    	new_slt->link= seq_end[ew].link;
    	seq_end[ew].link= new_slt;

        /* calculate priority structure for new path */
        if( gram->max_pri ) {
    	    score_pri(link->pri, (int)edge->net, phrase_score, gram);
            /* fill in ptr to new priority structure */
	    new_slt->pri= pri_buf_ptr;
	    pri_buf_ptr += gram->max_pri;
	    if( (pri_buf_end - pri_buf_ptr) < gram->max_pri) {
		fprintf(stderr, "ERROR: overflow PriBufSize  %d\n", PriBufSize);
	        return(-1);
	    }
        }
      }
    }
    /* first slot in seq */
    else {
	/* add frames containing slot */
	new_count= 0;
	for(i=0; i < gram->num_frames; i++ ) {
	    if( !net_in_frame(net, i, gram) ) continue;
	    new_frame[new_count].count= 1;
	    new_frame[new_count++].id= i;

	}

	/* fill in and add node */
    	if( !(new_slt= get_new_slot()) ) {
	    print_seq(seq_end[edge->ew].link, gram->labels);
	    return(-1);
	}
    	new_slt->edge= edge;
	new_slt->frame_id= fid_buf_ptr;
	new_slt->n_act= new_count;
	if( fid_buf_ptr + new_count >= fid_buf_end ) {
	    fprintf(stderr, "ERROR: overflow FidBufSize  %d\n", FidBufSize);
	    return(-1);
	}
	for(i=0; i<new_count; i++) *fid_buf_ptr++ = new_frame[i];

    	new_slt->back_link= 0;
	new_slt->n_frames= 1;

	/* link in node */
    	new_slt->link= seq_end[ew].link;
    	seq_end[ew].link= new_slt;

    	/* calculate priority structure for new path */
    	if( gram->max_pri ) {
    	    score_pri( 0, (int)edge->net, phrase_score, gram);
            /* fill in ptr to new priority structure */
	    new_slt->pri= pri_buf_ptr;
	    pri_buf_ptr += gram->max_pri;
	    if( (pri_buf_end - pri_buf_ptr) < gram->max_pri) {
	        fprintf(stderr, "ERROR: overflow PriBufSize  %d\n", PriBufSize);
	        return(-1);
	    }
	}
    }

    seq_end[ew].score= new_score;
    seq_end[ew].n_slots= ns;
    seq_end[ew].n_frames= nf;
    return(0);
}

void breakpt_reset(int num_nets)
{

  /* reset edge buffer */
  edge_ptr= edge_buf;
  edge_link_ptr= edge_link_buf;
  pe_buf_ptr= pe_buf;

  clear_chart(num_nets);
}

void reset(int num_nets)
{
  int	i;

    breakpt_reset(num_nets);
    brk_pt= 0;
    last_end= 0;

    pri_buf_ptr= pri_buf;

    /* parse tree buffer */
    fid_buf_ptr= fid_buf;
    pstr_buf_ptr= pstr_buf;

    /* initialize slot sequence table */
    seq_buf_ptr= seq_buf;
    frame_buf_ptr= frame_buf;


    /* initialize seq_end table */
    for(i=0; i < script_len; i++) {
	seq_end[i].link= (SeqNode *)0;
	seq_end[i].score= 0;
	seq_end[i].n_slots= 0;
    }

    script_len = 1;
    num_parses= 0;

    /* initialize history */
    if( !USE_HISTORY ) {
	for(i=0;i< HIST_LEN; i++) history[i]= -1;
	hi= 4;
    }
}

/* initialize variables for start of session */
void start_session()
{
    int i;

    /* initialize history */
    for(i=0;i< HIST_LEN; i++) history[i]= -1;
    hi= 4;
}

void config(argc, argv)
    int argc;
    char **argv;
{
    FILE	*fp;

    /* set command line parms */
    if (pconf(argc,argv,conf,NULL,NULL,NULL))
	pusage(argv[0],conf),exit(-1);

    if( config_file ) {
	if( !(fp= fopen(  config_file, "r" )) ) {
	    fprintf(stderr,"WARNING: can't open config file %s\n", config_file);
	}
	else {
	    fpconf(fp,conf,NULL,NULL,NULL);
	    fclose(fp);
	}
    }

}

void init_parse(dir, dict_file, grammar_file, frames_file, priority_file)
    char *dir, *dict_file, *grammar_file, *frames_file, *priority_file;
{
    FILE	*fp;
    int		i;
    char	name[LABEL_LEN];

    /* read grammar */
    gram=read_grammar(dir, dict_file, grammar_file, frames_file, priority_file);
    cur_nets= (int *)0;
    num_active= 0;

    /* if several grammar used, use max values, this mallocs space */
    num_nets= gram->num_nets;
    num_frames= gram->num_frames;
    max_pri=gram->max_pri;


    /* malloc space for edge buffer */
    if( !(edge_buf= (Edge *) malloc( EdgeBufSize * sizeof(Edge)) ) ) {
	fprintf(stderr, "ERROR: can't allocate space for Edge buffer\n");
	exit(-1);
    }
    edge_buf_end= edge_buf + EdgeBufSize;
    
    /* malloc space for edge link buffer */
    if(!(edge_link_buf=(EdgeLink *) malloc( ChartBufSize * sizeof(EdgeLink)))){
	fprintf(stderr, "ERROR: can't allocate space for Edge Link buffer\n");
	exit(-1);
    }
    edge_link_end= edge_link_buf + ChartBufSize;
    
    /* malloc space for tree buffer */
    if(!(pe_buf=(Edge **) malloc( PeBufSize * sizeof(Edge *)))){
	fprintf(stderr,"ERROR: can't allocate space for edge pointer buffer\n");
	exit(-1);
    }
    pe_buf_end= pe_buf + PeBufSize;
    
    /* malloc space for Frame buffer */
    if( !(frame_buf=(FrameNode *)malloc(FrameBufSize * sizeof(FrameNode)))){
	fprintf(stderr, "ERROR: can't allocate space for Frame buffer\n");
	exit(-1);
    }
    frame_buf_end= frame_buf + FrameBufSize;

    /* malloc space for Frame id buffer */
    if( !(fid_buf=(Fid *)malloc(FidBufSize * sizeof(Fid)))){
	fprintf(stderr, "ERROR: can't allocate space for Frame id buffer\n");
	exit(-1);
    }
    fid_buf_end= fid_buf + FidBufSize;
    
    /* malloc space for parses */
    if(!(pbuf=(SeqNode **)
           malloc(ParseBufSize * sizeof(SeqNode **)))){
	fprintf(stderr, "ERROR: can't allocate space for parse buffer\n");
	exit(-1);
    }
    pbuf_end= pbuf + ParseBufSize;
    
    /* malloc space for Seq buffer */
    if( !(seq_buf=(SeqNode *)malloc(SeqBufSize * sizeof(SeqNode)))){
	fprintf(stderr, "ERROR: can't allocate space for Seq buffer\n");
	exit(-1);
    }
    seq_buf_end= seq_buf + SeqBufSize;
    
    /* malloc space for Seq end list */
    if( !(seq_end=(SeqCell *)calloc(InputBufSize, sizeof(SeqCell)))){
	fprintf(stderr, "ERROR: can't allocate space for Seq end list\n");
	exit(-1);
    }


    /* malloc space for script buffer */
    if( !(script=(int *)malloc(InputBufSize * sizeof(int)))){
	fprintf(stderr, "ERROR: can't allocate space for script buffer\n");
	exit(-1);
    }


    /* malloc space for chart */
    if(!(chart=(EdgeLink **) malloc( num_nets * sizeof(EdgeLink *)))){
	fprintf(stderr, "ERROR: can't allocate space for chart\n");
	exit(-1);
    }

    /* malloc space for pointers to slots to search for */
    if( !(active_slots= (int *) malloc(num_nets * sizeof(int))) ) {
	fprintf(stderr, "ERROR: malloc active list failed\n");
	exit(-1);
    }

    /* malloc space for Frame buffer */
    if( !(fr_list1=(FrameNode **)malloc(num_frames *sizeof(FrameNode *)))){
	fprintf(stderr, "ERROR: can't allocate space for Frame list\n");
	exit(-1);
    }
    
    /* malloc space for Frame buffer */
    if( !(fr_list2=(FrameNode **)malloc(num_frames * sizeof(FrameNode *)))){
	fprintf(stderr, "ERROR: can't allocate space for Frame list\n");
	exit(-1);
    }
    
    /* malloc space buffer for parse substrings */
    if( !(pstr_buf= (char *) calloc(StringBufSize, sizeof(char)))){
	fprintf(stderr, "ERROR: can't allocate space for String buffer\n");
	exit(-1);
    }
    pstr_buf_ptr= pstr_buf;
    pstr_buf_end= pstr_buf + StringBufSize;

    if( max_pri) {
    	/* allow for start at 0 */
	max_pri++;
    	/* malloc space for priority scores */
    	if( !(pri_buf=(unsigned short *)
		malloc(max_pri * PriBufSize * sizeof(unsigned short)))){
		fprintf(stderr,"ERROR: can't allocate space priority buf\n");
		exit(-1);
    	}
	pri_buf_ptr= pri_buf;
	pri_buf_end= pri_buf+ max_pri*PriBufSize;
    }


    /* if function words not to count in score */
    for(i=0; i < MAX_WRDS; i++) fun_wrds[i]= 0;
    if( function_wrd_file ) {
	if( (fp= fopen(function_wrd_file, "r")) ) {
	    while( fscanf(fp, "%s", name) == 1 ) {
		i= find_word(name, gram);
		if( i >= 0 ) fun_wrds[i]= 1;
	    }
	    fclose(fp);
	}
    }

    script[0]= 0;
    script_len = 1;

    start_session();

    /* clear internal lists and buffers */
    reset(num_nets);

}



int cmp_pri( p1, p2, max_pri )
    unsigned short *p1, *p2;
    int max_pri;
{
    int i;

    if( !p1 ) return(-1);
    if( !p2 ) return(1);
    for( i=0; i < max_pri; i++) {
	if( *(p1+i) > *(p2+i) ) return(1);
	if( *(p1+i) < *(p2+i) ) return(-1);
    }
    return(0);
}

void read_script(char *line, Gram *gram)
{
    char	*s, *wd, word[LABEL_LEN];
    int		first_word = 1,
		w_idx;

    /* find word numbers for utt start and end tokens */
    /* convert to uppercase */
    for(wd= start_sym; *wd; wd++)
	if( islower((int)*wd) ) *wd= (char) toupper((int)*wd);
    start_token= find_word(start_sym, gram);
    for(wd= end_sym; *wd; wd++)
	if( islower((int)*wd) ) *wd= (char) toupper((int)*wd);
    end_token= find_word(end_sym, gram);

    /* convert word strings to numbers and put in script array */
    /* word position starts numbering from 1 */
    script[0]= 0;
    script_len= 1;

    /* first "word" is start of utt token */
    script[script_len++]= start_token;

    /* skip blanks */
    s= line;
    while( *s == ' ' ) s++;

    /* for each word */
    while (sscanf(s, "%s", word) == 1) {
	if (*word == '+') {
	    /* noise event */
	}
	else {
	    if (!first_word) *print_line_ptr++ = ' ';
	    first_word = 0;
    	    /* convert to uppercase */
    	    for(wd= s; *wd; wd++)
		if( islower((int)*wd) ) *wd= (char) toupper((int)*wd);
	    wd = word;
	    if ((w_idx= find_word(word, gram)) == -1) {
		/* unknown word */
		*print_line_ptr++ = '-';
		while (*wd) *print_line_ptr++ = *wd++;
		if( !IGNORE_OOV ) script[script_len++]= w_idx;
	    }
	    else {
		if( script_len == InputBufSize) {
		    fprintf(stderr, "ERROR: overflow InputBufSize %d\n",
				InputBufSize);
		    return;
		}
		script[script_len++]= w_idx;
		while (*wd) *print_line_ptr++ = tolower(*wd++);
	    }
	}
	/* skip blanks */
	s= strchr(s, ' ');
	if(!s) break;
	while( *s == ' ' ) s++;
    } 

    /* append end of utt token as last "word" */
    script[script_len++]= end_token;
}



void score_pri(unsigned short *pri, int net, int score, Gram *gram)
{
    int i;
    int pp;

    pp= (int) gram->priorities[net];
    if( !pri ) {
        for(i=0; i < gram->max_pri; i++) {
	    if( i == pp ) *(pri_buf_ptr+i)= score;
	    else           *(pri_buf_ptr+i)=0;
        }
	return;
    }

    for(i=0; i < gram->max_pri; i++) {
	*(pri_buf_ptr+i)= *(pri+i);
	if( i == pp ) *(pri_buf_ptr+i) += score;
    }
}


void copy_chart(int low, int high)
{
    SeqNode *sn;
    int	wp;
    Edge *ep;

    for(wp= high; wp > low; wp--) {
	for(sn= seq_end[wp].link; sn; sn= sn->link) {
	    if( !(ep= copy_tree(sn->edge)) ) return;
	    else sn->edge= ep;
	}
    }
}

Edge *copy_tree(Edge *edge)
{
    int	i;
    Edge *ep,
	 **to_ep, **from_ep,
	 **tp;

    /* if already copied, return pointer to copy */
    if( edge->net == NO_SCORE ) return(edge->link);

    /* malloc space for new edge */
    if( !(ep= (Edge *)malloc(  sizeof(Edge)))) {
	fprintf(stderr, "ERROR: Out of space saving edges\n");
	return(0);
    }

    /* fill in from old edge */
    *ep= *edge;
    ep->link= 0;
    
    if( !edge->nchld ) {
	ep->nchld= 0;
	ep->chld= (Edge **)0;
    }
    /* copy children */
    else {
        /* malloc space for pointers to children */
        if( !(tp= (Edge **)malloc(  edge->nchld * sizeof(Edge *)))) {
	    fprintf(stderr, "ERROR: Out of space saving edges\n");
	    return(0);
        }
	ep->nchld= edge->nchld;
	ep->chld= tp;
	for(i=0, to_ep= tp, from_ep= edge->chld; i < edge->nchld; i++) {
	    if( !( *to_ep = copy_tree(*from_ep) ) ) return(0); 
	    to_ep++;
	    from_ep++;
	}
    }

    edge->net= NO_SCORE;
    edge->link= ep;
    return(ep);
}

void print_profile()
{
	gettimeofday( &end_tp, &end_tzp );
	msec= (match_end_tp.tv_sec - match_start_tp.tv_sec) * 1000;
	msec += (match_end_tp.tv_usec - match_start_tp.tv_usec) / 1000;
	printf("match time %ld  milliseconds\n", msec);
	msec= (end_tp.tv_sec - start_tp.tv_sec) * 1000;
	msec += (end_tp.tv_usec - start_tp.tv_usec) / 1000;
	printf("parse time %ld  milliseconds\n", msec);
	printf("Number of parses= %d\n", num_seqs);
	printf("Number of slots in parse= %d\n", n_slots);

	if( PROFILE > 1 ) {
	    printf("EdgeBufSize= %d  \tused= %d \t%4.2fM\n",
		EdgeBufSize, edge_ptr-edge_buf,
     		( EdgeBufSize * sizeof(Edge))/1000000.0);
	    printf("ChartBufSize= %d  \tused= %d \t%4.2fM\n",
		ChartBufSize, edge_link_ptr-edge_link_buf,
	 	(ChartBufSize * sizeof(EdgeLink))/1000000.0);
	    printf("PeBufSize= %d  \tused= %d  \t%4.2fM\n",
		PeBufSize, pe_buf_ptr-pe_buf,
    		( PeBufSize * sizeof(Edge *))/1000000.0);
	    printf("InputBufSize= %d  \tused= %d  \t%4.2fM\n",
		InputBufSize, script_len,
    		(InputBufSize * sizeof(int))/1000000.0);
	    printf("ParseBufSize= %d  \tused= %d  \t%4.2fM\n",
		ParseBufSize, num_seqs * n_slots,
		(ParseBufSize * sizeof(SeqNode **))/1000000.0);
	    printf("FrameBufSize= %d  used= %d\n",
		FrameBufSize, frame_buf_ptr-frame_buf);
	    printf("SlotSeqLen= %d  used= %d\n", SlotSeqLen, n_slots);
	    printf("FidBufSize= %d  \tused= %d  \t%4.2fM\n",
		FidBufSize, fid_buf_ptr-fid_buf,
    		(FidBufSize * sizeof(Fid))/1000000.0);
	    printf("PriBufSize= %d  \tused= %d  \t%4.2fM\n",
		PriBufSize, pri_buf_ptr-pri_buf,
    		(PriBufSize * sizeof(unsigned short))/1000000.0);
	    printf("SeqBufSize= %d  \tused= %d  \t%4.2fM\n",
		SeqBufSize, seq_buf_ptr - seq_buf,
    		(SeqBufSize * sizeof(SeqNode))/1000000.0);

	}

}

void print_parse(int parse_num, char *out_str, int extract, Gram *gram)
{
    int	j,
	frame;
    SeqNode	*fptr;
    char	*out_ptr;

    if( parse_num >= num_parses ) return;

    out_ptr= out_str;
    frame= -1;

    fptr= parses[parse_num];
    for(j=0; j < n_slots; j++, fptr++) {
	/* if starting a new frame */
	if( fptr->n_act != frame ) {
	    frame= fptr->n_act;
	    if( (frame < 0) || (frame > gram->num_frames) ) {
		fprintf(stderr, "ERROR: frame  id out of range %d\n", frame);
		exit(-1);
	    }
	}

	/* print slot tree */
	if( extract ) {
	    sprintf(out_ptr, "%s:", gram->frame_name[frame]);
	    out_ptr += strlen(out_ptr);
	    first_concept= 1;
	    out_ptr= print_extracts( fptr->edge, gram, out_ptr, 0, 
		gram->frame_name[frame]);
	    if( !out_ptr ) { printf("\n"); }
	    else  {
		sprintf(out_ptr, "\n");
		out_ptr += strlen(out_ptr);
	    }
	}
	else {
	    sprintf(out_ptr, "%s:", gram->frame_name[frame]);
	    out_ptr += strlen(out_ptr);
	    out_ptr= print_edge( fptr->edge, gram, out_ptr);
	    if( out_ptr ) {
		sprintf(out_ptr, "\n");
		out_ptr += strlen(out_ptr);
	    }
	    else printf("\n");
	}
    }

    fflush(stdout);
}



