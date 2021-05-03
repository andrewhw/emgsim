/** ------------------------------------------------------------
 ** time string management
 ** ------------------------------------------------------------
 ** $Id: strTimeDelta.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
#include                <stdlib.h>
#include                <limits.h>
#include                <time.h>
#include                <ctype.h>
#endif

#include                "os_defs.h"


/*
 * ---------------------------------------------
 * parse a string and give back a delta in time_t
 * ---------------------------------------------
 */
OS_EXPORT int
strTimeToDelta(string)
    const char     *string;
{
	long ival, delta = 0;
	char *end;

	while (*string)
	{

		if (*string == 0)
			return (-1);

		ival = strtol(string, &end, 10);
		if (*end && !isspace(*end))
		{
			switch (*end)
			{
			case 'h':
			case 'H':
				ival *= (60 * 60);
				break;

			case 'm':
			case 'M':
				ival *= 60;
				break;

			case 's':
			case 'S':
				/** do nothing **/
				break;

			default:
				/** bad character found         **/
				return (-1);
			}
		}
		delta += ival;
		string = end + 1;
	}
	return (delta);
}

