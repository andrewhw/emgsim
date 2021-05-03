/** ------------------------------------------------------------
 ** Emergency strerror, if there isn't one on the local system
 ** ------------------------------------------------------------
 ** $Id: strerror.c 33 2009-04-26 13:48:18Z andrew $
 **/

#ifndef MAKEDEPEND
#include                <stdio.h>
#include                <errno.h>
#include                <string.h>
#endif

#include "os_defs.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#ifndef         __STDC__
#define         LOCAL_STRERROR
#endif

#ifdef          LOCAL_STRERROR

/** build a strerror function **/

#define                         MAX_ERROR               75
#define                         MIN_ERROR               0


/*
 * System error message -- it is a pain that a lot
 * of systems won't let us just get this list
 */

static char    *err_mesg[] = {
    " (l-err) No error ",
    " (l-err) Not owner ",
    " (l-err) No such file or directory ",
    " (l-err) No such process ",
    " (l-err) Interrupted system call ",
    " (l-err) I/O error ",
    " (l-err) No such device or address ",
    " (l-err) Arg list too long ",
    " (l-err) Exec format error ",
    " (l-err) Bad file number ",
    " (l-err) No children ",
    " (l-err) No more processes ",
    " (l-err) Not enough core ",
    " (l-err) Permission denied ",
    " (l-err) Bad address ",
    " (l-err) Block device required ",
    " (l-err) Mount device busy ",
    " (l-err) File exists ",
    " (l-err) Cross-device link ",
    " (l-err) No such device ",
    " (l-err) Not a directory",
    " (l-err) Is a directory ",
    " (l-err) Invalid argument ",
    " (l-err) File table overflow ",
    " (l-err) Too many open files ",
    " (l-err) Not a typewriter ",
    " (l-err) Text file busy ",
    " (l-err) File too large ",
    " (l-err) No space left on device ",
    " (l-err) Illegal seek ",
    " (l-err) Read-only file system ",
    " (l-err) Too many links ",
    " (l-err) Broken pipe ",
    " (l-err) Argument too large ",
    " (l-err) Result too large ",
    " (l-err) Operation would block ",
    " (l-err) Operation now in progress ",
    " (l-err) Operation already in progress ",
    " (l-err) Socket operation on non-socket ",
    " (l-err) Destination address required ",
    " (l-err) Message too long ",
    " (l-err) Protocol wrong type for socket ",
    " (l-err) Protocol not available ",
    " (l-err) Protocol not supported ",
    " (l-err) Socket type not supported ",
    " (l-err) Operation not supported on socket ",
    " (l-err) Protocol family not supported ",
    " (l-err) Address family not supported by protocol family ",
    " (l-err) Address already in use ",
    " (l-err) Can't assign requested address ",
    " (l-err) Network is down ",
    " (l-err) Network is unreachable ",
    " (l-err) Network dropped connection on reset ",
    " (l-err) Software caused connection abort ",
    " (l-err) Connection reset by peer ",
    " (l-err) No buffer space available ",
    " (l-err) Socket is already connected ",
    " (l-err) Socket is not connected ",
    " (l-err) Can't send after socket shutdown ",
    " (l-err) Too many references: can't splice ",
    " (l-err) Connection timed out ",
    " (l-err) Connection refused ",
    " (l-err) Too many levels of symbolic links ",
    " (l-err) File name too long ",
    " (l-err) Host is down ",
    " (l-err) No route to host ",
    " (l-err) Directory not empty ",
    " (l-err) Too many processes ",
    " (l-err) Too many users ",
    " (l-err) Disc quota exceeded ",
    " (l-err) NFS error ESTALE ",
    " (l-err) NFS error EREMOTE ",
    " (l-err) No message of desired type ",
    " (l-err) Identifier removed ",
    " (l-err) alignment error ",
    " (l-err) LOCK_MAX exceeded             "
};


/*
 * ---------------------------------------------
 * duplicate the ANSI strerror() function
 * ---------------------------------------------
 */
OS_EXPORT char *
msgstrerror(int err)
{
	char           *retptr;
	static char     ebuf[80];

	if (err > MAX_ERROR || err < MIN_ERROR)
	{
		sprintf(ebuf, "Out of Bounds Error %d", err);
		retptr = ebuf;
	} else
	{
		retptr = err_mesg[err];
	}
	return (retptr);
}

#else

/*
 * ---------------------------------------------
 * duplicate the ANSI strerror() function
 * ---------------------------------------------
 */
OS_EXPORT char *
msgstrerror(int err)
{
	return (strerror(err));
}


#endif

