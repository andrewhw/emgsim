/**
 ** Beginning of a general error logging/lookup routine.
 **
 ** $Id: logwrite.cpp 4 2008-04-24 21:27:41Z andrew $
 **/
#include        "os_defs.h"

#ifndef MAKEDEPEND
# include       <stdio.h>
# include       <stdarg.h>
# include       <errno.h>
# include       <string.h>
#endif

#include        "pathtools.h"
#include        "logwrite.h"
#include        "stringtools.h"


/*
#define         KEEP_LOG_OUTPUT_FILE
*/


#ifdef  KEEP_LOG_OUTPUT_FILE
static FILE             *noteFP = NULL;

#define FILENAME                "logwrite.txt"

static void
setup()
{
	if (noteFP == NULL)
	{
		noteFP = fopenpath(FILENAME, "w");
		if (noteFP == NULL)
		{
			fprintf(stderr, "Cannot open log output file '%s' : %s\n",
					FILENAME, strerror(errno));
		}
	}
}
#endif

/**
 * Print out an error message at the bottom, in reverse
 */
void logNote(const char *fmt, ...)
{
#ifdef  KEEP_LOG_OUTPUT_FILE
	va_list     vargs;

	setup();

	va_start(vargs, fmt);
	(void) vfprintf(noteFP, fmt, vargs);
	va_end(vargs);
#endif
}

size_t
logfwrite(void *ptr, const char *tag, const char *file, int line,
		        size_t size, size_t nelem, FILE *fp)
{
#ifdef  KEEP_LOG_OUTPUT_FILE
	size_t i;
	logNote("\nfwrite data %d * size %d \"%s\" - %s(%d)\n", nelem, size,
					tag, file, line);

	if (nelem > 1)
	{
			for (i = 0; i < nelem && i < 20; i++)
			{

					if (size == sizeof(int))
					{

							logNote("    %3d int : %d\n", i, ((int *)ptr)[i]);

					} else if (size == sizeof(short))
					{
							logNote("    %3d int : %hd\n", i, ((short *)ptr)[i]);

					} else if (size == sizeof(long))
					{
							logNote("    %3d int : %ld\n", i, ((long *)ptr)[i]);

					} else if (size == sizeof(float))
					{
							logNote("    %3d int : %f\n", i, ((float *)ptr)[i]);

					} else if (size == sizeof(double))
					{
							logNote("    %3d int : %f\n", i, ((float *)ptr)[i]);

					} else
					{
							logNote("    %3d ??? : [%s]\n", i,
											strunctrl((const char *) ptr, size));
					}
			}
	} else
	{

		if (size == sizeof(int))
		{
			logNote("    int : %d\n", *((int *)ptr));

		} else if (size == sizeof(short))
		{
			logNote("    int : %hd\n", *((short *)ptr));

		} else if (size == sizeof(long))
		{
			logNote("    int : %ld\n", *((long *)ptr));

		} else if (size == sizeof(float))
		{
			logNote("    int : %f\n", *((float *)ptr));

		} else if (size == sizeof(double))
		{
			logNote("    int : %f\n", *((float *)ptr));

		} else
		{
			logNote("    ??? : [%s]\n",
										strunctrl((const char *) ptr, size));
		}
	}
#endif
	return fwrite(ptr, size, nelem, fp);
}


