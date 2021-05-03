#ifndef	__ARRAY_ALLOCATION_HEADER__
#define	__ARRAY_ALLOCATION_HEADER__

# ifndef    OS_WINDOWS_NT
#	include <unistd.h>
# else
#   include    <io.h>
#endif

#include        "os_defs.h"

#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT void **alloc2darray(
		size_t nElementsX,
		size_t nElementsY,
		size_t elementSize);

OS_EXPORT void ***alloc3darray(
		size_t nElementsX,
		size_t nElementsY,
		size_t nElementsZ,
		size_t elementSize);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif /*	__ARRAY_ALLOCATION_HEADER__ */
