/** ------------------------------------------------------------
 ** Tools to help find files
 ** ------------------------------------------------------------
 ** $Id: pathtools.c 11 2008-04-24 22:13:19Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <stdarg.h>
#include        <string.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#ifdef  OS_WINDOWS_NT
#include       <io.h>
#else
#include       <unistd.h>
#endif
#include        <errno.h>
#endif

#include        "tclCkalloc.h"
#include        "stringtools.h"
#include        "pathtools.h"
#include        "os_defs.h"


/*
 * ---------------------------------------------
 * Return the stem of a file name by truncating
 * any of the given extensions from it
 * ---------------------------------------------
 */
OS_EXPORT char *
getPathStem(const char *filename,...)
{
	va_list         vargs;
	const char     *thisExt;
	char           *retptr = NULL;
	int             lenExt, lenHead, lenBase;


	/*
	 * iterate over the extensions given,
	 * matching the end of the filename
	 */
	lenBase = strlen(filename);
	va_start(vargs, filename);
	thisExt = va_arg(vargs, char *);
	while (thisExt != NULL)
	{
		lenExt = strlen(thisExt);
		lenHead = lenBase - lenExt;
		if (lenHead > 0)
		{
			/*
			 * if we get a match, copy the stem for the match and return
			 * that, making sure we terminate
			 */
			if (strcmp(&filename[lenHead], thisExt) == 0)
			{
				retptr = (char *) ckalloc(lenHead + 1);
				strncpy(retptr, filename, lenHead);
				retptr[lenHead] = 0;
				/* break, not return, to cash in on the va_end */
				break;
			}
		}
		thisExt = va_arg(vargs, char *);
	}
	va_end(vargs);

	/* if we found no matches, just pass back what we were given */
	if (retptr == NULL)
		return ckstrdup(filename);

	return (retptr);
}

/*
 * ---------------------------------------------
 * Return the file name of the first valid file
 * that can be built using the stem given and
 * the extensions supplied
 * ---------------------------------------------
 */
OS_EXPORT char *
getExistingFile(const char *stubname,...)
{
	va_list         vargs;
	const char     *thisExt;
	char           *thisAttempt, *thisOsIndep;
	int             statResult;
	struct stat     sb;


	/*
	 * iterate over the extensions given,
	 * matching the end of the filename
	 */
	va_start(vargs, stubname);
	thisExt = va_arg(vargs, char *);
	while (thisExt != NULL)
	{
		thisAttempt = strconcat(stubname, thisExt, NULL);
		thisOsIndep = osIndependentPath(thisAttempt);

		ckfree(thisAttempt);

		statResult = stat(thisOsIndep, &sb);

		/* if we opened one successfully, then return it! */
		if (statResult >= 0)
		{
			return thisOsIndep;

		} else if (errno != ENOENT)
		{
			/* then we have a failure -- leave errno as is */
			ckfree(thisOsIndep);
			return NULL;
		}
		/* otherwise the file just wasn't found, so continue */
		thisExt = va_arg(vargs, char *);
	}
	va_end(vargs);

	/* if we found no matches, set errno and return */
	errno = ENOENT;
	return NULL;
}


/*
 * ---------------------------------------------
 * Return the file name of the first valid file
 * that can be built using the stem given and
 * the extensions supplied
 * ---------------------------------------------
 */
OS_EXPORT char *
getUniqueFile(const char *stubname)
{
	struct stat     sb;
	int             statResult;
	int             counter = 1;
	char            counterBuffer[16];
	char           *result;

	/*
	 * Given the supplied name, ensure that the
	 * directory exists and is writable, and
	 * then locate a name in it by numbering the
	 * filename given until we have a valid one
	 */

	/** first make sure the directory is there */
	if (opStatBuildPath(stubname) < 0)
	{
		return NULL;
	}
	/** check whether we are ok from the start */
	statResult = stat(stubname, &sb);
	if ((statResult < 0) && (errno == ENOENT))
	{
		return ckstrdup(stubname);
	}
	/**
	 ** if stat returned failure for any other reason,
	 ** we have to fail too
	 **/
	if (statResult < 0)
	{
		return NULL;
	}
	/** now loop around in this directory to find an unused name */
	for (counter = 1; counter < 4096; counter++)
	{
		sprintf(counterBuffer, "%d", counter);
		result = strconcat(stubname, counterBuffer, NULL);


		statResult = stat(result, &sb);
		if ((statResult < 0) && (errno == ENOENT))
		{
			return result;
		}
		ckfree(result);

		if (statResult < 0)
		{
			return NULL;
		}
	}

	return NULL;
}

