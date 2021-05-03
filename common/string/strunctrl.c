/** ------------------------------------------------------------
 ** Make an unprintable string printable
 ** ------------------------------------------------------------
 ** $Id: strunctrl.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#endif

#include "tclCkalloc.h"
#include "stringtools.h"


static char    *sStringBuffer, sConversionBuffer[5];
static int      sBufferSize = 0;



/*
 * ---------------------------------------------
 * go through a string, generating a string
 * of 'unctrl' substrings for each character.
 *
 * Control characters will be in the form ^X
 * High bit charcters will be in the form \000
 * Other characters will be passed verbatim
 *
 * This function manages its own internal
 * buffer -- as such it should not be called
 * more than once in the same stack reference,
 * as the buffer will appear duplicated
 * ---------------------------------------------
 */
OS_EXPORT char *
strunctrl(s, len)
    const char *s;
    int len;
{
	int curlen, i;

	/** get a big enough buffer         **/
	if (sBufferSize < len * 4 + 1)
	{

		if (sBufferSize != 0)
		{
			free(sStringBuffer);
		}
		if ((sStringBuffer = (char *) malloc(len * 4 + 1)) == NULL)
		{
			perror("malloc failed");
			return (NULL);
		}
		sBufferSize = len * 4 + 1;
	}
	sStringBuffer[0] = 0;

	/** print out into the buffer               **/
	for (i = 0; i < len; i++)
	{

		if ((unsigned char) s[i] > (unsigned char) '~')
		{
			slnprintf(sConversionBuffer, 5,
							"\\%03o", (unsigned char) s[i]);
		} else if ((unsigned char) s[i] < (unsigned char) ' ')
		{
			sConversionBuffer[0] = '^';
			sConversionBuffer[1] = s[i] + '@';
			sConversionBuffer[2] = '\0';
		} else
		{
			sConversionBuffer[0] = s[i];
			sConversionBuffer[1] = '\0';
		}
		curlen = strlen(sStringBuffer);
		slnprintf(&sStringBuffer[curlen], sBufferSize - curlen,
				"%s",
				sConversionBuffer);
	}
	return (sStringBuffer);
}






/*
 * ---------------------------------------------
 * Same, but for 1 char
 * ---------------------------------------------
 */
OS_EXPORT char *
chunctrl(c)
    int             c;
{
	static char     buf[5];

	/* print out into the buffer                */
	if (c > '~')
	{
		slnprintf(buf, 5, "\\%03o", c);
	} else if (c < ' ')
	{
		buf[0] = '^';
		buf[1] = c + '@';
		buf[2] = '\0';
	} else
	{
		buf[0] = c;
		buf[1] = '\0';
	}

	return (buf);
}

