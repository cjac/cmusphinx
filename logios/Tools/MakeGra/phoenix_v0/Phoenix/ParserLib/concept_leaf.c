/* flags leaf nodes in nets for extracts function */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "parse.h"
#include "pconf.h"

char is_leaf();
Gram *read_grammar();
void write_net();

char	dict_file[LABEL_LEN] =		"base.dic",
	priority_file[LABEL_LEN] =	"NET_PRIORITIES",
	frames_file[LABEL_LEN] =	"frames";

/* command line parms */
char	*grammar_file = NULL,
	*dir = NULL;
int	SymBufSize=	50000;	/* buffer size to hold char strings */

/* Command Line structure */

  static config_t conf[] = {
  {"dir",	"dir containing forms and nets files",
       "-dir", 	STRING,	(caddr_t) &dir},
  {"grammar",	"file containing compiled grammar nets",
       "-grammar", 	STRING,	(caddr_t) &grammar_file},
  {"SymBufSize", "Size in bytes of symbol buffer for dictionary", 
       "-SymBufSize",	INT,	(caddr_t) &SymBufSize},
  {NULL, NULL, NULL, NOTYPE, NULL}
  };

Gram	*gram;
int	num_nets;	/* number of nets read in */
char	*leaf;

char	*sym_buf,	 	/* buff for ascii input strings */
	*sym_ptr, *sym_buf_end;
FrameDef	*frame_def;	/* nets used in each form */
char    **frame_name;	/* frame names */
int	*active_slots,		/* set of slot level nets used in forms */
	num_active_slots;

FILE	*fp_out;


int main( argc, argv)
    int argc;
    char *argv[];
{
    int	netid;

    /* set command line parms */
    if (pconf(argc,argv,conf,NULL,NULL,NULL))
	pusage(argv[0],conf),exit(-1);

    /* malloc space for symbol buffer */
    if( !(sym_buf=(char *)malloc(SymBufSize * sizeof(char)))){
	printf("can't allocate space for script buffer\n");
	exit(-1);
    }
    sym_ptr= sym_buf;
    sym_buf_end= sym_buf + SymBufSize;

    gram= read_grammar(".", dict_file, grammar_file, frames_file, priority_file);
    num_nets= gram->num_nets;

    if( !(leaf= (char *)malloc(num_nets * sizeof(char))) ) {
	printf("malloc failed\n");
	exit(-1);
    }
    for(netid= 0; netid < num_nets; netid++) leaf[netid]= (char)-1;

    /* set concept leaf flag for all nets, positive if no concept subnets */
    for(netid= 1; netid < num_nets; netid++) {
	if( leaf[netid] != -1 ) continue;
	if( !gram->labels[netid]) {
	    fprintf(stderr, "Net %d not found\n", netid);
	    continue;
	}
	if( !isupper((int) *(gram->labels[netid]+1)) ) continue;
	leaf[netid]= is_leaf( netid );
    }

    /* mark non-concept nets as not concept leaves */
    for(netid= 1; netid < num_nets; netid++) {
	    if(!gram->labels[netid] || !isupper((int)*(gram->labels[netid]+1)))
		leaf[netid]= 0;
    }

    /* open output .net file */
    if( !(fp_out= fopen(grammar_file,"w")) ) {
	printf("can't open %s\n", grammar_file);
	exit(1);
    }
    /* write number of nets at start of file */
    fprintf(fp_out, "Number of Nets= %d\n", num_nets-1);

    for(netid= 1; netid < num_nets; netid++) {
	write_net(netid);
    }
    fclose(fp_out);
    return(1);
}

char is_leaf(int net)
{
    int		nc, sc;
    Gnode	*gn;
    Gsucc	*gs;

    /* for each node in net */
    for( nc=0, gn=gram->Nets[net]; nc < gram->node_counts[net]; nc++, gn++) {
	/* for each sucsessor (arc) of node */
	for(sc=0, gs= gn->succ; sc < gn->n_suc; sc++, gs++) {
	    /* id not call arc */
	    if( !gs->call_net ) continue;
	    if( gs->call_net >= num_nets ) {
		fprintf(stderr, "net %d not found\n", gs->call_net);
		exit(-1);
	    }

	    if( !gram->labels[gs->call_net]) {
		fprintf(stderr, "net %d not found\n", gs->call_net);
		exit(-1);
	    }

	    /* if concept net */
	    if( isupper((int)*(gram->labels[gs->call_net]+1)) ) {
		return((char)0);
	    }
	    else {
		leaf[gs->call_net]= is_leaf(gs->call_net);
		if(!leaf[gs->call_net]) return((char)0);
	    }
	}
    }
    return((char)1);

}


void write_net(int net_id)
{
    Gnode	*gn;
    Gsucc	*gs;
    int		i, j;

    /* write net name, net number, number of states and concept_leaf flag */
    fprintf(fp_out, "%s %d %d %d\n",
	gram->labels[net_id], net_id, gram->node_counts[net_id], leaf[net_id]);

    /* write out nodes */
    for(i=0, gn= gram->Nets[net_id];  i < gram->node_counts[net_id]; i++, gn++) {
	/* write node */
	fprintf(fp_out, "%d  %d %d\n", i, gn->n_suc, gn->final);
	/* write arcs */
	for(j=0, gs= gn->succ; j < gn->n_suc; j++, gs++) {
            fprintf(fp_out, "\t\t%d    %d    %d\n", gs->tok,
                gs->call_net, gs->state - gram->Nets[net_id]);
      }
    }

}
