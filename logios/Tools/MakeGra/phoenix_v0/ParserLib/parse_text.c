/* process text input from stdin
   write parsed output to stdout
   utterances terminated with newline
   type "quit" to exit
*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "parse.h"
#include "pconf.h"
#include "globals_parse.h"


void strip_line();

extern char	dict_file[LABEL_LEN],
		priority_file[LABEL_LEN],
		frames_file[LABEL_LEN],
		*grammar_file;

static char	line[LINE_LEN];		/* input line buffer */

static char	outbuf[10000],		/* output text buffer for parses */
		*out_ptr= outbuf;

static int	utt_num;

int main(argc, argv)
	int argc;
	char **argv;
{
    FILE	*fp;
    char	*s;
    int		i;


    /* set command line or config file parms */
    config(argc, argv);

    /* read grammar, initialize parser, malloc space, etc */
    init_parse(dir, dict_file, grammar_file, frames_file, priority_file);

    /* terminal input */
    fp= stdin; fprintf(stderr, "READY\n");

    /* for each utterance */
    for( utt_num= 1; fgets(line, LINE_LEN-1, fp);  ) {
	/* if printing comment */
	if (*line == ';' ) { printf("%s\n", line); continue; }
	/* if non-printing comment */
	if (*line == '#' ) { continue; }
	/* if blank line */
	for(s= line; isspace((int)*s); s++); if( strlen(s) < 2 ) continue;

        /* strip out punctuation, comments, etc, to uppercase */
        strip_line(line);

	/* check for terminate */
        if( !strncmp(line, "QUIT", 4) ) exit(1);

	/* clear output buffer */
	out_ptr= outbuf; *out_ptr= 0;

        /* echo the line */
        if (verbose > 1){
	    sprintf(out_ptr, ";;;%d %s\n", utt_num, line);
	    out_ptr += strlen(out_ptr);
	}
    
        /* assign word strings to slots in frames */
        parse(line, gram);

        if( PROFILE ) print_profile();

	/* print parses to buffer */
	if( num_parses > MAX_PARSES ) num_parses= MAX_PARSES;
	if( ALL_PARSES ) {
            for(i= 0; i < num_parses; i++ ) {
	    	sprintf(out_ptr, "PARSE_%d:\n", i);
	    	out_ptr += strlen(out_ptr);
	    	print_parse(i, out_ptr, extract, gram);
	    	out_ptr += strlen(out_ptr);
	    	sprintf(out_ptr, "END_PARSE\n");
	    	out_ptr += strlen(out_ptr);
            }
	}
	else {
	    	print_parse(0, out_ptr, extract, gram);
	    	out_ptr += strlen(out_ptr);
	}
	sprintf(out_ptr, "\n");
	out_ptr += strlen(out_ptr);


	if( verbose ) {
	    if( num_parses > 0) printf("%s", outbuf);
	    fflush(stdout);
	}

        /* clear parser temps */
        reset(num_nets);

	utt_num++;
    }

    return(1);
}



void strip_line(line)
char	*line;
{
  char	*from, *to;

  for(from= to= line; ;from++ ) {
    if( !(*from) ) break;


    switch(*from) {

      /* filter these out */
    case '(' :
    case ')' :
    case '[' :
    case ']' :
    case ':' :
    case ';' :
    case '?' :
    case '!' :
    case '\n' :
      break;

      /* replace with space */
    case ',' :
    case '\\' :
      *to++ = ' ';
      break;

    case '#' :
	for( ++from; *from != '#' && *from; from++);
	if( *from == '#' ) from++;
	break; 

    case '-' :
      /* if partial word, delete word */
      if( isspace( (int) *(from+1) ) ) {
	while( (to != line) && !isspace( (int) *(--to) ) ) ;
	/* replace with space */
	*to++ = ' ';
      }
      else {
	/* copy char */
	*to++ = *from;
      }
      break;


    default:
      /* copy char */
      *to++ = (islower((int)*from)) ? (char) toupper((int)*from) : *from;
    }
    if( !from ) break;

  }
  *to= 0;

}
