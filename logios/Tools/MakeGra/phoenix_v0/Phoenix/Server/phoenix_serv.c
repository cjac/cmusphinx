/*************************************************************************
 *                                                                       * 
 *                Center for Speech and Language Research		 *
 *                   University of Colorado, Boulder                     *
 *                       Copyright (c) 1999,2000                         *
 *                        All Rights Reserved.                           *
 *                                                                       *
 *  The University of Colorado ("CU") hereby grants to you an            *
 *  irrevocable, nonexclusive, nontransferable, perpetual, royalty-free  *
 *  and worldwide license to use this source code solely for educational,*
 *  research, personal use, or evaluation.  Limitations of Use are       *
 *  described below.  A commercial license can be obtained only through  *
 *  expressed written permission of The Center for Speech and Language   *
 *  Research, University of Colorado at Boulder.			 *
 *									 *
 *  	Non-Commercial License Limitations and Use Requirements		 *
 *									 *
 *  Limitations on Use: The License is limited to noncommercial use.     *
 *  Noncommercial use relates only to educational, research, personal    *
 *  use, or evaluation purposes.  Any other use is commercial use.  You  *
 *  may not use the Software in connection with any business activities. *
 *  You may distribute and/or allow others to use a) the Software or b)  *
 *  the applications you create with the Software only if each new user  *
 *  is bound by the provisions of this Agreement.			 *
 *									 *
 *  Conditions of Use: This work is furnished to you under the following *
 *  conditions:								 *
 *									 *
 *   1. The code must retain the above copyright notice, this list of    *
 *      conditions and the following disclaimer.                         *
 *   2. Any modifications must be clearly marked as such.                *
 *   3. Original authors' names are not deleted.                         *
 *   4. The authors' names are not used to endorse or promote products   *
 *      derived from this software without specific prior written        *
 *      permission.                                                      *
 *   5. You agree to acknowledge CU-Boulder's Center for Speech and      *
 *      Language Reseach with appropriate citations in any publication or*
 *      presentation containing research results obtained in whole or in *
 *      part through the use of the Software.				 *
 *									 *
 *  Term of License: The License is effective upon receipt by you of the *
 *  Software and shall continue until terminated.  The License will      *
 *  terminate immediately without notice by CU if you fail to comply     *
 *  with the terms and conditions of this Agreement.  Upon termination   *
 *  of this License, you shall immediately discontinue all use of the    *
 *  Software provided hereunder, and return to CU or destroy the         *
 *  original and all copies of all such Software.  All of your           *
 *  obligations under this Agreement shall survive the termination of    *
 *  the License.							 *
 *									 *
 *  DISCLAIMER:                                                          *
 *                                                                       *
 *  THE UNIVERSITY OF COLORADO AND THE CONTRIBUTORS TO THIS WORK         *
 *  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   *
 *  SHALL THE UNIVERSITY OF COLORADO NOR THE CONTRIBUTORS BE LIABLE      *
 *  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    *
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   *
 *  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       *
 *  THIS SOFTWARE.                                                       *
 *                                                                       *
 *************************************************************************/
/*-----------------------------------------------------------------------*
 *                                                                       *
 *		     CU Communicator Phoenix Server		         * 
 *                                                                       *
 *                                                                       *
 *   Wayne Ward (whw@cslu.colorado.edu)                              	 *
 *   Center for Spoken Language Understanding				 *
 *   University of Colorado at Boulder					 *
 *   (c) 1999 All Rights Reserved					 *
 *                                                                       *
 *-----------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <strings.h>

#include <parse.h>
#include <pconf.h>
#include <globals_parse.h>

#include <galaxy/galaxy_all.h>
#define SERVER_FUNCTIONS_INCLUDE "server.h"
#define USE_SERVER_DATA
#include <galaxy/server_functions.h>

Gram	*read_grammar();
void	strip_line();

extern char	dict_file[LABEL_LEN],
		priority_file[LABEL_LEN],
		frames_file[LABEL_LEN],
		*grammar_file;

static char	outbuf[10000];

/*-----------------------------------------------------------------------*
 * Command-line options							 * 
 *-----------------------------------------------------------------------*/
static char *oas[] = {
   "-config filename", "use a configuration file for Phoenix", NULL,
   NULL
};


/*-----------------------------------------------------------------------*
 * Main routine which takes a string as input and returns a parse string * 
 *-----------------------------------------------------------------------*/
Gal_Frame send_to_parse(Gal_Frame f, void *server_data)
{
   int i, path_score;
   char *in_string, *out_string, *sysid;
   Gal_Frame f_new = Gal_MakeFrame("main", GAL_CLAUSE);

   /* get the input string to parse from the key's value */
   in_string  = Gal_GetString(f, ":input_string");
   sysid      = Gal_GetString(f, ":sysid");
   path_score = Gal_GetInt(f, ":path_score");

   if (in_string == NULL)
      in_string = Gal_GetString(f, ":parse_input");

   /* strip out punctuation, comments, etc, to uppercase */
   strip_line(in_string);

   /* Call Phoenix Parse function */
   parse(in_string, gram);

   /* print parses to buffer */
   if( num_parses > MaxParses ) num_parses= MaxParses;
   if( num_parses < 1 ) { strcpy(outbuf, "No Parse"); }
   else {
	out_string= outbuf;
	for(i= 0; i < num_parses; i++ ) {
	    sprintf(out_string, "PARSE_%d:\n", i);
	    out_string += strlen(out_string);
	    print_parse(i, out_string, extract, gram);
	    out_string += strlen(out_string);
	    sprintf(out_string, "END_PARSE\n");
	    out_string += strlen(out_string);
	}
	sprintf(out_string, "\n");
	out_string= outbuf;
   }

   /* clear parser temps */
   reset(num_nets);

   /*  create a new frame containing the parse */ 
   Gal_SetProp(f_new, ":parse_input", Gal_StringObject(in_string));
   Gal_SetProp(f_new, ":parse_output", Gal_StringObject(outbuf));
   if (sysid != NULL) Gal_SetProp(f_new, ":sysid", Gal_StringObject(sysid));
   if (path_score != 0) Gal_SetProp(f_new, ":path_score", Gal_IntObject(path_score));

   /* write parse output frame to HUB */
   GalSS_EnvWriteFrame((GalSS_Environment *) server_data, f_new, 0);

   return(f);
}

/*-----------------------------------------------------------------------*
 * server reinitialization routine.  This is called once the server is   *
 * started.                                                              *
 *-----------------------------------------------------------------------*/

Gal_Frame reinitialize(Gal_Frame f, void *server_data)
{
  start_session();
  return(f);
}

void *_GalSS_init_server(Gal_Server *s, int argc, char **argv)
{
    char *config_file = NULL;

    if (GalUtil_OACheckUsage(argc, argv, oas, NULL) == 0)
       exit(1);
    GalUtil_OAExtract(argc, argv, oas, "-config", GAL_OA_STRING, &config_file);
    if (config_file == NULL){
       printf("No config file specified... exiting\n");
       exit(1);
    }

    /* set config file parms */
    config(argc, argv);

    /* initialize parser, malloc space, etc */
    init_parse(dir, dict_file, grammar_file, frames_file, priority_file);

    return (void *) NULL;
}




void strip_line(line)
char	*line;
{
  char	*from, *to;

  for(from= to= line; ;from++ ) {
    if( !(*from) ) break;

    switch(*from) {

      /* filter these out */
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

    /* strip noise annotation */
    case '/' :
	for( ++from; *from != '/' && *from; from++);
	if( *from == '/' ) from++;
	break; 
    case '#' :
	for( ++from; *from != '#' && *from; from++);
	if( *from == '#' ) from++;
	break; 

    case '-' :
      /* if partial word, delete word */
      if( isspace( *(from+1) ) ) {
	while( (to != line) && !isspace( *(--to) ) ) ;
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
      *to++ = (islower(*from)) ? toupper(*from) : *from;
    }
    if( !from ) break;

  }
  *to= 0;

}

