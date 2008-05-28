#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "parse.h"
#include "globals_gram.h"
#include "functions.h"



/* read grammar and associated files */
Gram *read_grammar(dir, dict_file, grammar_file, frames_file, priority_file)
    char *dir, *dict_file, *grammar_file, *frames_file, *priority_file;
{
    Gram	*gram;
    char	name[LABEL_LEN];
    char	*sym_ptr, *sym_buf_end;
    int		i,
		num;
    FILE	*fp;

    /* malloc structure to hold grammar configuration */
    if( !(gram= (Gram *) calloc(1, sizeof(Gram))) ) {
	fprintf(stderr, "ERROR: malloc for grammar failed\n");
	exit(-1);
    }

    /* malloc space for symbol buffer */
    if( !(gram->sym_buf=(char *)malloc(SymBufSize * sizeof(char)))){
	fprintf(stderr, "ERROR: can't allocate space for script buffer\n");
	exit(-1);
    }
    sym_ptr= gram->sym_buf;
    sym_buf_end= gram->sym_buf + SymBufSize;


    /* read dictionary word strings */
    gram->num_words= read_dict(dir, dict_file, gram, &sym_ptr, sym_buf_end);

    /* read grammar networks */
    sprintf(name, "%s/%s", dir, grammar_file);
    read_nets(name, gram, &sym_ptr, sym_buf_end);

    /* read frames and create set of active  nets */
    read_frames(dir, frames_file, gram, &sym_ptr, sym_buf_end);

    /* initialize priorities */
    gram->priorities= (unsigned char *)0;
    gram->max_pri= 0;

    sprintf(name, "%s/%s", dir, priority_file);
    if( (fp= fopen(name, "r")) ) {
	if( !(gram->priorities= (unsigned char *) malloc(gram->num_nets)) ) {
	    fprintf(stderr, "ERROR: malloc priority failed\n");
	    exit(-1);
	}
	for(i=0; i < gram->num_nets; i++) gram->priorities[i]= (unsigned char)0;
	name[0]='[';
	while( fscanf(fp, "%s %d", name+1, &num) == 2 ) {
	    strcat(name,"]");
	    i= find_net(name, gram);
	    if( i < 0 ) {
		fprintf(stderr, "WARNING: Can't find net %s for priority\n",
				name);
		continue;
	    }
	    gram->priorities[i]= (unsigned char) num;
	    if( num > gram->max_pri) gram->max_pri= num;
	}
	fclose(fp);
    }
    return(gram);
}


/* read grammar networks */
void read_nets(net_file, gram, sb_start, sym_buf_end)
    char *net_file, **sb_start, *sym_buf_end;
    Gram *gram;
{
    FILE *fp;
    Gnode	*net,
		*gnode;
    Gsucc	*succ;
    char	name[LABEL_LEN],
		*sym_ptr;
    int		num_nodes,
		net_num,
		num_nets,
		offset,
		i,
		j;
    int		zz,
		val;

    sym_ptr= *sb_start;

    if( !(fp = fopen(net_file, "r") ))
	quit(-1, "Cannot open grammar file %s\n", net_file);

    /* read number of nets */
    if( fscanf(fp, "Number of Nets= %d", &num_nets) < 1 ) {
	fprintf(stderr,"ERROR: bad format in grammar file %s\n", net_file);
	exit(-1);
    }
    /* alow for net numbers start at 1 */
    num_nets++;
    gram->num_nets= num_nets;

    /* malloc space for net names pointers */
    if( !(gram->labels= (char **) calloc(num_nets, sizeof(char *))) ) {
	fprintf(stderr, "ERROR: malloc for labels failed\n");
	exit(-1);
    }

    /* malloc space for pointers to nets */
    if( !(gram->Nets= (Gnode **) malloc(num_nets * sizeof(Gnode *))) ) {
	fprintf(stderr, "ERROR: malloc for Nets failed\n");
	exit(-1);
    }

    /* malloc space for node counts to nets */
    if( !(gram->node_counts= (int *) calloc(num_nets, sizeof(Gnode *))) ) {
	fprintf(stderr, "ERROR: malloc for node counts failed\n");
	exit(-1);
    }

    /* malloc space for concept leaf flags */
    if( !(gram->leaf= (char *)malloc(num_nets * sizeof(char))) ) {
	printf("malloc failed\n");
	exit(-1);
    }

    /* read net name, net number, number of nodes and concept_leaf flag */
    while( fscanf(fp, "%s %d %d %d", name, &net_num, &num_nodes, &val) == 4 ) {
	gram->leaf[net_num]= val;
	/* save node count */
	gram->node_counts[net_num]= num_nodes;

        /* copy net name to labels array */
        strcpy(sym_ptr, name);
        gram->labels[net_num]= sym_ptr;
        sym_ptr += strlen(sym_ptr) +1;
        if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr, "ERROR: overflow SymBufSize %d\n", SymBufSize);
	    exit(-1);
        }

        /* malloc space for nodes */
        if( !(net= (Gnode *) malloc( num_nodes* sizeof(Gnode) ) ) ) {
	    fprintf(stderr, "ERROR: malloc net failed: net %s  num_nodes %d\n",
		name, num_nodes);
	    exit(-1);
        }

	/* save pointer to start node in Nets array */
	gram->Nets[net_num]= net;

        /* read nodes */
        for( i=0, gnode= net; i < num_nodes; i++, gnode++ ) {
	    /* read node */
	    if( fscanf(fp, "%d %hd %hd", &zz,
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
	    if( !(succ= (Gsucc *) malloc( gnode->n_suc * sizeof(Gsucc) ) ) ) {
	        fprintf(stderr, "ERROR: malloc for succ nodes failed\n");
	        exit(-1);
	    }
	    gnode->succ= succ;

	    /* read succs */
	    for(j=0; j < gnode->n_suc; j++, succ++) {
		if( fscanf(fp, "%d %d %d", &succ->tok,
				&succ->call_net, &offset) != 3) {
			fprintf(stderr, "ERROR: failed reading node succs\n");
			exit(-1);
		}
		succ->state= net + offset;
	    }
        }
    }
    fclose(fp);
    *sb_start= sym_ptr;
}




/* read frame definitions */
void read_frames(dir, frames_file, gram, sb_start, sym_buf_end)
    char *dir, *frames_file, **sb_start, *sym_buf_end;
    Gram *gram;
{
    int		slot_num, fn, j,
    		slot_per_frame,
    		num_frames;
    char	name[LABEL_LEN],
		line[LINE_LEN],
		prompt[LINE_LEN],
		*r,
		*sym_ptr;
    unsigned short	*form_buf;
    FILE	*fp;

    sym_ptr= *sb_start;

    /* open frames file */
    sprintf(line, "%s/%s", dir, frames_file);
    if( !(fp= fopen(line, "r")) ) {
	fprintf(stderr, "ERROR: can't open %s\n", line);
	exit(-1);
    }

    /* count frames and slots_per_frame */
    slot_per_frame= 0;
    slot_num= 0;
    for(num_frames= 0; ; num_frames++ ) {

	/* scan for start of frame */
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%*[^\n]\n", name);
	    if( !strncmp(name, "FRAME:", 6) ) break;
	}
	if( !r ) break;

	/* scan for net declarations */
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%*[^\n]\n", name);
	    if( !strncmp(name, "NETS:", 5) ) break;
	}
	if( !r ) {
	    fprintf(stderr, "Bad format in forms file\n");
	    exit(-1);
	}

	/* count net declarations */
	slot_num= 0;
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%*[^\n]\n", name);
	    if( name[0] == ';' ) break;
	    if( name[0] != '[' ) continue;	 /* if comment */
	    slot_num++;
	}
	if( !r ) {
	    fprintf(stderr, "Bad format in forms file\n");
	    exit(-1);
	}
	if( slot_num > slot_per_frame ) slot_per_frame= slot_num;

	/* scan for end of frame marker */
	if( name[0] != ';' ) {
	    while( (r= fgets(line, LINE_LEN, fp)) ) {
                sscanf(line, "%s%*[^\n]\n", name);
	        if( name[0] == ';') break;
	    }
	    if( !r ) {
	        fprintf(stderr, "Bad format in forms file\n");
	        exit(-1);
	    }
	}

    }

    gram->num_frames= num_frames;

    /* malloc space for frame templates */
    if(!(gram->frame_def= (FrameDef *) malloc(num_frames * sizeof(FrameDef)))) {
	fprintf(stderr, "ERROR: malloc Frame templates failed\n");
	exit(-1);
    }
    /* malloc space for frame names */
    if( !(gram->frame_name= (char **) malloc(num_frames * sizeof(char **))) ) {
	fprintf(stderr, "ERROR: malloc Frame names failed\n");
	exit(-1);
    }

    /* malloc space for temporary form buffer */
    if( !(form_buf= (unsigned short *) malloc(slot_per_frame *
		                            sizeof(unsigned short *))) ) {
	fprintf(stderr, "ERROR: malloc form buffer failed\n");
	exit(-1);
    }

    rewind(fp);

    /* read frames */
    slot_num= 0;
    for(fn= 0; ; fn++ ) {

	/* scan for start of frame */
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%s%*[^\n]\n", name, sym_ptr);
	    if( !strncmp(name, "FRAME:", 6) ) break;
	}
	if( !r ) break;
	if( strncmp(name, "FRAME:", 6) ) break;

	/* copy frame name */
	gram->frame_name[fn]= sym_ptr;
	sym_ptr += strlen(sym_ptr) +1;
	if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr, "ERROR: overflow SymBufSize\n");
	    exit(-1);
	}

	/* scan for net declarations */
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%*[^\n]\n", name);
	    if( !strncmp(name, "NETS:", 5) ) break;
	}
	if( !r ) {
	    fprintf(stderr, "Bad format in frames file\n");
	    exit(-1);
	}

	/* read net names */
	slot_num= 0;
	prompt[0]=0;
	while( (r= fgets(line, LINE_LEN, fp)) ) {
	    sscanf(line, "%s%*[^:]:%[^\n]\n", name, prompt);
	    if( name[0] == ';' ) break;
	    if( name[0] != '[' ) continue;	 /* if comment */

	    /* look up net number */
	    j= find_net(name, gram);
	    /* if name of a network */
	    if( j > -1 ) {
		/* insert slot for net in frame */
		form_buf[slot_num]= j;
		slot_num++;
	    }
	    else {
		fprintf(stderr, "WARNING: can't find net %s for frame\n", name);
	    }
	}
	if( !r ) {
	    fprintf(stderr, "Bad format in frames file\n");
	    exit(-1);
	}

	/* copy slot buffer to frame definition */
	gram->frame_def[fn].n_slot= slot_num;

	/* malloc space for slot id array */
	if( !(gram->frame_def[fn].slot= (unsigned short *)
		       malloc(slot_num * sizeof(unsigned short *))) ) {
	    fprintf(stderr, "ERROR: malloc form def failed\n");
	    exit(-1);
    	}


	/* copy net numbers */
	for(j= 0; j < slot_num; j++) {
	    gram->frame_def[fn].slot[j]= form_buf[j];
	}

	/* scan for end of frame marker */
        if( name[0] != ';' ) {
	    while( (r= fgets(line, LINE_LEN, fp)) ) {
	        if( line[0] == ';') break;
	    }
	    if( !r ) {
	        fprintf(stderr, "Bad format in frames file\n");
	        exit(-1);
	    }
	}

    }

    *sb_start= sym_ptr;
}


void read_net_names(dir, nets_file, gram, sb_start, sym_buf_end)
    char *dir, *nets_file, **sb_start, *sym_buf_end;
    Gram *gram;
{
    int		i;
    char	name[LABEL_LEN],
		line[LINE_LEN],
		*sym_ptr;
    FILE	*fp;

    sym_ptr= *sb_start;

    sprintf(line, "%s/%s", dir, nets_file);
    if( !(fp= fopen(line, "r")) ) {
	fprintf(stderr, "ERROR: can't open %s\n", line);
	exit(-1);
    }

    /* count number of nets */
    for(i= 0; fgets(line, LINE_LEN, fp); i++ );
    gram->num_nets= i+1; /* allow for net numbers start at 1 */

    /* malloc space for net names pointers */
    if( !(gram->labels= (char **) calloc(gram->num_nets, sizeof(char *))) ) {
	fprintf(stderr, "ERROR: malloc for labels failed\n");
	exit(-1);
    }


    rewind(fp);
    for(i= 1; fgets(line, LINE_LEN, fp); ) {
	if( sscanf(line, "%s", name) < 1 ) continue; /* empty line */
        /* copy net name to labels array */
	sprintf(sym_ptr, "[%s]", name);
        gram->labels[i++]= sym_ptr;
        sym_ptr += strlen(sym_ptr) +1;
        if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr, "ERROR: overflow SymBufSize %d\n", SymBufSize);
	    exit(-1);
        }
    }

    *sb_start= sym_ptr;
}


/* input net name, return net number */
int find_net(char *name, Gram *gram)
{
    int i;

    if( !*name ) return(-1);
    for( i=1; i < gram->num_nets; i++ ) {
	if( !gram->labels[i] ) continue;
	if(!strcmp(gram->labels[i], name) ) break;
    }

    if( i < gram->num_nets ) return(i);
    return(-1);

}



/* input frame name, return frame number */
int find_frame(char *name, Gram *gram)
{
    int i;

    if( !*name ) return(-1);
    for( i=0; i < gram->num_frames; i++ ) {
	if(!strcmp(gram->frame_name[i], name) ) break;
    }

    if( i < gram->num_frames ) return(i);
    return(-1);

}

