/** ------------------------------------------------------------
 ** time string management
 ** ------------------------------------------------------------
 ** $Id: strTimeString.c 33 2009-04-26 13:48:18Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
#include                <stdlib.h>
#include                <limits.h>
#include                <time.h>
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
#include                "massert.h"


/*
 * ---------------------------------------------
 * take a time, and give back a string
 * ---------------------------------------------
 */
OS_EXPORT char *
strTimeToString(delta)
    time_t          delta;
{
	static char    *retbuf = NULL;

	if (retbuf == NULL)
	{
		retbuf = (char *) ckalloc(128);
		MSG_ASSERT(retbuf != NULL, "ckalloc failed");
	}

	sprintf(retbuf, "%d hours %d mins %d secs",
				 ((int) delta) / (60 * 60),
				(((int) delta) % (60 * 60)) / 60,
				 ((int) delta) % 60);

	return (retbuf);
}

