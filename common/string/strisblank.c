/** ------------------------------------------------------------
 ** is a string made of blanks?
 ** ------------------------------------------------------------
 ** $Id: strisblank.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include        <ctype.h>
#endif

#include        "os_defs.h"


/*
 * is a string made of whitespace?
 */
OS_EXPORT int
strIsBlank(s)
    const char     *s;
{
	while (*s && isspace(*s))
		s++;
	if (*s)
		return (0);
	return (1);
}

