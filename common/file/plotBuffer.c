/** ------------------------------------------------------------
 ** log subsystem
 ** ------------------------------------------------------------
 ** $Id: plotBuffer.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include                "os_defs.h"

#ifndef MAKEDEPEND
#include                <stdio.h>
#include                <string.h>
#include                <stdarg.h>
#include                <errno.h>
#include                <sys/types.h>
#ifndef                OS_WINDOWS_NT
#include              <sys/fcntl.h>
#include              <unistd.h>
#else
#include              <fcntl.h>
#include              <io.h>
#endif
#include                <stdlib.h>
#include                <time.h>
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

#include                "pathtools.h"


/*
 * ---------------------------------------------
 * Log a buffer of floats to a file
 */
OS_EXPORT int
logFloatBuffer(float *data, long size, char *filenameFmt,...)
{
	char            filename[1024];
	va_list         vargs;
	FILE           *fp;
	long            i;

	va_start(vargs, filenameFmt);
	(void) vsprintf(filename, filenameFmt, vargs);
	va_end(vargs);

	fp = fopenpath(filename, "wb");

	for (i = 0; i < size; i++)
	{
		fprintf(fp, "%ld %f\n", i, data[i]);
	}

	fclose(fp);
	return 1;
}


/*
 * ---------------------------------------------
 * Log a buffer of longs to a file
 */
OS_EXPORT int
logLongBuffer(long *data, long size, char *filenameFmt,...)
{
	char            filename[1024];
	va_list         vargs;
	FILE           *fp;
	long            i;

	va_start(vargs, filenameFmt);
	(void) vsprintf(filename, filenameFmt, vargs);
	va_end(vargs);

	fp = fopenpath(filename, "wb");

	for (i = 0; i < size; i++)
	{
		fprintf(fp, "%ld %ld\n", i, data[i]);
	}

	fclose(fp);
	return 1;
}


/*
 * ---------------------------------------------
 * Log a buffer of shorts to a file
 */
OS_EXPORT int
logShortBuffer(short *data, long size, char *filenameFmt,...)
{
	char            filename[1024];
	va_list         vargs;
	FILE           *fp;
	long            i;

	va_start(vargs, filenameFmt);
	(void) vsprintf(filename, filenameFmt, vargs);
	va_end(vargs);

	fp = fopenpath(filename, "wb");

	for (i = 0; i < size; i++)
	{
		fprintf(fp, "%ld %d\n", i, data[i]);
	}

	fclose(fp);
	return 1;
}

