/**
 ** ----------------------------------------------------------------
 ** I/O utils for reading/writing files with graceful
 ** error reporting and recovery.
 ** ----------------------------------------------------------------
 **
 ** $Id: io_utils.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef __IO_UTILS_HEADER__
#define __IO_UTILS_HEADER__

#include "os_defs.h"
#include "os_types.h"


typedef struct FP {
        FILE    *fp;
        char    *name;
} FP;

#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT FP *openFP(const char *name, const char *mode);
OS_EXPORT void closeFP(FP *fp);
OS_EXPORT int fileExists(const char *name);
OS_EXPORT int getFileLength(const char *name);
OS_EXPORT int getFDFileLength(int fd);

/** read in a value of the appropriate type */
OS_EXPORT int r2byteInt(FP *fp, osInt16 *value);
OS_EXPORT int r4byteInt(FP *fp, osInt32 *value);
OS_EXPORT int rFloat(FP *fp, float *value);
OS_EXPORT int rGeneric(FP *fp, void *value, int len);


/** write out values of the appropriate type */
OS_EXPORT int w2byteInt(FP *fp, osInt16 value);
OS_EXPORT int w4byteInt(FP *fp, osInt32 value);
OS_EXPORT int wFloat(FP *fp, float value);

OS_EXPORT int wGeneric(FP *fp, void *value, int len);


# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif


