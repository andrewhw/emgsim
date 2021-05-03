/** ------------------------------------------------------------
 ** Interrupt/Retry routines.
 ** ------------------------------------------------------------
 ** $Id: msgir.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         INTERRUPT_HANDLING_IOHEADER__
#define         INTERRUPT_HANDLING_IOHEADER__

#include        "os_defs.h"
#include        "os_types.h"

#ifndef MAKEDEPEND
# include       <sys/types.h>
# ifndef        OS_WINDOWS_NT
#  include      <sys/time.h>
# endif
# include       <sys/stat.h>
/*
# ifndef    OS_WINDOWS_NT
#  include      <winsock2.h>
# else
#  include      <sys/socket.h>
*/
# ifndef    OS_WINDOWS_NT
#  include      <unistd.h>
#  include      <dirent.h>
# endif
#endif


#ifndef         lint


# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

#ifdef  UNUSED
    /** m_accept.c **/
int irAccept(int s, struct sockaddr *addr, int *addrlen);

    /** m_connect.c **/
int irConnect(int s, struct sockaddr *name, int namelen);
#endif /* UNUSED */

    /** m_close.c **/
int irClose(int fd);

    /** m_open.c **/
int irOpen(const char *name, int o_flag, int mode);

#ifndef OS_WINDOWS_NT
    /** m_opendir.c **/
DIR *irOpendir(const char *name);
#endif /* OS_WINDOWS_NT */

    /** m_rdwr.c **/
int logWrite(int fd, const void *buf, int size);
int irWrite(int fd, const void *buf, int size);
int irRead(int fd, void *buf, int size);
int irNullRead(int fd, int size);

#ifdef  UNUSED
    /** m_select.c **/
int irSelect(int nfds,
		fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		struct timeval *time);
#endif /* UNUSED */

    /** m_stat.c **/
int irStat(const char *path, struct stat *buf);
int irFstat(int fd, struct stat *buf);


    /** m_unlink.c **/
int irUnlink(const char *name);

#ifdef  UNUSED
    /** m_wait.c **/
int irWaitpid(pid_t pid, int *stat_loc, int options);
#endif /* UNUSED */


# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* lint */

#endif  /* INTERRUPT_HANDLING_IOHEADER__        */

