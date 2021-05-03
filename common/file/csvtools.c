/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: csvtools.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <ctype.h>
#include        <string.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#ifdef OS_WINDOWS_NT
#include        <io.h>
#else
#include        <unistd.h>
#endif
#endif

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


#include        "filetools.h"
#include        "listalloc.h"
#include        "tclCkalloc.h"


OS_EXPORT void
deleteStringVector(char **values, int nValues)
{
	int             i;

	if (values == NULL)
		return;

	for (i = 0; i < nValues; i++)
	{
		if (values[i] != NULL)
			ckfree(values[i]);
	}
	ckfree(values);
}

OS_EXPORT int
fileParseLine(FILE * fp, int *nValues, char ***values)
{
	static char     inputLine[BUFSIZ];
	int             lineNo = 0;
	char           *tokptr, *toktrim;
	int             nTokens = 0;
	int             nTokenBlocks = 0;

	if (fgets(inputLine, BUFSIZ, fp) != NULL)
	{
		lineNo++;

		/**
		 * Load all of the values
		 */
		tokptr = strtok(inputLine, ",");
		if (tokptr == NULL)
		{
			fprintf(stderr, "Cannot locate any tokens on line %d\n",
					lineNo);
			return 0;
		}
		/** trim whitespace from both ends and save value */
		while (isspace(*tokptr))
			tokptr++;
		toktrim = &tokptr[strlen(tokptr) - 1];
		while (isspace(*toktrim))
		{
			*(toktrim--) = 0;
		}
		listMkCheckSize(1,
						(void **) values,
						&nTokenBlocks,
						4,
						sizeof(char *),
						__FILE__, __LINE__);
		(*values)[nTokens++] = ckstrdup(tokptr);

		/** repeat for all tokens */
		while (1)
		{
			tokptr = strtok(NULL, ",");
			if (tokptr == NULL)
			{
				*nValues = nTokens;
				return 1;
			}
			/** trim whitespace from both ends and save value */
			while (isspace(*tokptr))
				tokptr++;
			listMkCheckSize(nTokens + 1,
							(void **) values,
							&nTokenBlocks,
							4,
							sizeof(char *),
							__FILE__, __LINE__);

			toktrim = &tokptr[strlen(tokptr) - 1];
			while (isspace(*toktrim))
			{
				*(toktrim--) = 0;
			}
			(*values)[nTokens++] = ckstrdup(tokptr);
		}
	}
	return 0;
}

OS_EXPORT int
parseDblFromStringVector(
		int nValues,
		double *floatValue,
		char **stringValue
	)
{
	int             i;

	for (i = 0; i < nValues; i++)
	{
		if (sscanf(stringValue[i], "%lf", &floatValue[i]) != 1)
		{
			fprintf(stderr,
					"Cannot parse float from '%s'\n", stringValue[i]);
			return 0;
		}
	}

	return 1;
}

