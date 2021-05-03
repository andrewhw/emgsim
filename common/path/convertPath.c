/**
 ** ----------------------------------------------------------------
 ** Convert a path over to a form useful in the current OS
 ** ----------------------------------------------------------------
 **
 ** $Id: convertPath.c 11 2008-04-24 22:13:19Z andrew $
 **/
#include "os_defs.h"

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifndef                OS_WINDOWS_NT
#include <unistd.h>
#endif
#endif


#include "msgir.h"
#include "pathtools.h"
#include "stringtools.h"
#include "tclCkalloc.h"

/**
 ** Convert a path to a form proper for the
 ** current OS.
 **/
OS_EXPORT char *
osIndependentPath(const char *constname)
{
	char           *delimpos;
	char           *workingName;
#ifndef         OS_WINDOWS_NT

	{
		const char     *copyName;

		/*
		 * if the name starts with "?:", skip it.
		 */
		if (constname[1] != ':')
		{
			copyName = constname;
		} else
		{
			copyName = &constname[2];
			if (*copyName == OS_PATH_DELIM
				|| *copyName == OS_PATH_INVALID_DELIM)
				copyName = (copyName + 1);
		}
		workingName = ckstrdup(copyName);
	}
#else

	/*
	 * if we are in the "root" directory,
	 * prepend a "c:" to the name
	 */
	if (constname[0] == OS_PATH_DELIM
		|| constname[0] == OS_PATH_INVALID_DELIM)
	{
		workingName = strconcat("c:", constname, NULL);
	} else
	{
		workingName = ckstrdup(constname);
	}

#endif



	/** make sure we don't have a zero length name */
	if (*workingName == 0)
	{
		goto FAIL;
	}
	/** convert between "\" and "/" in case of OS mixup */
	while ((delimpos = strchr(workingName, OS_PATH_INVALID_DELIM))
		   != NULL)
	{
		*delimpos = OS_PATH_DELIM;
	}

	return workingName;

FAIL:
	if (workingName != NULL)
		ckfree(workingName);
	return NULL;
}

