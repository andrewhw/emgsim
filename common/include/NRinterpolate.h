/**
 ** Interpolation functions taken from Numerical Recipes
 **
 ** $Id: NRinterpolate.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef __NR_INTERPOLATION_HEADER__
#define __NR_INTERPOLATION_HEADER__

#include        "os_defs.h"
#ifndef MAKEDEPEND
#  include      <stdlib.h>
#  include      <math.h>
#endif

#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

                        /** build up a spline definition */
OS_EXPORT void nrSpline (float X[], float Y[], int n,
                                float yp1, float yp2, float result[]);

                        /**
                         * use the definition built above to get a new
                         * y for the given x
                         */
OS_EXPORT float nrSplint (float X[], float Y[], float Y2[],
                                int n, float x);


OS_EXPORT int cubicSplineInterpolation
                (float *resultVector,
                        double *sourceVector,
                        int sourceVectorLength,
                        int expansionFactor,
                        int numControlPoints,
                        float deltaTime,
                        int interpolationIndex);
# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif  /* __NR_INTERPOLATION_HEADER__  */


