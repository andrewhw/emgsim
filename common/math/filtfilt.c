
#include "os_defs.h"

#include "tclCkalloc.h"
#include "mathtools.h"
#include "filtertools.h"

#include "massert.h"
#include "log.h"

/*
 * ----------------------------------------------------------------
 *
 * Zero-phase forward and reverse digital filtering.
 *
 *   Y = FILTFILT(B, A, X) filters the data in vector X with the
 *   filter described by vectors A and B to create the filtered
 *   data Y.  The filter is described by the difference equation:
 *
 *     y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *                      - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
 *
 *
 *   After filtering in the forward direction, the filtered sequence
 *   is then reversed and run back through the filter; Y is the time
 *   reverse of the output of the second filtering operation.  The
 *   result has precisely zero phase distortion and magnitude modified
 *   by the square of the filter's magnitude response.  Care is taken
 *   to minimize startup and ending transients by matching initial
 *   conditions.
 *
 *   The length of the input x must be more than three times
 *   the filter order, defined as max(length(b)-1,length(a)-1).
 *
 *   Note that FILTFILT should not be used with differentiator and
 *   Hilbert FIR filters, since the operation of these filters depends
 *   heavily on their phase response.
 */


struct ab_data
{
	int key_;
	char *tag_;
	double sampleRate_;
	int nB_;
	double *b_;

	int nA_;
	double *a_;
};

/*
 *	Converting from Hz (cycles) to radians/sample, which is what
 *	Matlab is going to need for the butter(order, W) function:
 *
 *	f_{radians per sample}
 *		= f_{cycles per second} * seconds/sample * radians/cycle
 *
 *		= f_{cycles per second} * \frac{2 \pi}{sampling rate}
 *
 *	(given on http://dsp.stackexchange.com/questions/7905/converting-frequency-from-hz-to-radians-per-sample)
 *
 *
 *	But in reality, it seems that we use:
 *		(2 * cutoff frequency) / (sampling rate)
 *
 *	Eg: http://www.mathworks.com/help/signal/ref/butter.html
 *
 *		0.6 = (2 * 300) / (1000)
 *
 *	below:
 *		0.064 = (2 * 1000) / (31250)
 *		0.64  = (2 * 10000) / (31250)
 */

/*
 *      Matlab setup with 1/0.032 samples/second:
 *
 *      Sampling rate   = 31250
 *      1/2 SR          = 15625
 *               1000 / 15625 = .06400
 *              10000 / 15625 = .64
 *
 *      W = [ 0.064, 0.64 ]
 *      [ B, A ] = butter(4, W)
 */

/* -------- 1000Hz -> 10KHz -------- */
static double   sFltA_B_8_31250_1000_10000Hz_[] = {
    0.1468,
    0,
    -0.5873,
    0,
    0.8810,
    0,
    -0.5873,
    0,
    0.1468
};

static double   sFltA_A_8_31250_1000_10000Hz_[] = {
    1.0,
    -2.4717,
    1.9421,
    -0.8282,
    0.9572,
    -0.6941,
    0.0504,
    0.0246,
    0.0247
};

static struct ab_data sFltAB_values_8_31250_1000_10000Hz_ = {
    FILTAB_O8_31250_1000_10000HZ,
    "Order 8, SR 31250, Bandpass 1000 -> 10000Hz",
    31250.0,
    9,
    sFltA_B_8_31250_1000_10000Hz_,
    9,
    sFltA_A_8_31250_1000_10000Hz_
};


/*
 *      Matlab setup with 1/0.032 samples/second:
 *
 *      Sampling rate   = 31250
 *      1/2 SR          = 15625
 *                500 / 15625 = .03200
 *              10000 / 15625 = .64
 *
 *      W = [ 0.032, 0.64 ]
 *      [ B, A ] = butter(4, W)
 */

/* -------- 500Hz -> 10KHz -------- */
static double   sFltA_B_8_31250_500_10000Hz_[] = {
    0.1744,
    0,
    -0.6976,
    0,
    1.0464,
    0,
    -0.6976,
    0,
    0.1744
};

static double   sFltA_A_8_31250_500_10000Hz_[] = {
    1.0,
    -2.6917,
    2.1957,
    -0.7233,
    0.8740,
    -0.7684,
    0.0207,
    0.0609,
    0.0323
};

static struct ab_data sFltAB_values_8_31250_500_10000Hz_ = {
    FILTAB_O8_31250_500_10000HZ,
    "Order 8, SR 31250, Bandpass 500 -> 10000Hz",
    31250.0,
    9,
    sFltA_B_8_31250_500_10000Hz_,
    9,
    sFltA_A_8_31250_500_10000Hz_
};




/*
 *      Matlab setup with 1/0.032 samples/second:
 *
 *      Sampling rate   = 31250
 *      1/2 SR          = 15625
 *                100 / 15625 = .0064
 *              10000 / 15625 = .64
 *
 *      W = [ 0.0064, 0.64 ]
 *      [ B, A ] = butter(4, W)
 */

/* -------- 100Hz -> 10KHz -------- */
static double   sFltA_B_8_31250_100_10000Hz_[] = {
    0.1991,
    0,
    -0.7962,
    0,
    1.1944,
    0,
    -0.7962,
    0,
    0.1991
};

static double   sFltA_A_8_31250_100_10000Hz_[] = {
    1.0,
    -2.8619,
    2.4229,
    -0.6354,
    0.7720,
    -0.8404,
    0.0043,
    0.0977,
    0.0409
};

static struct ab_data sFltAB_values_8_31250_100_10000Hz_ = {
    FILTAB_O8_31250_100_10000HZ,
    "Order 8, SR 31250, Bandpass 100 -> 10000Hz",
    31250.0,
    9,
    sFltA_B_8_31250_100_10000Hz_,
    9,
    sFltA_A_8_31250_100_10000Hz_
};



/*
 *      Matlab setup with 1/0.032 samples/second:
 *
 *      Sampling rate   = 31250
 *      1/2 SR          = 15625
 *                 50 / 15625 = .003200
 *              10000 / 15625 = .64
 *
 *      W = [ 0.0032, 0.64 ]
 *      [ B, A ] = butter(4, W)
 */

/* -------- 50Hz -> 10KHz -------- */
static double   sFltA_B_8_31250_50_10000Hz_[] = {
    0.2023,
    0,
    -0.8093,
    0,
    1.2139,
    0,
    -0.8093,
    0,
    0.2023
};

static double   sFltA_A_8_31250_50_10000Hz_[] = {
    1.0,
    -2.8828,
    2.4527,
    -0.6244,
    0.7568,
    -0.8501,
    0.0029,
    0.1029,
    0.0421
};

static struct ab_data sFltAB_values_8_31250_50_10000Hz_ = {
    FILTAB_O8_31250_50_10000HZ,
    "Order 8, SR 31250, Bandpass 50 -> 10000Hz",
    31250.0,
    9,
    sFltA_B_8_31250_50_10000Hz_,
    9,
    sFltA_A_8_31250_50_10000Hz_
};


/*
 *      Matlab setup with 1/0.032 samples/second:
 *
 *      Sampling rate   = 31250
 *      1/2 SR          = 15625
 *                 10 / 15625 = .00064
 *              10000 / 15625 = .64
 *
 *      W = [ 0.00064, 0.64 ]
 *      [ B, A ] = butter(4, W)
 */

/* -------- 10Hz -> 10KHz -------- */
static double   sFltA_B_8_31250_10_10000Hz_[] = {
    0.20495200066925645110,
    0,
    -0.81980800267702580442,
    0,
    1.22971200401553870662,
    0,
    -0.81980800267702580442,
    0,
    0.20495200066925645110
};

static double   sFltA_A_8_31250_10_10000Hz_[] = {
    1.0,
    -2.89947768325627874830,
    2.47683867392381529271,
    -0.61562506119845428199,
    0.74416145860108406929,
    -0.85804022886036301898,
    0.00189938368797690216,
    0.10711320004273780504,
    0.04313025711321025885
};


static struct ab_data sFltAB_values_8_31250_10_10000Hz_ = {
    FILTAB_O8_31250_10_10000HZ,
    "Order 8, SR 31250, Bandpass 10 -> 10000Hz",
    31250.0,
    9,
    sFltA_B_8_31250_10_10000Hz_,
    9,
    sFltA_A_8_31250_10_10000Hz_
};

static struct ab_data *sABvalues[] = {
    &sFltAB_values_8_31250_10_10000Hz_,
    &sFltAB_values_8_31250_50_10000Hz_,
    &sFltAB_values_8_31250_100_10000Hz_,
    &sFltAB_values_8_31250_500_10000Hz_,
    &sFltAB_values_8_31250_1000_10000Hz_,
    NULL
};


/*
 * Return the parameters for a given key
 *
 * Note that the "best A" and "best B" are to overcome problems
 * introduced with the base-10 constants above, which may not
 * convert nicely into doubles.  If the constant sets above
 * do not sum nicely to be 0.0, then we will have a DC factor
 * introduced in the final filter.
 */
OS_EXPORT int 
getABParams(
		double **b, int *nB,
		double **a, int *nA,
		int key,
		double sampleRate
	)
{
	static int      isInitialized = 0;
	int             i;

	/**
	 ** First time in, make sure that the values
	 ** sum to 1.0.
	 **/
	if (!isInitialized)
	{
		long double     accumulator;
		int             j;

		isInitialized = 1;
		for (i = 0; sABvalues[i] != NULL; i++)
		{

			/*
			fprintf(stderr, "Fixing %s\n", sABvalues[i]->tag_);
			*/

			/**
			 * calculate the best-roundoff A
			 *
			 * We know that the values of A from 1->N
			 * should sum to be -1.0 (i.e.; all of A
			 * should sum to be zero), so we calculate
			 * the amount we are out by, and fix the
			 * largest value (A[1]) by the amount
			 * we need to change.
			 */
			accumulator = 0.0;
			for (j = 1; j < sABvalues[i]->nA_; j++)
			{
				accumulator += sABvalues[i]->a_[j];
			}
			accumulator = (-1.0) - accumulator;
			sABvalues[i]->a_[1] += accumulator;


			/** sanity check */
			/*
			accumulator = 0.0;
			for (j = 0; j < sABvalues[i]->nA_; j++) {
				accumulator += sABvalues[i]->a_[j];
			}
			fprintf(stderr, "A Fixed up within %g\n",
						(double) accumulator);
			*/


			/**
			 * calculate the best-roundoff B
			 *
			 * similarly for B, we determine how much we
			 * are out from a perfect zero-sum, and we
			 * adjust the largest (central) value by the
			 * amount which we need.
			 */
			accumulator = 0.0;
			for (j = 0; j < sABvalues[i]->nB_ / 2; j++)
			{
				accumulator +=
					sABvalues[i]->b_[j] +
					sABvalues[i]->b_[sABvalues[i]->nB_ - (j + 1)];
			}

			/** now plug it back into the middle */
			j = (sABvalues[i]->nB_ / 2);
			sABvalues[i]->b_[j] = (-accumulator);


			/** sanity check */
			/*
			accumulator = 0.0;
			for (j = 0; j < sABvalues[i]->nB_; j++) {
				accumulator += sABvalues[i]->b_[j];
			}
			fprintf(stderr, "B Fixed up within %g\n",
						(double) accumulator);
			fprintf(stderr, "\n");
			*/
		}
	}
	for (i = 0; sABvalues[i] != NULL; i++)
	{
		if (sABvalues[i]->key_ == key
			&& sABvalues[i]->sampleRate_ == sampleRate)
		{
			*nB = sABvalues[i]->nB_;
			*b = sABvalues[i]->b_;
			*nA = sABvalues[i]->nA_;
			*a = sABvalues[i]->a_;
			return 1;
		}
	}

	fprintf(stderr, "No filter found\n");
	return 0;
}

#ifdef  UNUSED_DEBUG
static int 
dumpToFile(char *name, double *buffer, int nPoints)
{
	FILE           *fp;
	int             i;

	fp = fopen(name, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open debug file '%s'\n", name);
		return 0;
	}
	fprintf(stderr, "Dumping filter output to file '%s'\n", name);

	for (i = 0; i < nPoints; i++)
	{
		fprintf(fp, "%d %f\n", i, buffer[i]);
	}
	fclose(fp);

	return 1;
}
#endif                          /* UNUSED_DEBUG */

/*
 *   Filters the data in vector X with the filter described
 *   by vectors A and B to create the filtered data Y.  The filter
 *   is described by the difference equation:
 *
 *     y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *                      - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
 */
OS_EXPORT int
filtfilt(
         double *output,
         double *data, int nX,
         double *b, int nB,
         double *a, int nA)
{
	double         *tmpBuf, tmpVal;
	int             i, halfLen;

	tmpBuf = (double *) ckalloc(nX * sizeof(double));

	/* dumpToFile("filtstart", data, nX); */
	if (!filter(tmpBuf, data, nX, b, nB, a, nA))
	{
		return 0;
	}
	/* dumpToFile("filtered_1", tmpBuf, nX); */

	/* reverse the buffer */
	halfLen = (int) (nX / 2);
	for (i = 0; i < halfLen; i++)
	{
		tmpVal = tmpBuf[i];
		tmpBuf[i] = tmpBuf[nX - (i + 1)];
		tmpBuf[nX - (i + 1)] = tmpVal;
	}

	/* dumpToFile("revered_1", tmpBuf, nX); */

	if (!filter(output, tmpBuf, nX, b, nB, a, nA))
	{
		return 0;
	}
	/* dumpToFile("filtered_2", output, nX); */

	/* zero out the nA values at the end of the buffer */
	halfLen = (int) (nX / 2);
	for (i = 0; i < (nA * 2); i++)
	{
		output[i] = output[nX - (i + 1)] = 0;
	}

	/* place the "good" part of the buffer into the output */
	for (i = nA * 2; i < halfLen; i++)
	{
		tmpVal = output[i];
		output[i] = output[nX - (i + 1)];
		output[nX - (i + 1)] = tmpVal;
	}


	/* dumpToFile("revered_2", output, nX); */

	/* get rid of the temporary buffer */
	ckfree(tmpBuf);

	return 1;
}




/*
 *     y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *                      - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
 *
 */
OS_EXPORT int
filter(
		double *y,
		double *x, int nX,
		double *b, int nB,
		double *a, int nA
	)
{
	double          accumulatedNewValue;
	int             iY, iA, iB;

	/*
	 * fill in the first nA values with zero, so that we can
	 * accumulate based on them
	 */
	for (iY = 0; iY < nA; iY++)
	{
		y[iY] = 0.0;
	}

	/*
	 * Start the main loop at nA
	 */
	for (iY = 0; iY < nX; iY++)
	{
		accumulatedNewValue = 0.0;

		/*
		 * here we go back to zero in the data
		 */
		for (iB = 0; (iB < nB) && (iY - iB >= 0); iB++)
		{
			accumulatedNewValue += (b[iB] * x[iY - iB]);
			MSG_ASSERT(!isnan(accumulatedNewValue),
					   "Value became NaN!");

#ifndef OS_WINDOWS_NT
			MSG_ASSERT(!isinf(accumulatedNewValue),
					   "Value became infinite!");
#endif
		}

		/*
		 * in the result we can not go back to zero, as that will be
		 * undefined
		 */
		for (iA = 1; iA < nA && (iY - iA >= 0); iA++)
		{
			accumulatedNewValue -= (a[iA] * y[iY - iA]);
			MSG_ASSERT(!isnan(accumulatedNewValue),
					   "Value became NaN!");
#ifndef OS_WINDOWS_NT
			MSG_ASSERT(!isinf(accumulatedNewValue),
					   "Value became infinite!");
#endif
		}

		/** normalize by our (possibly != 1.0) a */
		y[iY] = accumulatedNewValue / a[0];
	}

	return 1;
}

