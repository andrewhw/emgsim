/**
 ** Interpolation routines given in Numerical Recipes for C
 **/

#include "NRinterpolate.h"

#include "log.h"
#include "tclCkalloc.h"


/**
 ** NR in C, 2nd ed. pp 115.
 **
 ** Given arrays x[1..n] and y[1..n] containing a tabulated function
 ** i.e., y[i] = f(x[i]), with x[1] < x[2] < x[n], and given values
 ** yp1 and ypn for the first derivative of the interpolating function
 ** at points 1 and n, repectively, this routine returns an array
 ** y2[1..n] that contains the second derivative of the interpolating
 ** function at the tabulated points x[i].  If yp1 and/or ypn are
 ** equal to 1e30 or larger, the routine is signaled to set the
 ** corresponding boundary conditions for a natural spline, with zero
 ** second derivative on that boundary.
 **/
OS_EXPORT void 
nrSpline(
         float x[],
         float y[],
         int n,
         float yp1,
         float ypn,
         float y2[]
	)
{
	int             i, k;
	double          p, qn, sig, un;
	double         *u;

	u = (double *) malloc(sizeof(double) * (n + 1));

	/**
	 ** the lower boundary condition is set either to be "natural",
	 ** or else to have a specified first derivative
	 **/
	if (yp1 > 0.99e30)
		u[1] = y2[1] = 0;
	else
	{
		y2[1] = (-0.5);
		u[1] = (3.0 / (x[2] - x[1]))
			* (((y[2] - y[1]) / (x[2] - x[1])) - yp1);
	}


	/*
	 * This is the decomposition loop of the tridiagonal algorithm: y2 and u
	 * are used for temporary storage of the decomposed factors
	 */
	for (i = 2; i <= n - 1; i++)
	{
		sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
		p = sig * y2[i - 1] + 2.0;
		y2[i] = (float) ((sig - 1.0) / p);
		u[i] = ((y[i + 1] - y[i]) / (x[i + 1] - x[i]))
			- ((y[i] - y[i - 1]) / (x[i] - x[i - 1]));
		u[i] = (((6.0 * u[i]) / (x[i + 1] - x[i - 1])) - (sig * u[i - 1])) / p;
	}


	/*
	 * The upper boundary condition is set either to be "natural" or to have
	 * a specified first derivative
	 */
	if (ypn > 0.99e30)
		qn = un = 0.0;
	else
	{
		qn = 0.5;
		un = (3.0 / (x[n] - x[n - 1]))
			* (ypn - ((y[n] - y[n - 1]) / (x[n] - x[n - 1])));
	}

	y2[n] = (float) ((un - qn * u[n - 1]) / (qn * y2[n - 1] + 1.0));


	/** this is the back-substitution loop of the tridiagonal algorithm */
	for (k = n - 1; k >= 1; k--)
	{
		y2[k] = (float) (y2[k] * y2[k + 1] + u[k]);
	}

	free(u);
}

/**
 ** NR in C, 2nd ed. pp 115.
 **
 ** use the definition built above to get a new y for the given x
 **
 **
 ** Given the arrays xa[1..n] and ya[1..n] which tabulate a function
 ** (with the xa[i]s in order), and given the array y2a[1..n], which
 ** is the output from nrSpline() above, and given a value of x, this
 ** routine returns a cubic-spline interpolated value y.
 */
OS_EXPORT float 
nrSplint(
         float xa[],
         float ya[],
         float y2a[],
         int n,
         float x
	)
{
	int             klo, khi, k;
	float           h, b, a;

	/** sanity check */
	if (x < xa[1] || x > xa[n])
	{
		LogErr("nrSplint : out of range\n");
		return 0.0;
	}
	/**
	 ** We will find the right place in the table by means of bisection.
	 ** This is optimal if sequential calls to this function are at random
	 ** values of x.  If sequential calls are in order, and closely spaced,
	 ** one would do better to store the previous values of klo and khi
	 ** and test if they remain appropriate on the next call.
	 **/
	klo = 1;
	khi = n;
	while (khi - klo > 1)
	{
		k = (khi + klo) >> 1;
		if (xa[k] > x)
			khi = k;
		else
			klo = k;
	}
	/** klo and khi now bracket the input value of x **/

	h = xa[khi] - xa[klo];

	/** all xa must be distinct */
	if (h == 0.0)
	{
		LogErr("nrSplint : bad input\n");
		return 0.0;
	}
	a = (xa[khi] - x) / h;
	b = (x - xa[klo]) / h;

	/** cubic spline polynomial is now evaluated */
	return (float) (a * ya[klo] + b * ya[khi]
					+ (((a * a * a) - a) * y2a[klo]
					   + ((b * b * b) - b) * y2a[khi])
					* ((h * h) / 6.0));
}

/**
 *      Interpolate from one vector to another,
 *      given all the relevant values, using
 *      the above functions
 */
int 
cubicSplineInterpolation(
		float *resultVector,
		double *sourceVector,
		int sourceVectorLength,
		int expansionFactor,
		int numControlPoints,
		float deltaTime,
		int index
	)
{
	float          *times;
	float          *workingData;
	float          *splineControlPoints;
	int             j;


	/** BEGIN INTERPOLATION SETUP **/
	{
		float           slopeAtStart, slopeAtEnd;


		times = ckalloc(sizeof(float) * (2 * numControlPoints + 1));
		workingData = ckalloc(sizeof(float) * (2 * numControlPoints + 1));
		splineControlPoints
			= ckalloc(sizeof(float) * (2 * numControlPoints + 1));


		/**
		 * Generate interpolator control points in the
		 * vicinity of the area we care about
		 */
		for (j = 1; j <= 2 * numControlPoints; j++)
		{
			times[j] = (float) ((index + j - numControlPoints)
								* deltaTime);
			if (((index + j - numControlPoints) >= 0)
				|| ((index + j - numControlPoints)
					< sourceVectorLength))
				workingData[j] = (float) sourceVector[
												index + j - numControlPoints
					];
			else
				workingData[j] = (float) 0.0;
		}


		/** calculate the slopes between the end-most points */
		slopeAtStart = workingData[2] - workingData[1];
		slopeAtEnd = workingData[numControlPoints * 2]
			- workingData[(numControlPoints * 2) - 1];

		/**
		 * calculate the spline control points,
		 * place in splineControlPoints
		 */
		nrSpline(times, workingData, 2 * numControlPoints,
				 slopeAtStart, slopeAtEnd,
				 splineControlPoints);
	}
	/** INTERPOLATION SETUP COMPLETE **/



	/** BEGIN INTERPOLATION **/
	{
		/**
		 * Now proceed to evaluate the function at times near the
		 * first peak approximation.
		 */
		int             targetIndex;
		float           startTime = (index - 1) * deltaTime;
		float           endTime = (index + 1) * deltaTime;
		double          step = deltaTime / expansionFactor;
		double          evalTime;


		/** plug in the first point, and verify with the source data */
		targetIndex = (index - 1) * expansionFactor;
		resultVector[targetIndex] = nrSplint(times, workingData,
											 splineControlPoints,
											 2 * numControlPoints,
											 startTime);

		/** add in all the actually interpolated points */
		for (evalTime = startTime + step;
			 evalTime <= endTime;
			 evalTime += step)
		{
			targetIndex++;
			resultVector[targetIndex] = nrSplint(times, workingData,
												 splineControlPoints,
												 2 * numControlPoints,
												 (float) evalTime);

		}
	}
	/** END INTERPOLATION **/


	/** delete arrays used in interpolation */
	ckfree(splineControlPoints);
	ckfree(workingData);
	ckfree(times);

	return 1;
}

