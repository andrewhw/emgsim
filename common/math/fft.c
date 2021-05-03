/**
 ** This is the fast fourier transform procedure.  It has been
 ** stolen with permission of the authors from the book,
 ** "Numerical Recipes".  The procedure has three inputs:
 **    a) points : an array of length 2 * nn which
 **            contains the real and complex parts
 **            of the input signal, alternating
 **            between real (2n) and complex (2n+1).
 **            It is of type grid in order that the
 **            array can be passed between the calling
 **            program and the procedure;
 **    b) nn     : The number of data points;
 **    c) isign  : Decides which opertion will be performed
 **            If isign = 1, fft.  If isign = -1,
 **            perform the inverse fft.
 **
 ** The vector points is passed back with the data points in
 ** standard order for most fft's.  (See Numerical Recipes for
 ** a further discussion.)
 **
 **
 ** $Id: fft.c 85 2011-03-29 18:05:57Z andrew $
 **/

#ifndef MAKEDEPEND
#include <math.h>
#endif

#include "error.h"
#include "filtertools.h"
#include "mathtools.h"



OS_EXPORT void 
four1(double *points, int nn, int isign)
{
	int             i, j, m, n;
	int             mmax, M, istep;
	double          wtemp, wr, wpr, wpi, wi, theta;
	double          tempr, tempi;

	n = 2 * nn;
	j = 1;

	/*
	printf("In four1 function n = %d, nn = %d\n", n,nn);
	*/

	/* Bit - reversal section */
	for (i = 1; i <= n; i = i + 2)
	{
		if (j > i)
		{
			tempr = points[j];  /* Exchange the two complex numbers */
			tempi = points[j + 1];
			points[j] = points[i];
			points[j + 1] = points[i + 1];
			points[i] = tempr;
			points[i + 1] = tempi;
		}
		m = n / 2;

		while ((m >= 2) && (j > m))
		{
			j = j - m;
			m = m / 2;
		}

		j = j + m;
	}

	mmax = 2;

	while (n > mmax)
	{
		istep = 2 * mmax;
		theta = 6.28318530717959 / (isign * mmax);      /* Initialize for */
		wpr = -2.0 * SQR(sin(0.5 * theta));     /* trigonometric recurrence */
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;

		/* Here are the two nested inner loops. */
		for (M = 1; M <= mmax; M = M + 2)
		{
			for (i = M; i <= n; i = i + istep)
			{
				j = i + mmax;   /* Danielson - Lanczos formula: */
				tempr = wr * points[j] - wi * points[j + 1];
				tempi = wr * points[j + 1] + wi * points[j];
				points[j] = points[i] - tempr;
				points[j + 1] = points[i + 1] - tempi;
				points[i] = points[i] + tempr;
				points[i + 1] = points[i + 1] + tempi;
			}

			wtemp = wr;         /* Trigonometric recurrence */
			wr = wr * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp * wpi + wi;
		}
		mmax = istep;
	}

	/*
	printf("returning from four1\n");
	*/

}

/*
 *	REALFT calculates the Fourier Transform of a set of 2N
 *	real-valued data points.  Replaces this data (which is stored in
 *	array DATA) by the positive frequency half of its complex
 *	Fourier Transform.  The real valued first and last components of
 *	the complex transform are returned as elements DATA(1) and
 *	DATA(2) respectively.  N must be a power of 2.  This routine
 *	also calculates the inverse transform of a complex data array if
 *	it is the transform of real data.  (Result in this case must be
 *	multiplied by 1/N.
 */
OS_EXPORT void 
realft(double *data, int n, int isign)
{
	double          wr, wi, wpr, wpi, wtemp, theta;
	int             i, i1, i2, i3, i4;
	double          c1, c2, h1r, h1i, h2r, h2i, wrs, wis;

	theta = 6.28318530717959 / (2.0 * n);
	c1 = 0.5;

	if (isign == 1)
	{
		c2 = -0.5;
		four1(data, n, 1);      /* Forward transform */

	} else
	{                           /* Set up reverse transform */
		c2 = 0.5;
		theta = -theta;
	}

	wpr = -2.0 * SQR(sin(0.5 * theta));
	wpi = sin(theta);
	wr = 1.0 + wpr;
	wi = wpi;

	/* Case i = 1 done separately below */
	for (i = 2; i <= (n / 2 + 1); i++)
	{
		i1 = i + i - 1;
		i2 = i1 + 1;
		i3 = n + n + 3 - i2;
		i4 = i3 + 1;
		wrs = wr;
		wis = wi;
		h1r = c1 * (data[i1] + data[i3]);       /* The two separate
												 * transforms are */
		h1i = c1 * (data[i2] - data[i4]);       /* separated out of z.         */
		h2r = -c2 * (data[i2] + data[i4]);
		h2i = c2 * (data[i1] - data[i3]);
		data[i1] = h1r + wrs * h2r - wis * h2i; /* Here they are recombined   */
		data[i2] = h1i + wrs * h2i + wis * h2r; /* to form the true transform */
		data[i3] = h1r - wrs * h2r + wis * h2i; /* of the original real data  */
		data[i4] = -h1i + wrs * h2i + wis * h2r;
		wtemp = wr;             /* The recurrence */
		wr = wr * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	}

	if (isign == 1)
	{
		h1r = data[1];
		data[1] = h1r + data[2];/* Squeeze the first and last data together
								 * to */
		data[2] = h1r - data[2];/* get them all within the original array      */
	} else
	{
		h1r = data[1];
		data[1] = c1 * (h1r + data[2]);
		data[2] = c1 * (h1r - data[2]);

		/* This is the inverse transform for the case ISIGN = -1 */
		four1(data, n, -1);
	}
}


/**
 ** ----------------------------------------------------------------
 **
 ** CONVLV convolves or deconvolves a real data set DATA of length
 ** N (including any user-supplied zero padding) with a response
 ** function RESPNS, stored in wrap around order in a real array
 ** of length M <= N.  (M should be an odd integer.)
 **
 ** Wrap around order means that the first half of the array RESPNS
 ** contains the impulse response function at positive times, while
 ** the second half of the array contains the impulse response at
 ** negative times, counting down from the highest element RESPNS(M).
 ** On input ISIGN is +1 for convolution, -1 for deconvolution.
 ** The answer is returned in the first N components of ANS.
 ** However, ANS must be supplied in the calling program with length
 ** at least 2*N, for consistency with TWOFFT.
 **
 ** N MUST be an integer power of two.
 **/
OS_EXPORT int
convolve(double *fft,
		double *data, int n,
		double *respns, int m,
		int isign,
		double *resultBuffer,
		double deltaT
	)
{
	int             no2, i, ii;
	double          dum, mag2;

	if (m != n)
	{

		/* Put RESPNS in array of length N. */
		for (i = 1; i <= ((m - 1) / 2); i++)
		{
			respns[n + 1 - i] = respns[m + 1 - i];
		}

		/* Pad with zeros. */
		for (i = ((m + 3) / 2); i <= (n - ((m - 1) / 2)); i++)
		{
			respns[i] = 0.0;
		}
	}
	/* FFT both arrays at once. */
	twofft(data, respns, fft, resultBuffer, n);


	no2 = n / 2;

	for (i = 1; i <= (no2 + 1); i++)
	{
		ii = 2 * i;

		/* Multiply FFTs to convolve */
		if (isign == 1)
		{
			dum = resultBuffer[ii - 1];

			resultBuffer[ii - 1] =
				(fft[ii - 1] * resultBuffer[ii - 1] -
				 fft[ii] * resultBuffer[ii]) / no2 * deltaT;

			resultBuffer[ii] =
				(fft[ii] * dum +
				 fft[ii - 1] * resultBuffer[ii]) / no2 * deltaT;


			/* Divide FFTs to deconvolve */
		} else if (isign == -1)
		{


			dum = resultBuffer[ii - 1];
			mag2 = SQR(resultBuffer[ii - 1])
				+ SQR(resultBuffer[ii]);

			if (mag2 == 0.0)
			{
				Derror(__FILE__, __LINE__,
					   "deconvolving at response zero\n");
			}
			resultBuffer[ii - 1] =
				(fft[ii - 1] * resultBuffer[ii - 1] +
				 fft[ii] * resultBuffer[ii]) / mag2 / no2;
			resultBuffer[ii] =
				((fft[ii] * dum - fft[ii - 1] * resultBuffer[ii])
				 / mag2)
				/ no2;
		} else
		{
			Derror(__FILE__, __LINE__, "No meaning for isign\n");
		}
	}


	/* Pack last element with first for REALFT. */
	resultBuffer[2] = resultBuffer[n + 1];

	/* set DC value to zero ? */
	resultBuffer[1] = 0.0;

	/* Inverse transform back to time domain    */
	realft(resultBuffer, no2, -1);

	return 1;
}

/**
 ** ----------------------------------------------------------------
 ** Given two real input arrays DATA1 and DATA2, each of length N,
 ** TWOFFT calls FOUR1 and returns two complex output arrays, FFT1
 ** and FFT2, each of complex length N (i.e. real length 2*N),
 ** which contain the discrete Fourier transforms of the respective
 ** DATAs.
 **
 ** N MUST be an integer power of 2.
 **/
OS_EXPORT void
twofft(double *data1, double *data2, double *fft1, double *fft2, int n)
{
	int             jj, j, NJ;
	double          H1_real, H1_imag, H2_real, H2_imag;


	/** Pack the two real arrays into one complex array */
	for (j = 1; j <= n; j++)
	{
		jj = j + j;
		fft1[jj - 1] = data1[j];
		fft1[jj] = data2[j];
	}

	/** Transform the complex array */
	four1(fft1, n, 1);

	fft2[1] = fft1[2];
	fft1[2] = 0.0;
	fft2[2] = 0.0;

	for (j = 2; j <= (n / 2) + 1; j++)
	{
		jj = 2 * j;
		NJ = 2 * (n + 2 - j);

		/** Use symmetries to separate the two transforms */
		H1_real = 0.5 * (fft1[jj - 1] + fft1[NJ - 1]);

		H1_imag = 0.5 * (fft1[jj] - fft1[NJ]);
		H2_real = 0.5 * (fft1[jj] + fft1[NJ]);
		H2_imag = -0.5 * (fft1[jj - 1] - fft1[NJ - 1]);


		/** Ship them out in two complex arrays */
		fft1[jj - 1] = H1_real;
		fft1[jj] = H1_imag;
		fft1[NJ - 1] = H1_real;
		fft1[NJ] = -H1_imag;
		fft2[jj - 1] = H2_real;
		fft2[jj] = H2_imag;
		fft2[NJ - 1] = H2_real;
		fft2[NJ] = -H2_imag;
	}
}

