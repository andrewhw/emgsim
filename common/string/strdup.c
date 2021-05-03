/** ------------------------------------------------------------
 ** duplicate a string, using malloc
 ** ------------------------------------------------------------
 ** $Id: strdup.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
#endif

#include                "tclCkalloc.h"


#ifdef          ultrix

char           *
strdup(cptr)
    const char     *cptr;
{
	char           *rptr;

	if ((rptr = (char *) malloc(strlen(cptr) + 1)) == NULL)
		return (NULL);

	strcpy(rptr, cptr);

	return (rptr);
}

#endif

