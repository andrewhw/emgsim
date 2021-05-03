/** ------------------------------------------------------------
 ** Tools to perform many handy string functions.
 ** ------------------------------------------------------------
 ** $Id: filetools.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         FILETOOLS_HEADER__
#define         FILETOOLS_HEADER__

#include        <os_defs.h>

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <sys/types.h>
#endif


#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT int copyFile(const char *targetName, const char *srcName);
OS_EXPORT int copyFileContents(int targetfd, int srcfd);
OS_EXPORT int copyFileIfPresent(int targetfd, const char *srcName);

OS_EXPORT char *allocValidFileName(const char *sourceData);


OS_EXPORT char *allocTempFileName(const char *prefix);

/* file dumping tools */
OS_EXPORT int logFloatBuffer(float *data, long size,
				char *filenameFmt, ...);
OS_EXPORT int logLongBuffer(float *data, long size,
				char *filenameFmt, ...);
OS_EXPORT int logShortBuffer(float *data, long size,
				char *filenameFmt, ...);

OS_EXPORT void  deleteStringVector(char **values, int n);
OS_EXPORT int fileParseLine(FILE *fp, int *nValues, char ***values);
OS_EXPORT int parseDblFromStringVector(int nValues, double *floatValue, char **stringValues);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif  /* FILETOOLS_HEADER__   */

