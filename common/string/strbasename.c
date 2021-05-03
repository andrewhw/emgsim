/** ------------------------------------------------------------
 ** get the base name from a directory name
 ** ------------------------------------------------------------
 ** $Id: strbasename.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <string.h>
#endif
#include                "os_defs.h"

/*
 * ---------------------------------------------
 * similar to strdup, but a maximum length is
 * given
 * ---------------------------------------------
 */
OS_EXPORT const char *
strbasename(const char *string)
{
	const char     *mark;

	mark = strrchr(string, OS_PATH_DELIM);
	if (mark != NULL)
		return (++mark);

	return (string);
}

