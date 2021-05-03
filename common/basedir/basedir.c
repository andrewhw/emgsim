/** ------------------------------------------------------------
 ** directory name handling tools
 ** ------------------------------------------------------------
 ** $Id: basedir.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#endif

#include        "dir_defaults.h"
#include        "tclCkalloc.h"


static char    *baseDir = NULL;

/*
 * ---------------------------------------------
 * return the current base dir
 * ---------------------------------------------
 */
char           *
getBaseDirectory()
{
	if (baseDir == NULL)
	{
		baseDir = getenv(BASE_DIR_ENVVAR);
		if (baseDir == NULL)
		{
			baseDir = BASE_DIR_DEFAULT;
		}
	}
	return (baseDir);
}




/*
 * ---------------------------------------------
 * set the base dir
 * ---------------------------------------------
 */
int 
setBaseDirectory(char *newdir)
{
	baseDir = ckstrdup(newdir);

	return (baseDir == NULL ? 0 : 1);
}

