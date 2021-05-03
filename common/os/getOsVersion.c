/** ------------------------------------------------------------
 ** generate a version info based on which OS we are on.  For UNIX,
 ** we use uname to do this -- for windows we use this code gleaned
 ** from the Microsoft Knowledge base
 ** ------------------------------------------------------------
 ** $Id: getOsVersion.c 115 2020-09-25 19:59:26Z andrew $
 **/

#include    "os_defs.h"

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#if defined( OS_BSD ) ||  defined( OS_LINUX ) || defined( OS_DARWIN )
#include       <sys/utsname.h>
#elif defined( OS_WINDOWS_NT )
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

#include    "os_types.h"
#include    "log.h"


#define OS_MAX_VERSION_LEN      256
static char     sVersionBuffer[OS_MAX_VERSION_LEN];
static int      sVersionBufferInitialized = 0;


OS_EXPORT const char *
osGetVersionId(void)
{
	if (sVersionBufferInitialized != 0)
		return sVersionBuffer;

#if defined( OS_IRIX )
	{
		int             status;
		struct utsname  un;

		status = uname(&un);
		if (status < 0)
		{
			sprintf(sVersionBuffer, "SGI/IRIX -- unknown version");
		} else
		{
			sprintf(sVersionBuffer, "SGI/IRIX %s", un.release);
		}
	}


#elif defined( OS_SOLARIS )
	{
		int             status;
		struct utsname  un;

		status = uname(&un);
		if (status < 0)
		{
			sprintf(sVersionBuffer, "SUN/Solaris -- unknown version");
		} else
		{
			sprintf(sVersionBuffer, "SUN/Solaris %s", un.release);
		}
	}


#elif defined( OS_LINUX )
	{
		int             status;
		struct utsname  un;

		status = uname(&un);
		if (status < 0)
		{
			sprintf(sVersionBuffer, "Linux -- unknown version");
		} else
		{
			sprintf(sVersionBuffer, "Linux variant: %s %s",
					un.sysname,
					un.release);
		}
	}

#elif defined( OS_BSD )
	{
		int             status;
		struct utsname  un;

		status = uname(&un);
		if (status < 0)
		{
			sprintf(sVersionBuffer, "BSD -- unknown version");
		} else
		{
			sprintf(sVersionBuffer, "BSD variant: %s %s",
					un.sysname,
					un.release);
		}
	}


#elif defined( OS_DARWIN )
	{
		int             status;
		struct utsname  un;

		status = uname(&un);
		if (status < 0)
		{
			sprintf(sVersionBuffer, "Mac OSX -- unknown version");
		} else
		{
			sprintf(sVersionBuffer, "Mac OSX variant: %s %s",
					un.sysname,
					un.release);
		}
	}

#elif defined( OS_WINDOWS_NT )
	strcpy(sVersionBuffer, "MS Windows");

#else
	strcpy(sVersionBuffer, "Unknown OS");

#endif

	sVersionBufferInitialized = 1;
	return (sVersionBuffer);
}

