#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "parse.h"
#include "globals_parse.h"
#include "functions.h"

extern int first_concept;

int	pwp;


char *print_edge(Edge *edge, Gram *gram, char *s)
{
    Edge	**cp;
    int		nxt, i;

    if( s ) {
	sprintf(s, "%s ( ", gram->labels[edge->net]);
	s += strlen(s);
    }
    else printf("%s ( ", gram->labels[edge->net]);

    pwp= edge->sw;
    /* print children */
    for( cp= edge->chld, i=0; i < edge->nchld; i++, cp++) {
	for(nxt= (*cp)->sw; pwp < nxt; pwp++) {
	    if( script[pwp] == -1 ) continue;
	    if( script[pwp] == start_token) continue;
	    if( script[pwp] == end_token) continue;
	    if( s ) {
		sprintf(s, "%s ", gram->wrds[ script[pwp] ] );
		s += strlen(s);
	    }
	    else printf("%s ", gram->wrds[ script[pwp] ] );
	}
	s= print_edge( *cp, gram, s );
	pwp= (*cp)->ew+1;
    }
    for( ; pwp <= edge->ew; pwp++) {
	    if( script[pwp] == -1 ) continue;
	    if( script[pwp] == start_token) continue;
	    if( script[pwp] == end_token) continue;
	    if( s ) {
		sprintf(s, "%s ", gram->wrds[ script[pwp] ] );
		s += strlen(s);
	    }
	    else printf("%s ", gram->wrds[ script[pwp] ] );
    }
    if( s ) {sprintf(s, ") "); s += strlen(s); }
    else printf(") ");

    return(s);
}




char *print_extracts(Edge *edge, Gram *gram, char *s, int level, char *fn)
{
    Edge	**cp;
    int		nxt, i;
    char	concept,
		name[LABEL_LEN];


    /* get rid of [ */
    strcpy(name, gram->labels[edge->net]+1);
    if( isupper((int)name[0]) ) concept= 1;
    else concept= 0;

    /* if flag */
    if( name[0] == '_' ) {
	/* get rid of ] */
	name[strlen(name)-1]= 0;
	if( s ) {
	    sprintf(s, "%s ", name+1 );
	    s += strlen(s);
	}
	else printf("%s ", name+1 );
    }

    /* if concept */
    else if( concept ) {
	/* start slot on new line */
	if( !level && !first_concept ) {
            if( s ) { sprintf(s, "\n%s:", fn); s += strlen(s); }
	    else printf("\n");
	}

        if( s ) {
	    sprintf(s, "%s.", gram->labels[edge->net]);
	    s += strlen(s);
        }
        else printf("%s.", gram->labels[edge->net]);

	first_concept= 0;
    }

    pwp= edge->sw;
    /* if concept leaf, print associated string */
    if( gram->leaf[edge->net] ) {
	/* if flag value */
	if( edge->nchld && (*(gram->labels[edge->chld[0]->net]+1) == '_') ) {
    	    strcpy(name, gram->labels[edge->chld[0]->net]+1);
	    /* get rid of ] */
	    name[strlen(name)-1]= 0;
	    strip_underscore(name);
	    if( s ) {
		/* get rid of _ */
	        sprintf(s, "%s ", name +1 );
	        s += strlen(s);
	    }
	    else printf("%s ", name+1 );
	    pwp= edge->ew+1;
	}
	/* else print string */
	else {
            for( ; pwp <= edge->ew; pwp++) {
	        if( script[pwp] == -1 ) continue;
	        if( script[pwp] == start_token) continue;
	        if( script[pwp] == end_token) continue;
	        if( s ) {
		    sprintf(s, "%s ", gram->wrds[ script[pwp] ] );
		    s += strlen(s);
	        }
	        else printf("%s ", gram->wrds[ script[pwp] ] );
            }
	}
    }
    else {
        /* print children */
        for( cp= edge->chld, i=0; i < edge->nchld; i++, cp++) {
	    for(nxt= (*cp)->sw; pwp < nxt; pwp++) ;
	    s= print_extracts( *cp, gram, s, (concept) ? level+1 : level, fn);
	    pwp= (*cp)->ew+1;
        }
    }

    return(s);
}

void print_seq(SeqNode *ss, char **lbl)
{
    SeqNode	*sq, *np;

	for( sq= ss; sq; sq= sq->link) {
	    for( np= sq; np; np= np->back_link) {
		printf("%s ", lbl[np->edge->net]);
	    }
	    printf("\n");
    	}
}

void strip_underscore(char *str)
{
    char *f, *t;

    for(f= t= str; *f; f++) {
	if( *f != '_' ) { *t++ = *f; continue;}
	if( *(f+1) == '_' ) { *t++ = *f; f++; continue;}
	else  *t++ = ' ';
    }
    *t= (char)0;
}
