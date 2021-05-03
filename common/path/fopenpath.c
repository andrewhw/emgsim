/** ------------------------------------------------------------
 ** Open a file, creating the directory path to it if necessary.
 ** ------------------------------------------------------------
 ** $Id: fopenpath.c 33 2009-04-26 13:48:18Z andrew $
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


#include "msgir.h"
#include "pathtools.h"
#include "stringtools.h"
#include "tclCkalloc.h"

/**
 ** Open a file, creating the directory path it resides in
 ** if necessary.  Path names will be converted to the
 ** appropriate OS convention.
 **/
OS_EXPORT FILE *
fopenpath(const char *constname, const char *mode)
{
	FILE           *result;
	char           *delimpos;
	char           *statdirname = NULL;
	char           *workingname = NULL;


	/** get the path in a form usable on this platform */
	workingname = osIndependentPath(constname);


	/** make sure the directory is there */
	delimpos = strrchr(workingname, OS_PATH_DELIM);

	/** if the path contains directories, make sure they exist */
	if (delimpos != NULL)
	{
		struct stat     sb;

#ifdef   OS_WINDOWS_NT
		/**
		 * for some bizarre reason, stat fails on
		 * bare drive names on Windows, so we skip
		 * the check if that is what we are about
		 * to do
		 */
		if (!(((delimpos - workingname) == 2)
			  && (workingname[1] == ':')))
#endif
		{

			statdirname = ckstrndup(workingname, (delimpos - workingname));
			if (irStat(statdirname, &sb) < 0)
			{
				if (mode[0] == 'w' || mode[0] == 'a')
				{
					if (openDirPath(statdirname, 0777) < 0)
					{
						LogErr("Cannot open dir \"%s\"\n", statdirname);
						goto FAIL;
					}
				} else
				{
					LogErr("Directory \"%s\" does not exist; cannot open file for read\n",
						   statdirname);
					goto FAIL;
				}
			}
		}
	}
	/** now open the file we want to open */
	result = fopen(workingname, mode);


	if (statdirname != NULL)
		ckfree(statdirname);
	ckfree(workingname);

	return result;


FAIL:
	if (statdirname != NULL)
		ckfree(statdirname);
	if (workingname != NULL)
		ckfree(workingname);
	return NULL;
}

