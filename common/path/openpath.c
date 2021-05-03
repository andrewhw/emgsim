/** ------------------------------------------------------------
 ** Various path handling utilities
 ** ------------------------------------------------------------
 ** $Id: openpath.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include        <os_defs.h>

#ifndef MAKEDEPEND
#include        <stdio.h>
#ifndef        OS_WINDOWS_NT
#include      <unistd.h>
#else
#include      <direct.h>
#endif
#include        <string.h>
#include        <errno.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#endif

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#include        "msgir.h"
#include        "msgstrerror.h"
#include        "tclCkalloc.h"
#include        "msgdbids.h"
#include        "massert.h"
#include        "stringtools.h"
#include        "pathtools.h"


/*
 * ---------------------------------------------
 * open a file in the given path with the
 * given perms
 * ---------------------------------------------
 */
int
opStatBuildPath(const char *confirmpath)
{
	char           *tokenpath, *curtok, *newpath, *oldpath = NULL;
	struct stat     statbuf;

	tokenpath = ckstrdup(confirmpath);
	MSG_ASSERT(tokenpath != NULL, "ckalloc failed");

	/** take off the file part of the tokenizable name **/
	newpath = strrchr(tokenpath, OS_PATH_DELIM);
	if (newpath != NULL)
	{
		/**
		 ** terminate the tokenizing string so we don't make
		 ** a dir for the terminating entry
		 **/
		*newpath = 0;

		/**
		 ** if the tokenpath starts with the delimiter, save it
		 **/
		if (tokenpath[0] == OS_PATH_DELIM)
		{
			oldpath = ckstrdup(OS_PATH_DELIM_STRING);
		}
		/**
		 ** in this loop, we use newpath as a means
		 ** to generate the path so far, which must
		 ** always be freed at the end of the loop
		 **
		 ** now open up all the intermediate levels
		 **/
		if ((curtok = strtok(tokenpath, OS_PATH_DELIM_STRING)) != NULL)
		{

			int             iteration = 0;

			do
			{
				if (oldpath == NULL)
				{
					newpath = ckstrdup(curtok);
				} else
				{
					newpath = strconcat(oldpath, OS_PATH_DELIM_STRING,
										curtok, NULL);
				}
				MSG_ASSERT(newpath != NULL, "ckalloc failed");

#ifdef          OS_WINDOWS_NT
				/**
				 *  on windows system, skip trying to create the drive itself,
				 *  as that doesn't seem to return the right stuff from stat.
				 */
				{
					int             len = strlen(newpath);
					if (iteration == 0 && len == 2 && newpath[1] == ':')
						goto SKIP_STAT;
				}
#endif                          /* OS_WINDOWS_NT */

				if ((irStat(newpath, &statbuf) < 0)
					&& (errno == ENOENT))
				{
					int             mkdirStatus;

#ifdef  OS_WINDOWS_NT
					mkdirStatus = _mkdir(newpath);
#else
					mkdirStatus = mkdir(newpath, 0777);
#endif

					if (mkdirStatus < 0)
					{
						LogErr("Failure from mkdir in buildpath : %s\n",
							   msgstrerror(errno));
						return (-1);
					}
				}
#ifdef  OS_WINDOWS_NT
		SKIP_STAT:
#endif
				if (oldpath != NULL)
					ckfree(oldpath);
				oldpath = newpath;

				/** get next token **/
				curtok = strtok(NULL, OS_PATH_DELIM_STRING);
				iteration++;
			} while (curtok != NULL);

			ckfree(newpath);
		}
	}
	ckfree(tokenpath);

	return (0);
}




/*
 * ---------------------------------------------
 * open a file in the given path with the
 * given perms
 * ---------------------------------------------
 */
OS_EXPORT int
openPath(filename, flags, perms)
    const char     *filename;
    int             flags, perms;
{
	int             fd;

	if (opStatBuildPath(filename) < 0)
		return (-1);

	/** now we can just open the file there **/
	fd = irOpen(filename, flags, perms);

	return (fd);
}




/*
 * ---------------------------------------------
 * open a dir in the given path with the
 * given perms
 * ---------------------------------------------
 */
OS_EXPORT int
openDirPath(dirname, perms)
    const char     *dirname;
    int             perms;
{
	if (opStatBuildPath(dirname) < 0)
		return (-1);

#ifdef  OS_WINDOWS_NT
	return _mkdir(dirname);
#else
	return (mkdir(dirname, perms));
#endif
}




/*
 * ---------------------------------------------
 * open a dir in the given path with the
 * given perms
 * ---------------------------------------------
 */
OS_EXPORT int
confirmDirPath(dirname, perms)
    const char     *dirname;
    int             perms;
{
	struct stat     sb;
	int             status;

	if (irStat(dirname, &sb) < 0)
	{
		status = openDirPath(dirname, perms);
		return (status);
	}
	return (1);
}

