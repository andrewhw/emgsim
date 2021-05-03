/** 
 * ------------------------------------------------------------
 * Safely print to a string, a la BSD snprintf, but do so portably.
 * ------------------------------------------------------------
 * $Id: slnprintf.c 81 2010-07-12 19:38:05Z andrew $
 */

#include "os_defs.h"

#ifndef MAKEDEPEND
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
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



/*
 * Print to a string using system libs and bounds if possible
 */
OS_EXPORT int
slnprintf(char *buffer, int bufferSize, const char *fmt, ...)
{
	va_list vargs;
	int result;

	va_start(vargs, fmt);

#ifdef		OS_HAS_SNPRINTF
	result = vsnprintf(buffer, bufferSize, fmt, vargs);
#else

# ifdef		OS_WINDOWS_NT
	result = _vsnprintf(buffer, bufferSize, fmt, vargs);
# else
	result = vsprintf(buffer, fmt, vargs);
# endif
#endif

	va_end(vargs);

	return result;
}

#ifdef		OS_NEEDS_STRLCPY

OS_EXPORT size_t
strlcpy(char *dst, char *src, size_t buflen)
{
	size_t srclen;

	srclen = strlen(src);

	if (srclen < buflen)
	{
		strcpy(dst, src);
	} else
	{
		strncpy(dst, src, buflen);
		dst[buflen-1] = 0;
	}

	return srclen;
}

OS_EXPORT size_t
strlcat(char *dst, char *src, size_t buflen)
{
	size_t srclen;
	size_t curlen;
	size_t emptylen;

	srclen = strlen(src);
	curlen = strlen(dst);

	if ((srclen + curlen) < buflen)
	{
		strcpy(&dst[curlen], src);
	} else 
	{
		emptylen = buflen - curlen;
		strncpy(&dst[curlen], src, emptylen);
		dst[buflen-1] = 0;
	}

	return srclen;
}

#endif


