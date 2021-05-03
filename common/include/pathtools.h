/** ------------------------------------------------------------
 ** Utilities to handle path creation.
 ** ------------------------------------------------------------
 ** $Id: pathtools.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         WB_PATHTOOLS_HEADER__
#define         WB_PATHTOOLS_HEADER__

#include        <os_defs.h>

#ifndef MAKEDEPEND
# ifndef        OS_WINDOWS_NT
#  include      <unistd.h>
# endif
#endif


typedef struct DirList {
        const char **entry_name;
        int n_entries;
        const char *directory_name;
} DirList;



#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** openpath.c **/
OS_EXPORT int openPath(const char *filename, int flags, int perms);
OS_EXPORT int openDirPath(const char *dirname, int perms);
OS_EXPORT int confirmDirPath(const char *dirname, int perms);

			/** open a file, creating the dir to it if required */
OS_EXPORT FILE *fopenpath(const char *filename, const char *mode);


OS_EXPORT DirList *dirListLoadEntries(const char *path,
		const char *mask);

OS_EXPORT void dirListDelete(DirList *list);
OS_EXPORT int dirToolsGetNextId(const char *path, const char *mask);


OS_EXPORT char *getPathStem(const char *path, ...);

OS_EXPORT char *getExistingFile(const char *stem, ...);
OS_EXPORT char *getUniqueFile(const char *stem);

int opStatBuildPath(const char *basename);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* lint */

#endif  /* WB_PATHTOOLS_HEADER__        */


