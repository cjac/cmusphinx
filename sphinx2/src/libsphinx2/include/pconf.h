/* PCONF.H
 *------------------------------------------------------------*
 * Copyright 1988, Fil Alleva and Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 *------------------------------------------------------------*
 * HISTORY
 * $Log$
 * Revision 1.1  2000/12/05  01:33:29  lenzo
 * files added or moved
 * 
 * Revision 1.1.1.1  2000/01/28 22:09:07  lenzo
 * Initial import of sphinx2
 *
 *
 * Revision 1.1  1993/03/23  14:57:57  eht
 * Initial revision
 *
 */

#ifndef _PCONF_
#define _PCONF_

#include <sys/types.h>

#ifdef WIN32
#include <posixwin32.h>
#endif

typedef enum {NOTYPE,
	      BYTE, SHORT, INT, LONG,
	      U_BYTE, U_SHORT, U_INT, U_LONG,
	      FLOAT, DOUBLE,
	      BOOL, CHAR, STRING,
	      DATA_SRC
} arg_t;

typedef enum {
	SRC_NONE, SRC_HSA, SRC_VQFILE, SRC_CEPFILE, SRC_ADCFILE
} data_src_t;


typedef union _ptr {
    char	*CharP;
    char	*ByteP;
    u_char	*UByteP;
    short	*ShortP;
    u_short	*UShortP;
    int		*IntP;
    u_int	*UIntP;
    long	*LongP;
    u_long	*ULongP;
    float	*FloatP;
    double	*DoubleP;
    int		*BoolP;
    char	**StringP;
    data_src_t	*DataSrcP;

} ptr_t;

typedef struct _config {
	char const *LongName;		/* Long Name */
	char const *Doc;		/* Documentation string */
	char const *swtch;		/* Switch Name */
	arg_t	arg_type;		/* Argument Type */
	caddr_t	var;			/* Pointer to the variable */
} config_t;

typedef struct _InternalConfig {
	char const *LongName;		/* Long Name */
	char const *Doc;		/* Documentation string */
	char const *swtch;		/* Switch Name */
	arg_t	arg_type;		/* Argument Type */
	ptr_t	var;			/* Pointer to the variable */
} Config_t;

int pconf(int argc, char *argv[],
	  config_t *config_p, char **display, char **geometry,
	  char * (*GetDefault)(char const *, char const *));
int ppconf(int argc, char *argv[],
	   config_t *config_p, char **display, char **geometry,
	   char * (*GetDefault)(char const *, char const *), char last);
void pusage(char *prog, Config_t *cp);

#endif _PCONF_
