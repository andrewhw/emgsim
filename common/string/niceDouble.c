/** ------------------------------------------------------------
 ** Get rid of extra zeros at the end of a double
 ** ------------------------------------------------------------
 ** $Id: niceDouble.c 56 2010-01-09 18:40:35Z andrew $
 **/

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#include <math.h>
#endif

#include <os_defs.h>
#include <stringtools.h>


/** buffer shared by the below two functions */
static char     sBuffer[128];

/*
 * -------------------------------------------------
 * Trim any extra zeros at the end of a double value
 * -------------------------------------------------
 */
OS_EXPORT void
trimFloatBuffer(char *buffer)
{
	int             i;

	/** if there is no decimal point (for some reason) we cannot trim */
	if (strchr(buffer, '.') == NULL)
		return;

	i = strlen(buffer) - 1;
	while (i > 0 && buffer[i] == '0' && buffer[i - 1] != '.')
	{
		buffer[i] = 0;
		i--;
	}

	return;
}


/*
 * -------------------------------------------------
 * Trim any extra zeros at the end of a double value
 * -------------------------------------------------
 */
OS_EXPORT char *
niceDouble(double value)
{
	if (fabs(value) > 1e-6)
	{
		slnprintf(sBuffer, 128, "%.7f", value);
		trimFloatBuffer(sBuffer);
	}
	else
	{
		slnprintf(sBuffer, 128, "%.7g", value);
	}
	return sBuffer;
}


/*
 * -------------------------------------------------
 * Trim any extra zeros at the end of a double value
 * -------------------------------------------------
 */
OS_EXPORT char *
fullyTrimmedDouble(double value)
{
	int             len;

	(void) niceDouble(value);
	len = strlen(sBuffer);
	if (sBuffer[len - 1] == '0' && sBuffer[len - 2] == '.')
	{
		sBuffer[len - 2] = 0;
	}
	return sBuffer;
}

