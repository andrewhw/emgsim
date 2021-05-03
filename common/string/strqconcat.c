/** ------------------------------------------------------------
 ** Concatenate and quote a string
 ** ------------------------------------------------------------
 ** $Id: strqconcat.c 34 2009-04-26 14:22:18Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
#include                <stdarg.h>
#include                <string.h>
#endif

#include "os_defs.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#include                "tclCkalloc.h"


/*
 * ---------------------------------------------
 * Concatenate a set of strings together,
 * quoting all arguments,
 * ckalloc'ing memory for the purpose.
 *
 * End of list is indicated by a NULL pointer
 * ---------------------------------------------
 */
OS_EXPORT char *
strqconcat(const char *leadptr,...)
{
	va_list         vargs;
	const char     *argptr;
	char           *curptr;
	char           *retptr = NULL;
	int             total_len = 1;


	/* sum the lengths                              */
	va_start(vargs, leadptr);
	argptr = leadptr;
	while (argptr != NULL)
	{
		total_len += strlen(argptr) + 2;
		argptr = va_arg(vargs, char *);
	}
	va_end(vargs);


	if ((retptr = (char *) ckalloc(total_len)) == NULL)
		return (NULL);

	curptr = retptr;

	/* now paste in the strings             */
	va_start(vargs, leadptr);
	argptr = leadptr;
	while ((argptr = va_arg(vargs, char *)) != NULL)
	{
		*curptr++ = '"';
		strcpy(curptr, argptr);
		curptr += strlen(argptr);
		*curptr++ = '"';
	}
	va_end(vargs);


	return (retptr);
}

