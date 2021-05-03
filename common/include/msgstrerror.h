/** ------------------------------------------------------------
 ** Platform-independent strerror
 ** ------------------------------------------------------------
 ** $Id: msgstrerror.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         WB_MSG_STRERROR_HEADER__
#define         WB_MSG_STRERROR_HEADER__


#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** msgerror.c **/
OS_EXPORT char *msgstrerror(int errid);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif  /* lint */

#endif  /* WB_MSG_STRERROR_HEADER__     */


