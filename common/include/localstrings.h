/** ------------------------------------------------------------
 ** Local string handling tools
 ** ------------------------------------------------------------
 ** $Id: localstrings.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         LOCAL_STRINGS_HEADER__
#define         LOCAL_STRINGS_HEADER__

#include        <os_defs.h>

#ifndef MAKEDEPEND
#ifdef          OS_WINDOWS_NT
# include       <string.h>
#else
# include       <strings.h>
#endif
#endif


# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

/**
 ** this is here for those systems that do not have their
 ** own version of strdup
 **/

#ifdef ultrix
extern  char *strdup(char *);
#endif

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* LOCAL_STRINGS_HEADER__       */


