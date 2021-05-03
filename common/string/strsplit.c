/* ------------------------------------------------------------
 * Split a string into an array
 * ------------------------------------------------------------
 * $Id: strsplit.c 33 2009-04-26 13:48:18Z andrew $
 */

#ifndef MAKEDEPEND
#include    <stdio.h>
#include    <string.h>
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

#include    "tclCkalloc.h"


/*
 * ---------------------------------------------
 * go through a string, splitting based on the
 * dilimiters given, until we have filled up
 * the given array
 * ---------------------------------------------
 */
OS_EXPORT int
split(fill_array, delims, source, max)
    char          **fill_array;
    const char     *delims;
    const char     *source;
    int             max;
{
	int             n_found;
	char           *work, *newtok;

	work = ckstrdup(source);

	/** find the first value */
	if ((newtok = strtok(work, delims)) == NULL)
	{
		return 0;
	}
	fill_array[0] = ckstrdup(newtok);

	/**
	 * we have set up strtok, now call it in a loop until we see
	 * no more tokens
	 */
	n_found = 1;
	while (n_found < max)
	{
		newtok = strtok(NULL, delims);
		if (newtok == NULL)
		{
			break;
		}
		fill_array[n_found] = ckstrdup(newtok);
		n_found++;
	}

	ckfree(work);
	return (n_found);
}

