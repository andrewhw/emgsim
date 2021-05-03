/** ------------------------------------------------------------
 ** debug error messages
 ** ------------------------------------------------------------
 ** $Id: Derror.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <errno.h>
#include        <stdarg.h>
#include        <string.h>
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


#include        "stringtools.h"
#include        "error.h"
#include        "log.h"

/*
 * ---------------------------------------------
 * debug error message -- this is not macro
 * dependant.  Form is:
 * Derror(__FILE__,__LINE__, "...", ...);
 * ---------------------------------------------
 */
OS_EXPORT int
Derror(const char *file, int line, const char *fmt,...)
{
	va_list         vargs;
	char            obuf[4096];

	slnprintf(obuf, 4096, "Error at (%d) in '%s':\n", line, file);

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(&obuf[strlen(obuf)], 4096, fmt, vargs);
#else
	(void) vsprintf(&obuf[strlen(obuf)], fmt, vargs);
#endif
	va_end(vargs);


	if (obuf[strlen(obuf) - 1] != '\n')
		slnprintf(&obuf[strlen(obuf)], 4096, "\n");

	LogCrit("Derror: %s", obuf);

	return (1);
}



/*
 * ---------------------------------------------
 * General error message -- this is not macro
 * dependant.  Form is:
 * Error("...", ...);
 * ---------------------------------------------
 */
OS_EXPORT int
Error(const char *fmt,...)
{
	va_list         vargs;
	char            obuf[4096];

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(obuf, 4096, fmt, vargs);
#else
	(void) vsprintf(obuf, fmt, vargs);
#endif
	va_end(vargs);


	if (obuf[strlen(obuf) - 1] != '\n')
		slnprintf(&obuf[strlen(obuf)], 4096, "\n");

	LogErr("%s", obuf);

	return (1);
}


/*
 * ---------------------------------------------
 * General error message -- this is not macro
 * dependant.  Form is:
 * Warning("...", ...);
 * ---------------------------------------------
 */
OS_EXPORT int
Warning(const char *fmt,...)
{
	va_list         vargs;
	char            obuf[4096];

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(obuf, 4096, fmt, vargs);
#else
	(void) vsprintf(obuf, fmt, vargs);
#endif
	va_end(vargs);


	if (obuf[strlen(obuf) - 1] != '\n')
		slnprintf(&obuf[strlen(obuf)], 4096, "\n");

	LogWarn("%s", obuf);

	return (1);
}

