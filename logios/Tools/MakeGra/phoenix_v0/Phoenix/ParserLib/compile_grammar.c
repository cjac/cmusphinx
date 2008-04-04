/* take input grammar and generate nets

   input: file containing list of nets to compile is first arg
          dictionary is second argument
   output: <file>.net files
*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>	  
#include <ctype.h>
#include "grammar.h"
#include "pconf.h"


/* command line parameters, with defaults */
char	*grammar = ".",
	*netsfile = "nets",
	*dictfile = "base.dic",
  	*grammar_file = NULL;
int	SymBufSize=   50000;	/* buffer size to hold char strings */
int	TokBufSize=   10000;	/* buffer size to hold char strings */
int	MaxNonTerm=    100000;	/* max number non-terminal pointers */
int	MaxSymbol=     10000;	/* max number of nonterminal symbols */
int	MaxNfa=        150000;	/* max number of nodes in network */
int	MaxSucLink=    500000;	/* max number of arcs in network */

/* Command Line structure */

  static config_t conf[] = {
  {"grammar",	"dir containing .gra files",
       "-g", 	STRING,	(caddr_t) &grammar},
  {"netsfile",	"filename for list of nets",
       "-n", 	STRING,	(caddr_t) &netsfile},
  {"dictfile",	"file containing dictionary",
       "-d", 	STRING,	(caddr_t) &dictfile},
  {"grammar_file",	"single grammar file to compile",
       "-f", 	STRING,	(caddr_t) &grammar_file},
  {"SymBufSize", "Size in bytes of symbol buffer for dictionary", 
       "-SymBufSize",	INT,	(caddr_t) &SymBufSize},
  {"TokBufSize", "Size in bytes of symbol buffer for dictionary", 
       "-TokBufSize",	INT,	(caddr_t) &TokBufSize},
  {"MaxNonTerm", "Size in bytes of symbol buffer for dictionary", 
       "-MaxNonTerm",	INT,	(caddr_t) &MaxNonTerm},
  {"MaxSymbol", "Size in bytes of symbol buffer for dictionary", 
       "-MaxSymbol",	INT,	(caddr_t) &MaxSymbol},
  {"MaxNfa", "Size in bytes of symbol buffer for dictionary", 
       "-MaxNfa",	INT,	(caddr_t) &MaxNfa},
  {"MaxSucLink", "Size in bytes of symbol buffer for dictionary", 
       "-MaxSucLink",	INT,	(caddr_t) &MaxSucLink},
  {NULL, NULL, NULL, NOTYPE, NULL}
  };


Gnode *new_nfa();
SucLink *new_nfa_link();

static void alloc_sp ();
static void net_gen ();
void rewrite();
void add_nt();
int find_sym();
int get_symbol();
int get_right ();
int blank_line();
int pconf();
void pusage();
void write_net();
void reset();
int find_net();
int lookup_word();
int add_word();


Gnode	*nfa,
	*nfa_ptr,
	*nfa_end;

SucLink *nfa_succ,
	*nfa_suc_ptr,
	*nfa_suc_end;

int	NetNumber;
char	NetName[LABEL_LEN];
char	NetSym[LABEL_LEN];
char	fname[LABEL_LEN];

non_term *non_t;
int num_nt;

char	*sym_buf,	/* buff for ascii input strings */
	*sym_ptr,
	*sym_buf_end;
char	*tok_buf,	/* buff for ascii input strings */
	*tok_ptr,
	*tok_buf_end;
char **sym;		/* symbol table */
int num_sym;
int last_net;
int num_nets;
char	state;
int	nt_tok;

char	*wrds[MAX_WRDS];	/* pointers to strings for word numbers */
char	update_dict;

char rule_buf[LINE_LEN];
char *rule_ptr;

FILE	*fp_out;

int main( argc, argv)
    int argc;
    char *argv[];
{
    int 	net_num,
		i, j;
    char	net_name[LABEL_LEN];
    FILE	*fp, *fp_nets;


    /* set command line parms */
    if (pconf(argc,argv,conf,NULL,NULL,NULL)) pusage(argv[0],conf),exit(-1);
    if( !grammar_file ) {
	printf("Usage: Must specify output file with -f option\n");
	exit(-1);
    }

    /* malloc space for data structures */
    alloc_sp ();

    nfa_ptr= nfa;
    nfa_suc_ptr= nfa_succ;
    num_nt = 0;
    state= 'n';

    /* leave null symbol for net 0 */
    sym[0]= sym_ptr;
    strcpy(sym_ptr, "*");
    sym_ptr += strlen(sym_ptr)+1;
    num_sym= 1;


    /* remove dictionary */
    sprintf(fname, "%s/%s", grammar, dictfile);
    if( !(fp= fopen(fname, "w")) ) {
	fprintf(stderr, "ERROR: can't create %s\n", fname);
	exit(-1);
    }
    fclose (fp);

    /* open file with list of nets */
    sprintf(fname, "%s/%s", grammar, netsfile);
    if( !(fp_nets= fopen(fname, "r")) ) {
	fprintf(stderr, "ERROR: can't open %s\n", fname);
	exit(-1);
    }
    /* read net names into symbol table */
    for(net_num = 1; fscanf(fp_nets, "%s", net_name) == 1; net_num++) {
	/* add braces to net name */
	sprintf(fname, "[%s]", net_name);
	/* check for duplicate net names */
	for(j=1; j < num_sym; j++ ) {
	    if( !strcmp(sym[j], fname) ) {
		fprintf(stderr, "Duplicate net name %s\n", fname);
		exit(-1);
	    }
	}
	sym[num_sym++]= sym_ptr;
	if( (sym_ptr + strlen(fname) +1) >= sym_buf_end ) {
	    printf("Buffer overflow, SymBufSize= %d\n", SymBufSize);
	    exit(1);
	}
	strcpy( sym_ptr, fname );
	sym_ptr += strlen(fname) +1;
    }
    last_net= num_sym-1;
    num_nets= net_num-1;

    /* open output .net file */
    sprintf(fname, "%s/%s.net", grammar, grammar_file);
    if( !(fp_out = fopen(fname,"w")) ) {
	printf("can't open %s\n", fname);
	exit(1);
    }
    /* write number of nets at start of file */
    fprintf(fp_out, "Number of Nets= %d\n", num_nets);

    /* use input .gra file with same stem name */
    sprintf(fname, "%s/%s.gra", grammar, grammar_file);

    /* read grammar rules and compile networks */
    net_gen(fname, fp_out);

    fclose(fp_out);

    if( update_dict ) {
    	/* rewrite dictionary */
        sprintf(fname, "%s/%s", grammar, dictfile);
    	if( !(fp= fopen(fname, "w") ) ) {
      	    printf("open %s failed.\n", dictfile);
      	    exit(-1);
    	}
    	for(i=1; i < MAX_WRDS; i++) {
      	    if( !wrds[i] ) continue;
      	    fprintf(fp, "%s %d\n", wrds[i], i);
    	}
    	fclose(fp);
    }
    return(1);
}

static void net_gen (char *filename, FILE *fp_out)
{
    FILE	*fp_in;
    SucLink	*arc;
    Gnode	*start_node, *end_node;
    int		s_idx;
    int		nt;
    char	*s;
    char	fname[LABEL_LEN];
    char	name[LABEL_LEN];
  

    /* open grammar file */
    if( !(fp_in = fopen(filename,"r")) ) {
	printf("ERROR: Can't open grammar file %s\n", filename);
	return;
    }

    /* generate non-deterministic FSA by re-writing non-terminals */

    /* read each line from gra file */
    while( fgets(rule_buf, LINE_LEN, fp_in) ) {

	if( blank_line(rule_buf) ) continue;

	/* include file */
	if( !strncmp(rule_buf, "#incl", 5) ) {
	    sscanf(rule_buf, "#include %s", name);
	    sprintf(fname, "%s/%s", grammar, name);
	    net_gen(fname, fp_out);
	    continue;
	}

	/* comment */
	else if( *rule_buf == '#' ) continue;

	/* net definition end */
	else if( *rule_buf == ';' ) {
	    if( state != 'n' ) {
		/* write out compiled net */
		write_net();
		/* reset structures */
		reset();
		state= 'n';
	    }
	    continue;
	}

	/* net name, begin net definition */
	else if( *rule_buf == '[' ) {

	    if( state != 'n' ) {
		printf("ERROR: Unexpected net definition %s\n", rule_buf);
		continue;
	    }

	    /* set net number */
	    sscanf(rule_buf, "%s", NetName);
	    NetNumber= find_net( NetName );

	    /* replace square brackets with angle brackets */
	    s= strchr(rule_buf, '[');
	    if(s) *s= '<';
	    s= strchr(rule_buf, ']');
	    if(s) *s= '>';
	    sscanf(rule_buf, "%s", NetSym);
	    s_idx= find_sym( NetSym );

	    /* create start and end nodes */
	    start_node= new_nfa();
	    start_node->n_suc= 1;
	    start_node->final= 0;
	    end_node= new_nfa();
	    end_node->n_suc= 0;
	    end_node->final= 1;
    
	    /* arc between labelled with net name */
	    arc= new_nfa_link();
	    arc->succ.tok= s_idx;
	    arc->succ.state= end_node;
	    arc->succ.call_net= 0;
	    arc->link= 0;
	    arc->nt= 1;
	    start_node->succ= (struct gsucc *) arc;

	    add_nt( s_idx, arc );
	    rule_ptr= rule_buf;
	    nt_tok= get_symbol();
	    state= 'p';
	    continue;
	}

	/* rewrite rule */
	else if( isspace( (int) *rule_buf) ) {
            if( !index(rule_buf, (int) '(' ) ||
                !index(rule_buf, (int) ')' ) ) {
		printf("ERROR: Bad format for rule: %s\n", rule_buf);
		continue;
	    }
    	    /* rewrite each previous occurence of symbol with rhs */
    	    for( nt= 0; nt < num_nt; nt++) {
      		if( non_t[nt].tok == nt_tok ) {
        	    rewrite( non_t[nt].arc );
        	    /* mark as rewritten */
        	    non_t[nt].rw++;
      		}
    	    }
	}

	/* macro definition */
	else if( isupper( (int) *rule_buf) ) {
	    rule_ptr= rule_buf;
	    if( (nt_tok= get_symbol()) < 0) continue;
	    state= 'e';
	}

	/* error */
	else {
	    printf("ERROR: bad format: %s\n", rule_buf);
	}

    }

    fclose(fp_in);
}

void rewrite( arc)
    SucLink *arc;
{
    int tok, tok1;
    SucLink	*alt,
		*new_arc,
		*link,
		*start;

    Gnode	*state,
		*from_state;
    char opt, opt1,
	 self, self1,
	 type, type1,
	 prev_self;

    /* set rule_ptr to start of right hand side */
    if( !(rule_ptr= index(rule_buf, (int) '(' )) ) return;
    rule_ptr++;
    from_state= 0;
    start= 0;
    prev_self=1;


    if( (tok= get_right(&opt, &self, &type)) < 0 ) return;

    /* while more tokens remaining */
    while( (tok1= get_right(&opt1, &self1, &type1)) >= 0 ) {

	/* create arc for token */
	alt= new_nfa_link();
	alt->succ.tok= tok;
	alt->succ.call_net= 0;
	alt->nt= 0;
	alt->link= 0;

        if( type == 1) {
	    /* if non_term token */
	    alt->nt= 1;
	    add_nt( tok, alt );
        }
        else if( type == 2 ) {
	    /* if pre_term token */
	    alt->succ.call_net= tok;
        }

	if( self && opt ) {
	    /* if start or prev state contained self trans add null arc */
	    if ( !from_state || prev_self ) {
		/* create next state */
		state= new_nfa();

		/* add null arc to state */
	        new_arc= new_nfa_link();
	        new_arc->succ.tok= 0;
	        new_arc->succ.call_net= 0;
                new_arc->succ.state= state;
	        new_arc->nt= 0;
		if( from_state ) {
	    	    new_arc->link= (SucLink *) from_state->succ;
	    	    from_state->succ= (struct gsucc *) new_arc;
		}
	        /* if first arc from start node */
	        if( !start ) start= new_arc;

		/* add self transition */
		alt->succ.state= state;
	        alt->link= (SucLink *) state->succ;
	        state->succ= (struct gsucc *) alt;

	    }
	    else {
		/* add self transition */
		alt->succ.state= from_state;
	        alt->link= (SucLink *) from_state->succ;
	        from_state->succ= (struct gsucc *) alt;
	    }
	}
	else {
	    /* if first arc from start node */
	    if( !start ) start= alt;

	    /* create next state */
	    state= new_nfa();
	    /* point arc at state */
	    alt->succ.state= state;

	    /* if not start of rule, link in to state */
	    if( from_state ) {
	        alt->link= (SucLink *) from_state->succ;
	        from_state->succ= (struct gsucc *) alt;
	    }

	    if( opt ) {
	        /* add null arc to next state */
	        new_arc= new_nfa_link();
	        new_arc->succ.tok= 0;
	        new_arc->succ.call_net= 0;
                new_arc->succ.state= state;
	        new_arc->nt= 0;
	        new_arc->link= alt->link;
	        alt->link= new_arc;
	    }
            if( self ) {
	        /* add self-transition */
	        new_arc= new_nfa_link();
		*new_arc= *alt;
        	if( type == 1) {
	    	    /* if non_term token */
	    	    add_nt( tok, new_arc );
        	}
	        new_arc->link= (SucLink *) state->succ;
	        state->succ= (struct gsucc *) new_arc;
	    }
	}
	from_state= state;
	prev_self= self;
	tok= tok1;
	opt= opt1;
	self= self1;
	type= type1;
	
    }

    /* create ending arc */
    alt= new_nfa_link();
    if( !start ) start= alt;
    alt->succ.tok= tok;
    alt->succ.call_net= 0;
    alt->succ.state= arc->succ.state;
    alt->nt= 0;
    alt->link= 0;

    if( type == 1) {
	/* if non_term token */
	alt->nt= 1;
	add_nt( tok, alt );
    }
    else if( type == 2 ) {
	/* if pre_term token */
	alt->succ.call_net= tok;
    }

    if( self && opt ) {
	/* if prev state contained self transition add null arc */
	if ( !from_state || prev_self ) {
	    /* create next state */
	    state= new_nfa();

	    /* add null arc to state */
	    new_arc= new_nfa_link();
	    new_arc->succ.tok= 0;
	    new_arc->succ.call_net= 0;
            new_arc->succ.state= state;
	    new_arc->nt= 0;
	    if( from_state ) {
	    	new_arc->link= (SucLink *) from_state->succ;
	    	from_state->succ= (struct gsucc *) new_arc;
	    }

	    /* add self transition */
	    alt->succ.state= state;
	    alt->link= (SucLink *) state->succ;
	    state->succ= (struct gsucc *) alt;
	}
	else {
	    /* add self transition */
	    alt->succ.state= from_state;
	    alt->link= (SucLink *) from_state->succ;
	    from_state->succ= (struct gsucc *) alt;
	    state= from_state;
	}

	/* create null arc to final state */
	new_arc= new_nfa_link();
	new_arc->succ.tok= 0;
    	new_arc->succ.state= arc->succ.state;
	new_arc->succ.call_net= 0;
	new_arc->nt= 0;
	new_arc->link= (SucLink *) state->succ;
	state->succ= (struct gsucc *) new_arc;
    }
    else {
	/* if not start of rule, link in to state */
	if( from_state ) {
	    alt->link= (SucLink *) from_state->succ;
	    from_state->succ= (struct gsucc *) alt;
	}

        if( self) {
	    /* create next state */
	    state= new_nfa();
	    alt->succ.state= state;

	    /* add self transition */
	    new_arc= new_nfa_link();
	    *new_arc= *alt;
    	    if( type == 1) add_nt( tok, new_arc );
	    new_arc->link= (SucLink *) state->succ;
	    state->succ= (struct gsucc *) new_arc;

	    /* create null arc to final state */
	    new_arc= new_nfa_link();
	    new_arc->succ.tok= 0;
    	    new_arc->succ.state= arc->succ.state;
	    new_arc->succ.call_net= 0;
	    new_arc->nt= 0;
	    new_arc->link= (SucLink *) state->succ;
	    state->succ= (struct gsucc *) new_arc;
	}
	else {
	    alt->succ.state= arc->succ.state;
	    if( opt ) {
		/* if optional, add null arc */
		new_arc= new_nfa_link();
		new_arc->succ.tok= 0;
		new_arc->succ.call_net= 0;
		new_arc->succ.state= alt->succ.state;
		new_arc->nt= 0;
		new_arc->link= alt->link;
		alt->link= new_arc;
	    }
	}
    }

    /* link in start arc */
    if( !arc->link ) {
	arc->link= start;
    }
    else {
	for( link= arc->link; link->link; link= link->link) ;
	link->link= start;
    }
    
}


static void alloc_sp ()
{
  /* allocate space for symbol buffer */
  if( !(sym_buf= (char *) malloc(SymBufSize)) ) {
    printf("not enough memory to allocate symbol buffer\n");
    exit(1);
  }
  sym_buf_end= sym_buf + SymBufSize;
  sym_ptr= sym_buf;

  /* allocate space for non-term symbol buffer */
  if( !(tok_buf= (char *) malloc(TokBufSize)) ) {
    printf("not enough memory to allocate symbol buffer\n");
    exit(1);
  }
  tok_buf_end= tok_buf + TokBufSize;
  tok_ptr= tok_buf;

  /* allocate space for non-term array */
  if( !(non_t= (non_term *) calloc(MaxNonTerm, sizeof(non_term))) ) {
    printf("not enough memory to allocate non-term array\n");
    exit(1);
  }

  /* allocate space for symbol table */
  if( !(sym= (char **) calloc(MaxSymbol, sizeof(char *))) ) {
    printf("not enough memory to allocate non-term array\n");
    exit(1);
  }

  /* allocate space for nfa */
  if( !(nfa= (Gnode *) calloc(MaxNfa, sizeof(Gnode))) ) {
    printf("not enough memory to allocate nfa\n");
    exit(1);
  }
  nfa_end= nfa + MaxNfa;
  if( !(nfa_succ= (SucLink *) malloc(MaxSucLink * sizeof(SucLink))) ) {
    printf("not enough memory to allocate nfa_succ\n");
    exit(1);
  }
  nfa_suc_end= nfa_succ + MaxSucLink;
}


int get_right(opt, self, type)
    char *opt, *self, *type;
{
    char symbol[LABEL_LEN];
    char *sym_ptr = symbol;
    char *sym_end = symbol+LABEL_LEN;
    int w_idx;

    /* skip white space */
    if( !*rule_ptr ) return(-1);
    while( *rule_ptr &&  isspace( (int) *rule_ptr ) ) rule_ptr++;
    if( *rule_ptr == ')' ) return(-1);

    /* check optional and self-transition flags */
    *opt= 0;
    *self= 0;
    for( ; 1; rule_ptr++ ) {
        if( *rule_ptr == '*' ) *opt= 1;
        else if( *rule_ptr == '+' ) *self= 1;
	else break;
    }

    /* copy symbol */
    while(!isspace((int)*rule_ptr) && (sym_ptr< sym_end) && (*rule_ptr != ')'))
	*sym_ptr++ = *rule_ptr++;
    if( sym_ptr >= sym_end ) {
	printf("ERROR: symbol too long on right side.\n");
	return(-1);
    }
    *sym_ptr= '\0';

    if( *symbol == '[' ) {
	/* net call */
	int	symi;
 
	*type= 2;
	symi= find_sym(symbol);
	if( symi == (num_sym-1) ) {
	    printf("WARNING: compiling net %s could not find called net %s\n",
			 NetName, symbol);
	}
	return( symi );
    }
    else if( isupper((int) *symbol) ) {
	/* non-terminal */
	*type= 1;
	return( find_sym( symbol ) );
    }
    else {
	/* word */
	*type= 0;
	w_idx= lookup_word(symbol);
	if( w_idx < 0) {
	    w_idx= add_word(symbol);
	    update_dict= 1;
	}
	return( w_idx );
    }
}


int find_sym( s )
    char *s;
{
    int i;

    /* if null arc */
    if( !strcmp( s, sym[0] ) )  return(0);

    for(i=num_sym-1; i >= 0; i-- )
	if( !strcmp( s, sym[i] ) ) break;
    if( i >=  0 ) return(i);

    /* install new symbol */
    if( num_sym >= MaxSymbol ) {
	printf("Table overflow, MaxSymbol= %d\n", MaxSymbol);
	exit(1);
    }
    if( (tok_ptr + strlen(s)+1 ) >= tok_buf_end ) {
	printf("Buffer overflow, TokBufSize= %d\n", TokBufSize);
	exit(1);
    }

    strcpy(tok_ptr, s);
    sym[num_sym]= tok_ptr;
    tok_ptr += strlen(s)+1;
    return( num_sym++);
}

Gnode *new_nfa()
{
    if( nfa_ptr >= nfa_end ) {
	printf("Table overflow, MaxNonTerm= %d\n", MaxNonTerm);
	exit(1);
    }

    nfa_ptr->n_suc = 0;
    nfa_ptr->final = 0;
    nfa_ptr->succ = NULL;

    return( nfa_ptr++);
}

SucLink *new_nfa_link()
{
    if( nfa_suc_ptr >= nfa_suc_end ) {
	printf("Table overflow, MaxSucLink= %d\n", MaxSucLink);
	exit(1);
    }

    nfa_suc_ptr->succ.tok = 0;
    nfa_suc_ptr->succ.state = NULL;
    nfa_suc_ptr->succ.call_net = 0;
    nfa_suc_ptr->link = NULL;
    nfa_suc_ptr->nt = 0;

    return( nfa_suc_ptr++);
}



void add_nt( token, arc )
    int token;
    SucLink *arc;
{

    if( num_nt >= MaxNonTerm ) {
	printf("Table overflow, MaxNonTerm= %d\n", MaxNonTerm);
	exit(1);
    }
    non_t[num_nt].tok= token;
    non_t[num_nt].rw= 0;
    non_t[num_nt++].arc= arc;
}


void write_net()
{
    Gnode	*state;
    SucLink	*arc;
    int		count,
    		tot_arcs;
    int	i;

#ifdef DEBUG
  {
    Gnode *state;
    SucLink *arc;

    printf("Symbol Table\n");
    for(i=0; i < num_sym; i++ ) printf("%s\n", sym[i]);
    printf("\n\n");

    printf("NFA Network\n");
    for(state= nfa, i=0; state < nfa_ptr; state++, i++) {
      printf("node %d  %d %d\n", i, state->n_suc, state->final);
      for( arc= (SucLink *) state->succ; arc; arc= arc->link ) {
        printf("\t\t%d    %d    %d\n", arc->succ.tok,
               arc->succ.call_net, arc->succ.state - nfa);
      }
    }
  }
#endif

    /* check to see all nt rewritten */
    for(i=0; i < num_nt; i++) {
    	if( !non_t[i].rw ) {
      	    printf("WARNING: In <%s> NonTerm %s not rewritten\n",
                 NetName, sym[ non_t[i].tok ]);
    	}
    }

    /* write net name, net number, number of states and concept_flag */
    fprintf(fp_out, "%s %d %d %d\n", NetName, NetNumber, nfa_ptr - nfa, 0);

    /* generate final net by removing non-terminal arcs */
    tot_arcs= 0;
    for(state= nfa, i=0; state < nfa_ptr; state++, i++) {
      /* count succs */
      for(arc= (SucLink *)state->succ,count=0; arc; arc= arc->link) {
        if( !arc->nt ) count++;
      }
      fprintf(fp_out, "%d  %d %d\n", i, count, state->final);
      for( arc= (SucLink *) state->succ; arc; arc= arc->link ) {
        if( arc->nt ) continue;
        fprintf(fp_out, "\t\t%d    %d    %d\n", arc->succ.tok,
                arc->succ.call_net, arc->succ.state - nfa);
      }
      tot_arcs += count;
    }
    printf("%s  %d states  %d arcs\n", NetName, nfa_ptr - nfa, tot_arcs);

}

void reset()
{
    num_sym= last_net +1;
    tok_ptr= tok_buf;
    nfa_ptr= nfa;
    nfa_suc_ptr= nfa_succ;
    num_nt = 0;
}


int find_net( s )
    char *s;
{
    int i;

    for(i=1; i <= last_net; i++ )
	if( !strcmp( s, sym[i] ) ) break;
    if( i <=  last_net ) return(i);

    printf("WARNING: net %s not found\n", s);
    fflush(stdout);
    return(-1);
}

int lookup_word(char *s)
{
    unsigned short key;
    char *c;
    int idx;

    /* convert to uppercase */
    for(c= s; *c; c++) if( islower((int)*c) ) *c= (char) toupper((int)*c);

    /* use first two chars as hash key */
    c= (char *) &key;
    *c = *s;
    c++;
    *c = *(s+1);

    idx= key;
    for( ; idx < MAX_WRDS; idx++) {
	/* if unknown word */
	if( !wrds[idx] ) return(-1);
	if( !strcmp(wrds[idx], s) ) return(idx);
    }
    return(-1);
}


/* same as lookup_word, but adds word if not found */
int add_word(char *s)
{
    unsigned short key;
    char *c;
    int idx;

    /* convert to uppercase */
    for(c= s; *c; c++) if( islower((int)*c) ) *c= (char) toupper((int)*c);

    /* use first two chars as hash key */
    c= (char *) &key;
    *c = *s;
    c++;
    *c = *(s+1);

    idx= key;
    for( ; idx < MAX_WRDS; idx++) {
	/* if unknown word */
	if( !wrds[idx] ) {
    	    if( (sym_ptr + strlen(s) +1) >= sym_buf_end ) {
		fprintf(stderr, "ERROR: overflow SymBufSize %d\n", SymBufSize);
		return(-1);
	    }
	    wrds[idx]= sym_ptr;
	    strcpy(sym_ptr, s);
	    sym_ptr += strlen(s) +1;
	    return(idx);
	}
	if( !strcmp(wrds[idx], s) ) return(idx);
    }
    fprintf(stderr, "ERROR: Unable to add word %s\n", s);
    return(-1);
}

int get_symbol()
{
    static int last_tok = 0;
    char symbol[LABEL_LEN];
    char *sym_ptr = symbol;
    char *sym_end = symbol+LABEL_LEN;
    char *ptr;

    while( isspace((int) *rule_ptr) ) rule_ptr++;
    if( !*rule_ptr || (*rule_ptr == '\n')) return(-1);

    ptr= rule_ptr;
    while( !isspace( (int) *rule_ptr ) && (sym_ptr < sym_end) )
	*sym_ptr++ = *rule_ptr++;
    if( sym_ptr >= sym_end ) {
	printf("ERROR: symbol too long: %s.\n", ptr);
	return(-1);
    }
    *sym_ptr= '\0';

    last_tok= find_sym(symbol);
    return( last_tok );
}

int blank_line(char *s)
{
    while( *s && isspace( (int) *s) ) s++;
    if( !*s ) return(1);
    else return(0);
}

