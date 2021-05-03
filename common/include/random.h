/**
 ** Forward declarations for the various random number series
 ** generation functions.
 **
 ** $Id: random.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef __RANDOMS_HEADER__
#define __RANDOMS_HEADER__

#include        "os_defs.h"
#ifndef MAKEDEPEND
#  include      <stdlib.h>
#  include      <math.h>
#endif

#ifndef MAKEDEPEND
# if defined( OS_IRIX )
#       warning Unconfirmed RAND_MAX
#       define          OS_RAND_MAX             RAND_MAX

# elif defined( OS_SOLARIS )
#       define          OS_RAND_MAX             LONG_MAX

# elif defined( OS_HPUX )
#       warning Unconfirmed RAND_MAX
#       define          OS_RAND_MAX             RAND_MAX

# elif defined( OS_AIX )
#       warning Unconfirmed RAND_MAX
#       define          OS_RAND_MAX             RAND_MAX

# elif defined( OS_BSD )
#       define          OS_RAND_MAX             RAND_MAX

# elif defined( OS_LINUX )
#       define          OS_RAND_MAX             RAND_MAX

# elif defined( OS_DARWIN )
#       define          OS_RAND_MAX             RAND_MAX

# elif defined( OS_WINDOWS_NT )
#       define          OS_RAND_MAX             LONG_MAX

# endif
#endif

#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT void seedLocalRandom(int);
OS_EXPORT void dumpRandom();

OS_EXPORT int localRandom();
OS_EXPORT double localRandomDouble();
OS_EXPORT int intRangeRandom(int range);
OS_EXPORT float floatNormalizedRandom();
OS_EXPORT float floatSignedRangeRandom(float range);

OS_EXPORT double pgauss(double zdev, double *prob, double *dens);
OS_EXPORT double zgauss(double prob);
OS_EXPORT double gauss01();

OS_EXPORT double poisson(double mean);

OS_EXPORT float averageInt(int nValues, int *values);
OS_EXPORT float averageInt(int nValues, int *values);
OS_EXPORT float sigmaInt(int nValues, int *values, float mu);

OS_EXPORT double nr_ran2(long *idum);

extern int gDumpRandom;
 

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif  /* __RANDOMS_HEADER__   */


