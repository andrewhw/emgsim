/** ------------------------------------------------------------
 ** Interrupt/Retry writing to/reading from a FD
 ** ------------------------------------------------------------
 ** $Id: i_rdwr.c 107 2013-06-22 20:22:05Z andrew $
 **/

#include                <os_defs.h>

#ifndef MAKEDEPEND
#include                <errno.h>
#ifndef                OS_WINDOWS_NT
#include              <sys/time.h>
#endif
#include                <sys/types.h>
#ifndef                OS_WINDOWS_NT
#include              <sys/uio.h>
#include              <unistd.h>
#else
#include              <io.h>
#endif
#endif

#include                "msgir.h"
#include                "tclCkalloc.h"
#include                "msgdbids.h"


/*
 * ---------------------------------------------
 * Write out a block -- same as below, but used
 * from the logging system, so there can be no
 * debugging info
 * ---------------------------------------------
 */
int
logWrite(fd, buf, size)
    int             fd;
    const void     *buf;
    int             size;
{
	int             w_stat;

	while (1)
	{
#ifndef OS_WINDOWS_NT
		w_stat = write(fd, buf, size);
#else
		w_stat = _write(fd, buf, size);
#endif

		if (w_stat < 0)
		{
			if (errno != EINTR)
			{
				return (w_stat);
			}
		} else
		{
			return (w_stat);
		}
	}
}


/*
 * ---------------------------------------------
 * Write out a block
 * ---------------------------------------------
 */
int
irWrite(fd, buf, size)
    int             fd;
    const void     *buf;
    int             size;
{
	int             writeStatus = (-1);

	while (writeStatus < 0)
	{
#ifndef OS_WINDOWS_NT
		writeStatus = write(fd, buf, size);
#else
		writeStatus = _write(fd, buf, size);
#endif
		if (writeStatus < 0)
		{
			if (errno != EINTR)
			{
				return (writeStatus);
			}
		}
	}

	return (writeStatus);
}





/*
 * ---------------------------------------------
 * Read in a block, re-trying on signal interrupt
 * ---------------------------------------------
 */
int
irRead(fd, buf, size)
    int             fd;
    void           *buf;
    int             size;
{
	int             readStatus = (-1);
	/**
	 ** errnoStatus is here so that we can recover errno to
	 ** what the read set it to, regardless of what happens
	 ** down in debug logger land
	 **
	 ** (unused) errnoStatus removed, Jun 2013
	 **/

	while (readStatus < 0)
	{

#ifndef OS_WINDOWS_NT
		readStatus = read(fd, buf, size);
#else
		readStatus = _read(fd, buf, size);
#endif
		if (readStatus < 0)
		{
			if (errno != EINTR)
			{
				return (readStatus);
			}
		}
	}

	return (readStatus);
}




/*
 * ---------------------------------------------
 * Syncronize streams -- read n bytes and toss
 * ---------------------------------------------
 */
int
irNullread(fd, size)
    int             fd;
    int             size;
{
	int             r_stat;
	char           *nullbuf;


	/* we will deal with this failing as we go */
	nullbuf = (char *) ckalloc(size);


	if (nullbuf != NULL)
	{
		r_stat = irRead(fd, nullbuf, size);
		ckfree(nullbuf);
		return (r_stat);

	} else
	{
		int             i;
		char            buf;

		for (i = 0; i < size; i++)
		{
			r_stat = irRead(fd, &buf, 1);
			if (r_stat != 1)
				return (r_stat);
		}
		return (size);
	}
	/* NOTREACHED               */
}

