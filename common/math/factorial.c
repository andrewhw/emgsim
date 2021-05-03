/** ------------------------------------------------------------
 ** Calculate a factorial.  Is this really not in the library?
 ** ------------------------------------------------------------
 ** $Id: factorial.c 55 2009-12-30 16:57:44Z andrew $
 **/

#ifndef MAKEDEPEND
#include       <stdio.h>
#include       <stdlib.h>
#include       <math.h>
#include       <time.h>
#include       <limits.h>
#endif

#include        "os_defs.h"
#include        "mathtools.h"

OS_EXPORT long 
factorial(long val)
{
	long result = 1, preresult = 1;

	if (val > 0)
	{
		while (val > 1)
		{
			result = preresult * val;

			/** check for overflow */
			if (result < preresult) return (-1);

			preresult = result;
			val--;
		}
		return result;
	}
	return 0;
}

