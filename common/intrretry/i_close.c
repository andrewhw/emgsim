/** ------------------------------------------------------------
 ** Interrupt/Retry closing of a FD
 ** ------------------------------------------------------------
 ** $Id: i_close.c 11 2008-04-24 22:13:19Z andrew $
 **/

#include        <os_defs.h>

#ifndef MAKEDEPEND
#include        <errno.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#ifndef        OS_WINDOWS_NT
#include      <unistd.h>
#else
#include      <io.h>
#include      <fcntl.h>
#define       close(x)            _close(x)
#endif
#endif

#include        "msgir.h"


/*
 * ---------------------------------------------
 * Close a file
 * ---------------------------------------------
 */
int
irClose(fd)
    int             fd;
{
	int             closeStatus = (-1);

	while (closeStatus < 0)
	{
		closeStatus = close(fd);
		if (closeStatus < 0)
		{

			if (errno != EINTR)
			{
				return (closeStatus);
			}
		}
		/** sync(); sync(); **/
	}

	return (closeStatus);
}

