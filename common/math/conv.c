/**
 ** ----------------------------------------------------------------
 ** Given two different input arrays DATA1 and DATA2,
 ** CONVOLUTION calculates the discrete convolution of these two
 ** arrays, using direct method
 ** These arrays have different lengths, N and M, the convolution result is
 ** an array with the length of N + M - 1.
 **
 ** $Id: conv.c 11 2008-04-24 22:13:19Z andrew $
 **/

#include "tclCkalloc.h"
#include "mathtools.h"

OS_EXPORT int
directConvolve(
		double *wfn,
		int wfnSize,
		double *cur,
		int curSize,
		double *conv_res,
		float z_inc
	)
{
	int             i, ii, k, j;
	int             convSize = wfnSize + curSize - 1;
	double          sum, A, B, C;


	double         *ext_wfn = (double *) ckalloc(convSize * (sizeof(double)));
	double         *ext_cur = (double *) ckalloc(convSize * (sizeof(double)));
	double         *result = (double *) ckalloc(convSize * (sizeof(double)));

	for (i = 0; i < convSize; i++)
	{
		if (i < wfnSize)
			ext_wfn[i] = (double) wfn[i];
		else
			ext_wfn[i] = (double) 0.0;
	}

	/* plot_mah(ext_wfn,convSize); */


	for (i = 0; i < convSize; i++)
	{
		if (i < curSize)
			ext_cur[i] = (double) cur[i];
		else
			ext_cur[i] = (double) 0.0;
	}
	/*
	 * plot_mah(ext_cur,convSize); (mA)
	 */

	for (ii = 0; ii < convSize; ii++)
	{
		sum = 0.0;
		for (k = 0; k <= ii; k++)
		{
			B = ext_cur[k];
			A = ext_wfn[ii - k];
			C = A * B * z_inc;
			sum = sum + C;
		}


		result[ii] = (double) sum;
	}


	for (j = 0; j < convSize; j++)
		conv_res[j] = (double) result[j];

	/*
	 * plot_mah(conv_res,convSize); (mv)
	 */

	ckfree(ext_cur);
	ckfree(ext_wfn);
	ckfree(result);


	return 1;
}


/** ----------------------------------------------------------------
 ** This function adjusts the convolution artifacts, using the algorithm
 ** mentioned in "A Model of EMG Generation", IEEE TRANSACTIONS ON BIOMEDICAL
 ** ENGINEERING, vol.47, Feb.2000
 ** The left and right affixes stand for the data related to the left part of
 ** the endplate and to the right part of the endplate respectively.
 ** They should be the same length (convSize).
 **/
OS_EXPORT int 
adjust_elimi(
		int NI,
		int convSize,
		int NT_Right,
		int NT_Left,
		float d_z,
		float diameterInMM,
		double *data_left,
		double *data_right,
		double *current,
		double *wfn_left, double *wfn_right
	)
{
	int             i, j;
	double          tendonRight, TendonRightoffset;
	double          tendonLeft, TendonLeftoffset;
	double          EndplateOffset;
	double          tendon_endplate;
	double          CurrentIntegralConst;
	double         *currentExpand = NULL;
	double         *integ_current = NULL;
	const double    sigmai = 0.00101;
	float           z;

	tendonRight = wfn_right[NT_Right - 1];
	tendonLeft = wfn_left[NT_Left - 1];

	/*
	 * Expanding the Buffers with zero
	 */
	currentExpand = (double *) ckalloc(convSize * (sizeof(double)));

	for (i = 0; i < convSize; i++)
	{
		if (i < NI)
			currentExpand[i] = current[i];
		else
			currentExpand[i] = (double) 0.0;
	}

	integ_current = (double *) ckalloc(convSize * (sizeof(double)));


	/*
	 * From Nandedkar's paper
	 *
	 * 768z^3 e^{-2z} - 90
	 *
	 * which comes from 2z into Rosenfalk's original.
	 */
	CurrentIntegralConst = ((3072 * sigmai * M_PI) / 4.0) * SQR(diameterInMM);

	for (j = 0; j < NI; j++)
	{
		z = j * d_z;

		integ_current[j] = (double)
			(CurrentIntegralConst * SQR(z) * (0.75 - 0.5 * z) *
			 exp(-2.0 * z));
	}


	/*
	 * for (i=0 ; i < convSize; i++) { sum = 0.0 ; for (j=0 ; j < i+1 ; j++)
	 * { sum = (double) (sum + current[j] * d_z) ; } integ_current[i] =
	 * (double) sum; }
	 */

	TendonLeftoffset = tendonLeft * integ_current[NI - 1];
	TendonRightoffset = tendonRight * integ_current[NI - 1];

	/*
	 * The Tendon effect
	 */
	for (i = 0; i < NI; i++)
	{
		/*
		 * The left Tendon
		 */
		tendon_endplate = (double) tendonLeft *integ_current[i];
		tendon_endplate = (double) (tendon_endplate + TendonLeftoffset);
		data_left[i + NT_Left] = data_left[i + NT_Left] - tendon_endplate;
		/*
		 * The right Tendon
		 */
		tendon_endplate = (double) tendonRight *integ_current[i];
		tendon_endplate = (double) (tendon_endplate + TendonRightoffset);
		data_right[i + NT_Right] = data_right[i + NT_Right] - tendon_endplate;

	}

	/* The endplate effect */

	EndplateOffset = wfn_right[0] * integ_current[0];


	for (i = 0; i < NI; i++)
	{
		tendon_endplate = wfn_right[0] * integ_current[i];
		data_right[i] = data_right[i] + tendon_endplate - EndplateOffset;
		tendon_endplate = wfn_left[0] * integ_current[i];
		data_left[i] = data_left[i] + tendon_endplate - EndplateOffset;
	}


	/* Flipping and summing up the left and right parts */
	for (i = 0; i < convSize; i++)
	{
		data_right[i] = data_right[i] + data_left[i];
	}


	ckfree(integ_current);
	ckfree(currentExpand);

	return 1;

}

