/** ------------------------------------------------------------
 ** directory handling tools
 ** ------------------------------------------------------------
 ** $Id: dir_defaults.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         DIR_DEFAULT_HEADER__
#define         DIR_DEFAULT_HEADER__

#ifndef MAKEDEPEND
#include        <stdio.h>
#endif

/**
 ** MACRO DEFINITIONS
 **/

#define         BASE_DIR_DEFAULT        "."
#define         BASE_DIR_ENVVAR         "WORKGUESTDIR"


#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** basedir.h **/
char *getBaseDirectory(void);
int	setBaseDirectory(char *newdir);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif

#endif  /* DIR_DEFAULT_HEADER__ */


