/**
 * ------------------------------------------------------------
 * Record/parse command line
 * ------------------------------------------------------------
 * $Id: commandline.h 10 2008-04-24 18:37:51Z andrew $
 */

#ifndef         COMMANDLINE_HEADER__
#define         COMMANDLINE_HEADER__

#include        <os_defs.h>

#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** openpath.c **/
OS_EXPORT int	recordCommandline (
		    const char *directory,
		    int argc,
		    char * const*argv
	    );

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* lint */

#endif  /* COMMANDLINE_HEADER__        */


