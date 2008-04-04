/* PCONF.C */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "pconf.h"
extern void quit ();
void pusage ();



#define	TRUE	1
#define	FALSE	0
#define QUIT(x)		{fprintf x; exit(-1);}


static int SetVal();
static void SPrintVal();


/* PCONF
 *------------------------------------------------------------*
 */
extern
int pconf (argc, argv, config_p, display, geometry, GetDefault)
int argc;
char *argv[];
Config_t *config_p;
char **display, **geometry;
char *(*GetDefault)();
{
    int i, parsed;
    int bad_usage = FALSE;
    char *str_p;
    Config_t *cp;

    if (GetDefault) {
	for (cp = config_p; cp->arg_type != NOTYPE; cp++) {
	    if( (str_p = GetDefault (argv[0], cp->LongName)) ) {
		bad_usage |= SetVal (cp, str_p);
	    }
	}
    }

    for (i = 1; i < argc; i++) {
	for (parsed = FALSE, cp = config_p; cp->arg_type != NOTYPE; cp++) {
	    if (strcasecmp (argv[i], cp->swtch) == 0) {
		parsed = TRUE;
		if (++i < argc)
		    bad_usage |= SetVal (cp, argv[i]);
		else
		    bad_usage = TRUE;
	    }
	}

	if (parsed)
	    continue;
	if (geometry && (*argv[i] == '=')) {
	    *geometry = argv[i];
	    continue;
	}
	if (display && (strchr (argv[i], ':') != 0)) {
	    *display = argv[i];
	    continue;
	}
	if ((strcasecmp ("-?", argv[i]) == 0) ||
	    (strcasecmp ("-help", argv[i]) == 0))
	    pusage (argv[0], config_p);
	fprintf (stderr, "ERROR: %s: Unrecognized argument %s\n",
			argv[0], argv[i]);
	bad_usage = TRUE;
    }
    return (bad_usage);
}

/* PPCONF
 *------------------------------------------------------------*
 */
extern
int ppconf (argc, argv, config_p, display, geometry, GetDefault, last)
int argc;
char *argv[];
Config_t *config_p;
char **display, **geometry;
char *(*GetDefault)();
char last;
{
    int i, parsed;
    int bad_usage = FALSE;
    char *str_p;
    Config_t *cp;

    if (GetDefault) {
	for (cp = config_p; cp->arg_type != NOTYPE; cp++) {
		if( (str_p = GetDefault (argv[0], cp->LongName)) ) {
		    bad_usage |= SetVal (cp, str_p);
	    }
	}
    }

    for (i = 1; i < argc; i++) {
	/* argument has been processed already */
	if (argv[i][0] == '\0') continue;

	for (parsed = FALSE, cp = config_p; cp->arg_type != NOTYPE; cp++) {
	    if (strcasecmp (argv[i], cp->swtch) == 0) {
		parsed = TRUE;
		/* remove this switch from consideration */
		argv[i][0] = '\0';
		if (++i < argc) {
		    bad_usage |= SetVal (cp, argv[i]);
		    argv[i][0] = '\0';
		}
		else bad_usage = TRUE;
	    }
	}

	if (parsed || !last) continue;

	if (geometry && (*argv[i] == '=')) {
	    *geometry = argv[i];
	    continue;
	}

	if (display && (strchr (argv[i], ':') != 0)) {
	    *display = argv[i];
	    continue;
	}

	if ((strcasecmp ("-?", argv[i]) == 0) ||
	    (strcasecmp ("-help", argv[i]) == 0))
	    pusage (argv[0], config_p);
	fprintf(stderr, "ERROR: %s: Unrecognized argument, %s\n",
			argv[0], argv[i]);
	bad_usage = TRUE;
    }
    return (bad_usage);
}

/* PUSAGE
 *------------------------------------------------------------*
 */
void pusage (prog, cp)
char *prog;
Config_t *cp;
{
    char valstr[256];

    printf ("Usage: %s\n", prog);
    for (; cp->arg_type != NOTYPE; cp++) {
	SPrintVal (cp, valstr);
	printf (" [%s %s] %s - %s\n", cp->swtch, valstr,
		cp->LongName, cp->Doc);
    }
    exit (-1);
}

/* env_scan: substitutes environment values into a string */
static
char *
env_scan(str)
 register char *str;
{
    extern char *getenv();
    char buf[1024];		/* buffer for temp use */
    register char *p = buf;	/* holds place in the buffer */
    char var[50];		/* holds the name of the env variable */
    char *val;

    while (*str)
	if (*str == '$') {
	    if (*++str == '$') {
		*p++ = *str++;
		continue;
	    }
	    val = var;
	    while (isalnum((int) *str) || *str == '_')
		*val++ = *str++;
	    *p = '\0';
	    val = (val == var) ? "$" : getenv(var);
	    if (val) {
		strcat(p, val);
		p += strlen(val);
	    }
	} else
	    *p++ = *str++;
    *p = '\0';
    return strdup(buf);
}

static int SetVal (cp, str)
register Config_t *cp;
char *str;
{
    switch (cp->arg_type) {
    case CHAR:
	*(cp->var.CharP) = (char) str[0];
	break;
    case BYTE:
	*(cp->var.ByteP) = (char) atoi (str);
	break;
    case U_BYTE:
	*(cp->var.UByteP) = (u_char) atoi (str);
	break;
    case SHORT:
	*(cp->var.ShortP) = (short) atoi (str);
	break;
    case U_SHORT:
	*(cp->var.UShortP) = (u_short) atoi (str);
	break;
    case INT:
	*(cp->var.IntP) = (int) atoi (str);
	break;
    case U_INT:
	*(cp->var.UIntP) = (u_int) atoi (str);
	break;
    case FLOAT:
	*(cp->var.FloatP) = (float) atof (str);
	break;
    case DOUBLE:
	*(cp->var.DoubleP) = (double) atof (str);
	break;
    case BooL:
	if ((strcasecmp ("on", str) == 0) ||
	    (strcasecmp ("1", str) == 0) ||
	    (strcasecmp ("t", str) == 0) ||
	    (strcasecmp ("true", str) == 0) ||
	    (strcasecmp ("y", str) == 0) ||
	    (strcasecmp ("yes", str) == 0))
	    *(cp->var.BoolP) = TRUE;
	else
	if ((strcasecmp ("off", str) == 0) ||
	    (strcasecmp ("0", str) == 0) ||
	    (strcasecmp ("f", str) == 0) ||
	    (strcasecmp ("false", str) == 0) ||
	    (strcasecmp ("n", str) == 0) ||
	    (strcasecmp ("no", str) == 0))
	    *(cp->var.BoolP) = FALSE;
	else
	    return (-1);
	break;
    case STRING:
	*(cp->var.StringP) = env_scan(str);
	break;
    case DATA_SRC:
	if (strcasecmp("hsa", str) == 0)
	    *(cp->var.DataSrcP) = SRC_HSA;
	else if (strcasecmp("vqfile", str) == 0)
	    *(cp->var.DataSrcP) = SRC_VQFILE;
	else if (strcasecmp("adcfile", str) == 0)
	    *(cp->var.DataSrcP) = SRC_ADCFILE;
	else
	    quit(-1, "Unknown data source %s\n", str);
	break;
    default:
	fprintf (stderr, "ERROR: bad param type %d\n", cp->arg_type);
	return (-1);
    }
    return (0);
}

static	void
SPrintVal (cp, str)
register Config_t *cp;
char *str;
{
    switch (cp->arg_type) {
    case CHAR:
	sprintf (str, "%c", *(cp->var.CharP));
	break;
    case BYTE:
	sprintf (str, "%d", *(cp->var.ByteP));
	break;
    case U_BYTE:
	sprintf (str, "%u", *(cp->var.UByteP));
	break;
    case SHORT:
	sprintf (str, "%d", *(cp->var.ShortP));
	break;
    case U_SHORT:
	sprintf (str, "%u", *(cp->var.UShortP));
	break;
    case INT:
	sprintf (str, "%d", *(cp->var.IntP));
	break;
    case U_INT:
	sprintf (str, "%u", *(cp->var.UIntP));
	break;
    case FLOAT:
	sprintf (str, "%f", *(cp->var.FloatP));
	break;
    case DOUBLE:
	sprintf (str, "%f", *(cp->var.DoubleP));
	break;
    case BooL:
	sprintf (str, "%s", (*(cp->var.BoolP) ? "TRUE" : "FALSE"));
	break;
    case STRING:
	sprintf (str, "%s", *(cp->var.StringP));
	break;
    default:
	sprintf (str, "bad param type %d\n", cp->arg_type);
    }
}

#define MAX_NAME_LEN 32
#define MAX_VALUE_LEN 80

#define NAME 0
#define VALUE 1
#define COMMENT 2

/*
 * fpconf
 */
extern
int fpconf (config_fp, config_p, display, geometry, GetDefault)
FILE *config_fp;
Config_t config_p[];
char **display, **geometry;
char *(*GetDefault)();
{
    int parsed, read_mode = NAME, inchar;
    int bad_usage = FALSE;
    Config_t *cp;
    char name[MAX_NAME_LEN+1], value[MAX_VALUE_LEN+1], name_fm[12],
	value_fm[12];

    sprintf(name_fm, "%%%d[^:]", MAX_NAME_LEN);
    sprintf(value_fm, "%%%d[^\n]", MAX_VALUE_LEN);

    for (;;) {
	inchar = fgetc(config_fp);

	if (inchar == EOF) {
	    if (read_mode == VALUE)
		fprintf(stderr,"ERROR: Value expected after name, %s.\n", name);

	    return bad_usage;
	}

	if (inchar == '\n') {
	    read_mode = NAME;
	    continue;
        }

	/* skip whitespace or comments */
        if ( read_mode == COMMENT || inchar == ' ' || inchar == '\t')
	    continue;

	if ( inchar == ';') {
	    read_mode = COMMENT;
	    continue;
	}

	/* put first char of name or value back */
	ungetc(inchar, config_fp);

	if (read_mode == NAME) {
	    fscanf(config_fp, name_fm, name);

	    if (fgetc(config_fp) != ':') {
		fprintf(stderr, "ERROR: fpconf: Parameter name too long.");
		exit(-1);
	    }

	    read_mode = VALUE;
	    continue;
	}

	if (read_mode == VALUE) {
	    fscanf(config_fp, value_fm, value);
	    read_mode = NAME;
	}

	/* got a (name, value) pair */

	for (parsed = FALSE, cp = config_p; cp->arg_type != NOTYPE; cp++) {
	    if (strcasecmp (name, cp->LongName) == 0) {
		parsed = TRUE;
		bad_usage |= SetVal (cp, strdup(value));

	    }
	}

	if (!parsed) {
	    fprintf(stderr, "ERROR: fpconf: Unknown parameter %s\n", name);
	    bad_usage = TRUE;
	}
    }
    return (bad_usage);
}



void quit(err, fmt, str)
    int err;
    char *fmt, *str;
{
    printf(fmt, str);
    exit(err);
}

