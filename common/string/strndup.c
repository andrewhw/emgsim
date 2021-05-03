/** ------------------------------------------------------------
 ** duplicate a string up to a given size
 ** ------------------------------------------------------------
 ** $Id: strndup.c 33 2009-04-26 13:48:18Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
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
 * similar to strdup, but a maximum length is
 * given
 * ---------------------------------------------
 */
OS_EXPORT char *
ckstrndup(cptr, size)
    const char     *cptr;
    int             size;
{
	char           *rptr;
	int             minsize;

	minsize = strlen(cptr);
	if (minsize > size)
	{
		minsize = size;
	}
	if ((rptr = (char *) ckalloc(minsize + 1)) == NULL)
		return (NULL);

	strncpy(rptr, cptr, minsize);
	rptr[minsize] = 0;

	return (rptr);
}

