/** ------------------------------------------------------------
 ** Calculate a factorial.  Is this really not in the library?
 ** ------------------------------------------------------------
 ** $Id: functions.c 17 2008-07-03 17:24:49Z andrew $
 **/

#ifndef MAKEDEPEND
#include       <stdio.h>
#include       <math.h>
#endif

#include        "os_defs.h"
#include        "mathtools.h"
#include        "massert.h"

/**
 * Calculate a y-value from a Normalized Gaussian curve
 */
OS_EXPORT double
calculateNormalizedGaussian(
		double mu,
		double sigma,
		double x
	)
{
    return exp(-SQR(x - mu) / (2.0 * SQR(sigma)));
}

/**
 * Calculate a y-value from a scaled Gaussian curve
 */
OS_EXPORT double
calculateScaledGaussian(
		double mu,
		double sigma,
		double x
	)
{
    return (1.0 / (sigma * sqrt(M_2_PI))) *
    calculateNormalizedGaussian(mu, sigma, x);
}

