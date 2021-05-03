/* ------------------------------------------------------------
 * Interrupt/Retry stat'ing of a file
 * ------------------------------------------------------------
 * $Id: i_stat.c 11 2008-04-24 22:13:19Z andrew $
 */

#ifndef MAKEDEPEND
#include        <errno.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#endif

#include        "msgir.h"


/*
 * ---------------------------------------------
 * Stat a file by name
 * ---------------------------------------------
 */
int
irStat(path, buf)
    const char     *path;
    struct stat    *buf;
{
	int             statStatus = (-1);

	while (statStatus < 0)
	{
		statStatus = stat(path, buf);
		if (statStatus < 0)
		{

			if (errno != EINTR)
			{
				return (statStatus);
			}
		}
	}

	return (statStatus);
}




/*
 * ---------------------------------------------
 * Stat a file by descriptor
 * ---------------------------------------------
 */
int
irFstat(fd, buf)
    int             fd;
    struct stat    *buf;
{
	int             statStatus = (-1);

	while (statStatus < 0)
	{
		statStatus = fstat(fd, buf);
		if (statStatus < 0)
		{

			if (errno != EINTR)
			{
				return (statStatus);
			}
		}
	}

	return (statStatus);
}

