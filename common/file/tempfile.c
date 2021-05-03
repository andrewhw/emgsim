/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: tempfile.c 17 2008-07-03 17:24:49Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#ifndef OS_WINDOWS_NT
#include        <unistd.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <dirent.h>
#else
#include        <io.h>
#endif
#endif

#include        "filetools.h"
#include        "stringtools.h"

/**
 * return a new temporary file name in a system independent
 * way.
 */
OS_EXPORT char *
allocTempFileName(
		const char *prefix
	)
{
	char *newTempName = NULL;


#ifdef OS_WINDOWS_NT
	{
		char *systemTempName;

		systemTempName = _tempnam("c:\\temp", prefix);
		if (systemTempName == NULL)
			systemTempName = _tempnam("c:\\WINNT\\temp", prefix);

		if (systemTempName == NULL)
			return NULL;

		newTempName = ckstrdup(systemTempName);
	}
#else
	{
		char *dirPrefix;
		int status;
		DIR *testdir;

		dirPrefix = "/usr/tmp";
		testdir = opendir(dirPrefix);
		if (testdir == NULL)
		{
			dirPrefix = "/var/tmp";
			testdir = opendir(dirPrefix);
			if (testdir == NULL)
			{
				dirPrefix = "/tmp";
			} else
			{
				closedir(testdir);
			}
		} else
		{
			closedir(testdir);
		}

		if (strstr(prefix, "XXXXXX") == NULL)
		{
			newTempName = strconcat(
									dirPrefix,
									OS_PATH_DELIM_STRING,
									prefix,
									"XXXXXX",
									NULL);
		} else
		{
			newTempName = strconcat(
									dirPrefix,
									OS_PATH_DELIM_STRING,
									prefix,
									NULL);
		}

		status = mkstemp(newTempName);
		if (status < 0)
		{
			ckfree(newTempName);
			return NULL;
		}
	}
#endif

	return newTempName;
}

