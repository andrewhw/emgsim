/** ------------------------------------------------------------
 ** Debug Error routines.
 ** ------------------------------------------------------------
 ** $Id: error.h 13 2008-04-24 23:16:32Z andrew $
 **/

#ifndef         DEBUG_ERROR_HEADER__
#define         DEBUG_ERROR_HEADER__


#include        "os_defs.h"

#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** derror.c **/
OS_EXPORT int Derror(const char *file, int line, const char *fmt, ...);
OS_EXPORT int Error(const char *fmt, ...);
OS_EXPORT int Warning(const char *fmt, ...);

#endif  /* lint */

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* DEBUG_ERROR_HEADER__ */


