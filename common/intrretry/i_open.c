/** ------------------------------------------------------------
 ** Interrupt/Retry opening of a FD
 ** ------------------------------------------------------------
 ** $Id: i_open.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include        <os_defs.h>
#include        <os_types.h>

#ifndef MAKEDEPEND
#include        <errno.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#ifndef        OS_WINDOWS_NT
#include      <unistd.h>
#else
#include      <io.h>
#define       open(x,y,z)     _open(x,y,z)
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

#include        "msgir.h"
#include        "msgdbids.h"


/*
 * ---------------------------------------------
 * Open a file
 * ---------------------------------------------
 */
int
irOpen(name, o_flag, filemode)
    const char     *name;
    int             o_flag;
    int             filemode;
{
	int             openStatus = (-1);

	while (openStatus < 0)
	{
		/** sync(); sync(); **/
		openStatus = open(name, o_flag, (mode_t) filemode);
		if (openStatus < 0)
		{

			if (errno != EINTR)
			{
				return (openStatus);
			}
		}
	}
	return (openStatus);
}

