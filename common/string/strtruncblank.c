/** ------------------------------------------------------------
 ** Get rid of blanks at the end of a string.
 ** ------------------------------------------------------------
 ** $Id: strtruncblank.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <ctype.h>
#include                <string.h>
#endif

#include                "os_defs.h"

#include                "stringtools.h"


/*
 * trim whitespace from both ends of the string
 */
OS_EXPORT char *
strTrim(s)
    char *s;
{
	while (isspace(*s))
	{
		s++;
	}

	return strTruncBlank(s);
}



/*
 * trim whitespace from the end of the strings
 */
OS_EXPORT char *
strTruncBlank(s)
    char *s;
{
	char           *e = &s[strlen(s) - 1];

	while (e >= s && isspace(*e))
	{
		*e = 0;
		e--;
	}

	return (s);
}

