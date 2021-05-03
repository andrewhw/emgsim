/**
 ** Tools used in local mathematical calculations
 **
 ** $Id: mathtools.h 112 2014-04-08 17:48:44Z andrew $
 **/

#ifndef __LOCAL_MATH_TOOLS__
#define __LOCAL_MATH_TOOLS__

#include "os_defs.h"

#if defined( OS_WINDOWS_NT )
#include <float.h>
#define	isnan(x)	_isnan(x)
#define	finite(x)	_finite(x)
#define	isfinite(x)	_finite(x)
#define	isinf(x)	(!_finite(x))

#define	IS_FINITE(x)	_finite(x)
#define	IS_NAN(x)		_isnan(x)

#elif defined( OS_DARWIN )
#include <math.h>
#include <limits.h>
#include <float.h>

#define	IS_FINITE(x)	isfinite(x)
#define	IS_NAN(x)		isnan(x)

#elif defined( OS_BSD )
#include <math.h>
#include <limits.h>
#include <float.h>

#define	IS_FINITE(x)	isfinite(x)
#define	IS_NAN(x)		isnan(x)

#elif defined( OS_CYGWIN )
#include <math.h>
#include <limits.h>
#include <float.h>

#define	IS_FINITE(x)	finite(x)
#define	IS_NAN(x)		isnan(x)

#else
#include <math.h>
#include <values.h>

#define	IS_FINITE(x)	finite(x)
#define	IS_NAN(x)		isnan(x)

#endif


#define	SQR(x)		((x) * (x))
#define	CUBE(x)		((x) * (x) * (x))
#define	MIN(x,y)	(((x) < (y)) ? (x) : (y))
#define	MAX(x,y)	(((x) > (y)) ? (x) : (y))

#define	CARTESIAN_DISTANCE(x0,y0, x1,y1)	\
			sqrt( \
				  ((x0 - x1)*(x0 - x1)) \
				+ ((y0 - y1)*(y0 - y1)) \
				)

#define	CARTESIAN_X_FROM_POLAR(r, theta)	(r * cos(theta))
#define	CARTESIAN_Y_FROM_POLAR(r, theta)	(r * sin(theta))

#define	POLAR_THETA_FROM_CARTESIAN(x,y)		atan2(y, x)
#define	POLAR_R_FROM_CARTESIAN(x, y)		\
				CARTESIAN_DISTANCE(0, 0, x, y)



# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT long factorial(long arg);

OS_EXPORT double calculateChordArea(
		double theta,
		double radius
	);

OS_EXPORT double calculateAngleOfSubsectionByTwoCircles(
		double xCenterMain, double yCenterMain,
		double radiusMain,
		double xCenterSubsector, double yCenterSubsector,
		double radiusSubsector
	);

OS_EXPORT double calculateAngleOfSubsectionByRadiiForTwoCircles(
		double separation,
		double radiusMain,
		double radiusSubsector
	);

OS_EXPORT double calculateAreaOfIntersectionOfTwoCircles(
		double xCenterMain, double yCenterMain,
		double radiusMain,
		double xCenterSubsector, double yCenterSubsector,
		double radiusSubsector
	);

OS_EXPORT double calculateAreaOfIntersectionByRadiiOfTwoCircles(
		double separation,
		double radiusA,
		double radiusB
	);

OS_EXPORT double calculateNormalizedGaussian(
		double mean, double sigma, double x
	);

OS_EXPORT double calculateScaledGaussian(
		double mean, double sigma, double x
	);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif


#endif


