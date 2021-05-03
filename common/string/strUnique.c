/** ------------------------------------------------------------
 ** get a set of unique strings from within an application
 ** ------------------------------------------------------------
 ** $Id: strUnique.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
#endif

#include                "os_defs.h"


#define                         U_BUF_SIZE              8
/*
 * ---------------------------------------------
 * return the pid, as a string
 * ---------------------------------------------
 */
OS_EXPORT char *
strUnique()
{
	static int      count = 0;
	static char     obuf[U_BUF_SIZE];
	int             start = U_BUF_SIZE - 1, thiscount;

	thiscount = count++;
	obuf[start--] = 0;
	while (start > 1)
	{
		if (thiscount > 0)
		{
			obuf[start--] = 'a' + thiscount % 26;
			thiscount /= 26;
		} else
		{
			obuf[start--] = 'a';
		}
	}
	obuf[1] = '_';
	obuf[0] = 'U';

	return (obuf);
}

