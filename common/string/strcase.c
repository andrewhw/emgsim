/** ------------------------------------------------------------
 ** duplicate a string converting to fixed case, using ckalloc
 ** ------------------------------------------------------------
 ** $Id: strcase.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#endif

#include "tclCkalloc.h"
#include "stringtools.h"

char *
strtoupper(cptr)
    char           *cptr;
{
	char           *workptr;

	workptr = cptr;
	while (*workptr)
	{
		*workptr = toupper(*workptr);
		workptr++;
	}

	return (cptr);
}

char *
ckstrdup_toupper(cptr)
    const char     *cptr;
{
	char *rptr, *modptr;
	int len;

	len = strlen(cptr) + 1;
	if ((rptr = (char *) ckalloc(len)) == NULL)
		return (NULL);

	strlcpy(rptr, cptr, len);
	modptr = rptr;
	while (*modptr)
	{
		*modptr = toupper(*modptr);
		modptr++;
	}

	return (rptr);
}

char *
strtolower(cptr)
    char           *cptr;
{
	char           *workptr;

	workptr = cptr;
	while (*workptr)
	{
		*workptr = tolower(*workptr);
		workptr++;
	}

	return (cptr);
}

char *
ckstrdup_tolower(cptr)
    const char     *cptr;
{
	char *rptr, *modptr;
	int len;

	len = strlen(cptr) + 1;
	if ((rptr = (char *) ckalloc(len)) == NULL)
		return (NULL);

	strlcpy(rptr, cptr, len);
	modptr = rptr;
	while (*modptr)
	{
		*modptr = tolower(*modptr);
		modptr++;
	}

	return (rptr);
}

