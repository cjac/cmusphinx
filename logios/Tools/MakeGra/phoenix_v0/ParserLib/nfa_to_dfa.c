/* convert nfa graph to dfa tree */

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
#define MAX_NODES 50000
#define MAX_ARCS 100000
#define MAX_SET 100

int	new_old[MAX_NODES][MAX_SET],
	num_nodes, num_new_nodes,
	net_num,
	concept_val,
	nn, na, no;
Gnode	*nodes;
Gsucc	*arcs;
Gnode	new_nodes[MAX_NODES];
Gsucc	new_arcs[MAX_ARCS];
char	net_name[LABEL_LEN];

void nfa_to_dfa();
void write_dfa();
void add_to_set();
int check_arcs();

int main(argc, argv)
	int argc;
	char **argv;
{
    char	name[LABEL_LEN];
    FILE	*fp_old, *fp_new;
    Gnode	*gnode;
    int		num_nets,
		offset,
		i,
		j;
    int		zz;

    if( argc < 2 ) {
	printf("Usage: nfa_to_dfa <net file name>\n");
	exit(-1);
    }

    /* open old compiled grammar file */
    if( !(fp_old = fopen(argv[1], "r") )) {
	printf("Cannot open compiled grammar file %s\n", argv[1]);
	exit(-1);
    }
    /* open new compiled grammar file */
    sprintf(name, "%s.tree", argv[1]);
    if( !(fp_new = fopen(name, "w") )) {
	printf("Cannot open new file %s\n", name);
	exit(-1);
    }

    /* read number of nets */
    if( fscanf(fp_old, "Number of Nets= %d", &num_nets) < 1 ) {
	fprintf(stderr,"ERROR: bad format in grammar file\n");
	exit(-1);
    }
    fprintf(fp_new, "Number of Nets= %d\n", num_nets);

    /* read net name, net number, number of nodes and concept_leaf flag */
    while( fscanf(fp_old, "%s %d %d %d",
		net_name, &net_num, &num_nodes, &concept_val) == 4 ) {

        /* malloc space for nodes */
        if( !(nodes= (Gnode *) malloc( num_nodes* sizeof(Gnode) ) ) ) {
	    fprintf(stderr, "ERROR: malloc net failed: net %s  num_nodes %d\n",
		name, num_nodes);
	    exit(-1);
        }

        /* read nodes */
        for( i=0, gnode= nodes; i < num_nodes; i++, gnode++ ) {
	    /* read node */
	    if( fscanf(fp_old, "%d %hd %hd", &zz,
		&gnode->n_suc, &gnode->final) != 3) {
		fprintf(stderr,"ERROR: failed reading grammar, node %d\n", zz);
		exit(-1);
	    }
	    if( zz != i ) fprintf(stderr,"WARNING: net %s node %d out of seq\n",
				 name, zz);

	    if( !gnode->n_suc ) {
		gnode->succ= 0;
		continue;
	    }

	    /* malloc space for succs */
	    if( !(arcs= (Gsucc *) malloc( gnode->n_suc * sizeof(Gsucc) ) ) ) {
	        fprintf(stderr, "ERROR: malloc for succ nodes failed\n");
	        exit(-1);
	    }
	    gnode->succ= arcs;

	    /* read succs */
	    for(j=0; j < gnode->n_suc; j++, arcs++) {
		if( fscanf(fp_old, "%d %d %d", &arcs->tok,
				&arcs->call_net, &offset) != 3) {
			fprintf(stderr, "ERROR: failed reading node succs\n");
			exit(-1);
		}
		arcs->state= nodes + offset;
	    }
        }

	printf("Net %s read %d nodes\n", net_name, num_nodes);
	nfa_to_dfa();
        write_dfa(fp_new);

	/* free arcs */
	for( i=0, gnode= nodes; i < num_nodes; i++, gnode++ )
		if( gnode->succ ) free(gnode->succ);
	/* free nodes */
	free(nodes);
    }
    return(0);
}



void nfa_to_dfa()
{
    int	i, j,
	oi,
	ns, os;
    Gsucc	*suc;

    /* clear structures */
    for(i=0; i < MAX_NODES; i++) {
	for(j=0; j < MAX_SET; j++) new_old[i][j]= -1;
    }

    /* initialize start state */
    new_old[0][0]= 0;
    new_nodes[0].n_suc= 0;
    new_nodes[0].final= 0;
    nn= na= 0;

    /* for each new state */
    for(no=0; (new_old[no][0] != -1) && (no < MAX_NODES); no++) {
	/* process all old states associated with new state */
	for(oi=0; (new_old[no][oi] != -1) && (oi< MAX_SET); oi++) {
	    os= new_old[no][oi];

/*whw*/printf("procesing no[%d][%d]  old state %d\n", no, oi, os);
	    /* for each arc out of old state */
	    for(i=0, suc= (nodes+os)->succ; i < (nodes+os)->n_suc; i++, suc++){

		/* if token already on arc out of new state */
		if( (ns= check_arcs(no, suc->tok)) ) {
		    /* add old to_state to set for new to_state */
		    add_to_set(ns, suc->state - nodes);
		    if( suc->state->final ) new_nodes[ns].final= 1;
		}
		else {
		    /* add arc to new state */
		    if( ++nn >= MAX_NODES ) {
			printf("MAX_NODES overflow\n");
			exit(-1);
		    }
		    new_nodes[nn].n_suc= 0;
		    new_nodes[nn].final= 0;
		    if( suc->state->final ) new_nodes[nn].final= 1;
		    if( !new_nodes[no].n_suc ) {
			new_nodes[no].succ= new_arcs+na;
		    }
		    new_nodes[no].n_suc++;
		    new_arcs[na].tok= suc->tok;
		    new_arcs[na].call_net= suc->call_net;
		    new_arcs[na].state= new_nodes+nn;
		    if( ++na >= MAX_ARCS ) {
			printf("MAX_ARCS overflow\n");
			exit(-1);
		    }
		    new_old[nn][0]= suc->state - nodes;
/*whw*/printf("add new state %d arc %d  no[%d][%d]= %d\n", nn, suc->tok, nn, 0, new_old[nn][0]);
		}
	    }
	}
    }
    num_new_nodes= nn+1;
}


void write_dfa( FILE *fp )
{
    int ni, ai, i;

    /* write net name, net number, number of states and concept_flag */
    fprintf(fp, "%s %d %d %d\n", net_name, net_num, num_new_nodes, concept_val);

    /* for each state */
    for(ni= 0; ni < num_new_nodes; ni++) {
	/* write node */
	fprintf(fp, "%d  %d %d\n", ni, new_nodes[ni].n_suc,new_nodes[ni].final);
	/* write arcs */
	for(i=0, ai= new_nodes[ni].succ - new_arcs;
		i < new_nodes[ni].n_suc; i++, ai++) {
            fprintf(fp, "\t\t%d    %d    %d\n", new_arcs[ai].tok,
                new_arcs[ai].call_net, new_arcs[ai].state - new_nodes);
	}

    }

}


void add_to_set(int ns, int os)
{
    int	i;

/*whw*/printf("add to set %d %d\n", ns, os);
    for(i=0; i < MAX_SET; i++) {
	if( new_old[ns][i] != -1 ) continue;
	new_old[ns][i]= os;
/*whw*/printf("new_old[%d][%d]= %d\n", ns, i, os);
	return;
    }
    printf("MAX_SET overflow\n");
    exit(-1);
}


int check_arcs(int no, int tok)
{
    int	i, ai;

    for(i=0,ai=new_nodes[no].succ-new_arcs; i < new_nodes[no].n_suc; i++,ai++){
	if( new_arcs[ai].tok == tok )
		return( new_arcs[ai].state - new_nodes );
    }
    return(0);
}
