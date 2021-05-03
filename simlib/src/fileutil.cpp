/**
 **
 ** Module:                             FILEUTIL.C  source module
 **
 ** Author:                             Rodrick  Naphan
 ** Date:                               Jan. 18, 1990
 **
 ** Description:
 ** Contains the routine to create and update binary
 ** configuration and data files.
 **
 ** $Id: fileutil.cpp 13 2011-03-23 11:22:44Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKEDEPEND
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/stat.h>
# ifndef OS_WINDOWS_NT
#  include <unistd.h>
# else
#  include <io.h>
# endif
#endif

#include "log.h"
#include "stringtools.h"
#include "pathtools.h"
#include "tclCkalloc.h"
#include "msgir.h"

#include "SimulatorControl.h"
#include "error.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

/**
 * Find the list of file names matching the given pattern, up
 * to the declared maximum id number
 */
int
statFilenameFromMask(
		const char *directory,
		const char *filenameMask,
		...
	)
{
	char filename[FILENAME_MAX];
	struct stat sb;
	va_list vargs;
	int len;

	strncpy(filename, directory, FILENAME_MAX);
	len = strlen(filename);
	if (filename[len - 1] != OS_PATH_DELIM)
	{
		filename[len++] = OS_PATH_DELIM;
		filename[len] = 0;
	}
	va_start(vargs, filenameMask);
#if defined(OS_HAS_SNPRINTF)
	vsnprintf(&filename[len],
			BUFSIZ - len,
			filenameMask, vargs);
#else
	vsprintf(&filename[len], filenameMask, vargs);
#endif
	va_end(vargs);

	if (irStat(filename, &sb) < 0)
	{
		return 0;
	}

	return 1;
}

