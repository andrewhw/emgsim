/** ------------------------------------------------------------
 ** do a sensible, portable thing when we have a panic
 ** ------------------------------------------------------------
 ** $Id: panic.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <stdlib.h>
#include        <stdarg.h>
#endif

#include        "os_defs.h"


OS_EXPORT void
panic(char *message,...)
{
	va_list         args;

	va_start(args, message);
	(void) fputs("System Panic : ", stderr);
	(void) vfprintf(stderr, message, args);
	(void) fputc('\n', stderr);

	abort();
	/* NOTREACHED */
}

