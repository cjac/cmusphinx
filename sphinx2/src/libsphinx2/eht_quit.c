/*
 *  quit  --  print message and exit
 *
 *  Usage:  quit (status,format [,arg]...);
 *	int status;
 *	(... format and arg[s] make up a printf-arglist)
 *
 *  Quit is a way to easily print an arbitrary message and exit.
 *  It is most useful for error exits from a program:
 *	if (open (...) < 0) then quit (1,"Can't open...",file);
 *
 **********************************************************************
 * HISTORY
 * $Log$
 * Revision 1.3  2001/12/07  04:27:35  lenzo
 * License cleanup.  Remove conditions on the names.  Rationale: These
 * conditions don't belong in the license itself, but in other fora that
 * offer protection for recognizeable names such as "Carnegie Mellon
 * University" and "Sphinx."  These changes also reduce interoperability
 * issues with other licenses such as the Mozilla Public License and the
 * GPL.  This update changes the top-level license files and removes the
 * old license conditions from each of the files that contained it.
 * All files in this collection fall under the copyright of the top-level
 * LICENSE file.
 * 
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:49  lenzo
 * Initial import of sphinx2
 *
 *
 * Revision 1.3  90/12/11  17:58:02  mja
 * 	Add copyright/disclaimer for distribution.
 * 
 * Revision 1.2  88/12/13  13:52:41  gm0w
 * 	Rewritten to use varargs.
 * 	[88/12/13            gm0w]
 * 
 **********************************************************************
 */

#include <stdio.h>
#include <stdarg.h>

void
quit (int status, char const *fmt, ...)
{
	va_list args;

	fflush(stdout);
	va_start(args, fmt);
	(void) vfprintf(stderr, fmt, args);
	va_end(args);
	exit(status);
}
