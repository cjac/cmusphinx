/*
 * logmsg.c - verbosity-controlled internal log messages.
 */

#include <stdio.h>
#include <stdarg.h>

#include "s2types.h"
extern int32 verbosity_level;

#define INST_LOGGER(name, fh, level)		\
void name (char const *fmt, ...)		\
{						\
    va_list args;				\
						\
    va_start(args, fmt);			\
    if (verbosity_level >= level) {		\
	vfprintf(fh, fmt, args);		\
	fflush(fh);				\
    }						\
    va_end(args);				\
}

/* XXX: yeah, maybe these should be macros in the header file that
   call a common function, however it might be useful to have them do
   different things in the future. */
INST_LOGGER(log_error, stderr, 0)
INST_LOGGER(E_WARN, stderr, 1)
INST_LOGGER(E_INFO, stdout, 2)
INST_LOGGER(log_debug, stdout, 3)

/*
 * Local variables:
 *  c-basic-offset: 4
 *  c-indentation-style: "BSD"
 * End:
 */
