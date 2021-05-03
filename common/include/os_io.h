/**
 * Include IO.h for windows
 */
#ifndef __LOCAL_UNIST_HEADER__
#define __LOCAL_UNIST_HEADER__

#include "os_defs.h"

#ifdef OS_WINDOWS_NT
# include <io.h>
#else
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/fcntl.h>
#endif

#endif /* __LOCAL_UNIST_HEADER__ */
