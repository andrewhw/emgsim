/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: copyfile.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#ifdef OS_WINDOWS_NT
#include        <io.h>
#else
#include        <unistd.h>
#endif
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


#include        "filetools.h"


OS_EXPORT int
copyFile(const char *targetName, const char *srcName)
{
	int             srcfd, trgfd, status;

#ifdef OS_WINDOWS_NT
	srcfd = _open(srcName, O_RDONLY | O_BINARY, 0);
#else
	srcfd = open(srcName, O_RDONLY, 0);
#endif
	if (srcfd < 0)
	{
		return 0;
	}
#ifdef OS_WINDOWS_NT
	trgfd = _open(targetName, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
#else
	trgfd = open(targetName, O_CREAT | O_TRUNC | O_WRONLY,
#endif
				 0666);
	if (srcfd < 0)
	{
		return 0;
	}
	status = copyFileContents(trgfd, srcfd);

#ifdef OS_WINDOWS_NT
	_close(srcfd);
	_close(trgfd);
#else
	close(srcfd);
	close(trgfd);
#endif

	return status;
}

OS_EXPORT int
copyFileContents(int trgfd, int srcfd)
{
	unsigned char   buffer[BUFSIZ * 32];
	size_t          nRead, nWritten;

#ifdef OS_WINDOWS_NT
	while ((nRead = _read(srcfd, buffer, BUFSIZ * 32)) > 0)
#else
	while ((nRead = read(srcfd, buffer, BUFSIZ * 32)) > 0)
#endif
	{
#ifdef OS_WINDOWS_NT
		nWritten = _write(trgfd, buffer, nRead);
#else
		nWritten = write(trgfd, buffer, nRead);
#endif
		if (nWritten != nRead)
			return 0;
	}

	return 1;
}

OS_EXPORT int
copyFileIfPresent(int trgfd, const char *srcName)
{
	int             srcfd, status;

#ifdef OS_WINDOWS_NT
	srcfd = _open(srcName, O_RDONLY | O_BINARY, 0);
#else
	srcfd = open(srcName, O_RDONLY, 0);
#endif
	if (srcfd < 0)
	{
		return 0;
	}
	status = copyFileContents(trgfd, srcfd);

#ifdef OS_WINDOWS_NT
	_close(srcfd);
#else
	close(srcfd);
#endif

	return status;
}

