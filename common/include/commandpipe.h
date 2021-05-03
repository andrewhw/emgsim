/**
 * ------------------------------------------------------------
 * Record/parse command line
 * ------------------------------------------------------------
 * $Id: commandpipe.h 10 2008-04-24 18:37:51Z andrew $
 */

#ifndef         COMMANDLINE_HEADER__
#define         COMMANDLINE_HEADER__

#include        <os_defs.h>

typedef struct CommandPipe {
    int ifd_;
    int ofd_;
    pid_t pid_;
} CommandPipe;

#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** commandpipe.c **/
OS_EXPORT int	commandLineToArgv (
		    int *nArgs,
		    char ***argList,
		    char *commandLine
	    );

OS_EXPORT int	execCommandPipe (
		    CommandPipe *cPipe,
		    char *commandLine
	    );

OS_EXPORT int	waitCommandPipe (
		    int *exitStatus,
		    CommandPipe *cPipe
	    );

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* lint */

#endif  /* COMMANDLINE_HEADER__        */


