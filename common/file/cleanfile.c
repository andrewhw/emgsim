/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: cleanfile.c 11 2008-04-24 22:13:19Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#include        <ctype.h>
#endif
#include        "filetools.h"
#include        "tclCkalloc.h"

#ifdef  OS_WINDOWS_NT
static char    *sInvalidChars = ":;$#@!%^&*[]{}|/\\\t\"\'<>?`~., ";
#else
static char    *sInvalidChars = ";$!^&*[]|/\\\t\"\'<>?`~ ";
#endif
static char     sReplaceChar = '_';


OS_EXPORT char *
allocValidFileName(const char *sourceName)
{
	char           *result;
	int             i, j;
	int             len, inReplace;

	result = ckstrdup(sourceName);

	/* first, trim off any chars at the end which we don't like . . . */
	len = strlen(result);
	i = len - 1;
	while ((i > 0) && (strchr(sInvalidChars, result[i]) != NULL))
	{
		result[i--] = 0;
	}

	len = strlen(result);
	inReplace = 0;

	/* then step through the remaining string, making sure we are happy */
	for (i = 0; i < len; i++)
	{
		if (strchr(sInvalidChars, result[i]) != NULL)
		{

			/* if we are in a multi-character replace, shorten the string */
			if (inReplace)
			{
				for (j = i + 1; j <= len; j++)
				{
					result[j - 1] = result[j];
				}
				/* adjust the length and current offset accordingly */
				len--;
				i--;

			} else
			{
				/* overwrite the current character with the replacement char */
				result[i] = sReplaceChar;
				inReplace = 1;
			}
		} else
		{
			inReplace = 0;
		}
#ifdef   OS_WINDOWS_NT
		result[i] = tolower(result[i]);
#endif
	}

	return result;
}

