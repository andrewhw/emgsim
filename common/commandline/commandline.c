/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: commandline.c 11 2008-04-24 22:13:19Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#endif
#include        "commandline.h"
#include        "stringtools.h"
#include        "pathtools.h"
#include        "massert.h"


/**
 * Record the arguments passed.
 */
OS_EXPORT int
recordCommandline(const char *directory, int argc, char *const * argv)
{
	char *progname;
	char *filename;
	char *tmp;
	FILE *tfp;
	int i, len;

	tmp = strrchr(argv[0], OS_PATH_DELIM);
	if (tmp != NULL)
	{
		progname = ckstrdup(++tmp);
	} else
	{
		progname = ckstrdup(argv[0]);
	}
	MSG_ASSERT(progname != NULL, "Out of memory");


	/** check if we have an .exe extension */
	len = strlen(progname);
	if (len > 4)
	{
		tmp = &progname[len - 4];
		if (strcmp(tmp, ".exe") == 0)
		{
			*tmp = 0;
		}
	}
	/** set up the filename */
	if (directory[strlen(directory) - 1] == OS_PATH_DELIM)
	{
		filename = strconcat(directory, progname, "-flags.txt", NULL);
	} else
	{
		filename = strconcat(directory, OS_PATH_DELIM_STRING,
							 progname, ".flags", NULL);
	}

	MSG_ASSERT(filename != NULL, "Out of memory");

	tfp = fopenpath(filename, "w");

	/** clean up our buffers */
	ckfree(filename);


	/** store the values if we can */
	if (tfp != NULL)
	{

		fprintf(tfp,
				"\n# %s was run with the following options:\n\n",
				progname);

		fputs(argv[0], tfp);
		for (i = 1; i < argc; i++)
		{
			fputc(' ', tfp);
			fputs(argv[i], tfp);
		}
		fputc('\n', tfp);
		fputc('\n', tfp);
		fclose(tfp);
	}
	ckfree(progname);

	return 1;
}

