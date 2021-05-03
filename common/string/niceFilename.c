/** ------------------------------------------------------------
 ** Get rid of extra zeros at the end of a double
 ** ------------------------------------------------------------
 ** $Id: niceFilename.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#endif

#include <os_defs.h>


/** buffer shared by the below two functions */
static char     sBuffer[FILENAME_MAX + 1];

/*
 * -------------------------------------------------
 * Trim any extra zeros at the end of a double value
 * -------------------------------------------------
 */
OS_EXPORT char *
niceFilename(char *inputBuffer)
{
	int             i, j, len;

	j = 0;

	len = strlen(inputBuffer);
	for (i = 0; i < len; i++)
	{
		if (j >= FILENAME_MAX)
			break;
		if (isprint(inputBuffer[i])
			&& inputBuffer[i] != ' '
			&& inputBuffer[i] != '\t'
			&& inputBuffer[i] != '\''
			&& inputBuffer[i] != '"'
			&& inputBuffer[i] != '\n'
			&& inputBuffer[i] != '\r')
		{
			sBuffer[j++] = inputBuffer[i];
		}
	}
	sBuffer[j] = 0;

	return sBuffer;
}

