/* routine to read dictionary */
#include <stdio.h>	   
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "parse.h"

extern int	SymBufSize;	/* buffer size to hold char strings */

int read_dict(dir, dict_file, gram, sb_start, sym_buf_end)
    char *dir, *dict_file, **sb_start, *sym_buf_end;
    Gram *gram;
{
    FILE	*fp;
    int		idx, num_words;
    char	filename[LABEL_LEN];
    char	*sym_ptr;

    sprintf(filename, "%s/%s", dir, dict_file);

    if( !(fp= fopen(filename, "r")) ) {
	printf("read_dict: can't open %s\n", filename);
	return(0);
    }

    sym_ptr= *sb_start;

    /* malloc space for word pointers */
    if( !(gram->wrds=(char **)calloc(MAX_WRDS, sizeof(char *)))){
	printf("can't allocate space for script buffer\n");
	exit(-1);
    }

    for(num_words=0; fscanf(fp, "%s %d", sym_ptr, &idx) == 2; num_words++) {
	if( (idx<0) || (idx>MAX_WRDS) ) {
	    printf("word number for %s out of range\n", sym_ptr);
	    exit(-1);
	}
	gram->wrds[idx]= sym_ptr;
	sym_ptr += strlen(sym_ptr) +1;
	if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr, "ERROR: overflow SymBufSize %d\n", SymBufSize);
	    exit(-1);
	}
    }

    fclose(fp);
    *sb_start= sym_ptr;
    gram->num_words= num_words;
    return(num_words);
}




int find_word(s, gram)
    char *s;
    Gram *gram;
{
    unsigned short key;
    char *c;
    int idx;


    /* use first two chars as hash key */
    c= (char *) &key;
    *c = *s;
    c++;
    *c = *(s+1);

    idx= key;
    for( ; idx < MAX_WRDS; idx++) {
	/* if unknown word */
	if( !gram->wrds[idx] ) return(-1);
	if( !strcmp(gram->wrds[idx], s) ) return(idx);
    }
    return(-1);
}

