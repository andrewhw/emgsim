/** ----------------------------------------------------------------
 ** This function adjusts the endplate and tendon artifact, using the algorithm
 ** mentioned in "A Model of EMG Generation", IEEE TRANSACTIONS ON BIOMEDICAL
 ** ENGINEERING, vol.47, Feb.2000
 ** The left and right affixes used in this function
 ** stand for the data related to the left part of
 ** the endplate and to the right part of the endplate respectively.
 ** They should be the same length (convSize).
 **
 ** $Id: adjust.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include "os_defs.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#include "tclCkalloc.h"
#include "mathtools.h"

OS_EXPORT void  plot_mah(double *convolution, int Size);



OS_EXPORT int
adjustConvArtifact(
		int NI,
		int convSize,
		int N_right,
		int N_left,
		float d_z,
		/* float  diameterInMM, */
		double *data_left,
		double *data_right,
		double *current,
		double *wfn_left, double *wfn_right
	)
{
	double         *integ_current = NULL;
	double          EndPlate = 0.0;
	double          sum = 0.0;
	double          tendonRight;
	double          tendonLeft;
	int             i, j;


	integ_current = (double *) ckalloc((NI + 1) * (sizeof(double)));
	tendonRight = wfn_right[N_right];
	tendonLeft = wfn_left[N_left];



	/* Step by Step current Integral */
	for (i = 1; i <= NI; i++)
	{
		sum = 0.0;
		for (j = 1; j <= i; j++)
		{
			sum = (double) (sum - current[j] * d_z);
		}
		integ_current[i] = (double) sum;
	}


	integ_current[0] = 0.0;
	/* plot_mah(integ_current,NI+1); */

	for (i = 1; i <= NI; i++)
	{
		integ_current[i] = integ_current[i] - integ_current[NI];
		/* Adjusting the EndPlate ConvArtifact */
		data_right[i] = data_right[i] + wfn_right[1] * integ_current[i];
		data_left[i] = data_left[i] + wfn_left[1] * integ_current[i];
		/* Adjusting the Tendon ConvArtifact */
		data_right[i + N_right] =
			data_right[i + N_right] - tendonRight * integ_current[i];
		data_left[i + N_left] =
			data_left[i + N_left] - tendonLeft * integ_current[i];
	}


	EndPlate = data_right[1] + data_left[1];


	/* Flipping and summing up the left and right parts */
	for (i = 0; i < N_right + N_left; i++)
	{
		data_right[i] = data_right[i] + data_left[i] - EndPlate;

	}                           /*
								   
								   if (i>__min(N_left,N_right))
								   {
									   if ( i == __min(N_left,N_right) + 1)
						   
										   tendonLeft = data_right[ __min(N_left,N_right) + 1 ] -
														   data_right[ __min(N_left,N_right) ];
						   
									   data_right[i] = data_right[i] - tendonLeft;
								   }
						   
						   
								   if (i>__max(N_left,N_right))
								   {
									   if (i == __max(N_left,N_right) + 1 )
						   
										   tendonRight = data_right[ __max(N_left,N_right) + 1 ] -
														   data_right[ __max(N_left,N_right) ];
						   
									   data_right[i] = data_right[i] - tendonRight;
								   }
							   }*/



    data_right[0] = 0.0;

    ckfree(integ_current);
    return 1;

}



/** This function calculates the initial z , in the way that the
 ** current integral would be the nearest possible to zero
 **/

OS_EXPORT double
optZinit(float z_inc, double currentfn_const, int NI)
{
	double         *Curr = NULL;
	double          IntegCurr = 0.0;
	float           z;
	int             j;

	Curr = (double *) ckalloc(NI * (sizeof(double)));

	z = z_inc;
	for (j = 0; j < NI; j++)
	{
		Curr[j] = (double)
			(currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) *
			 exp(-2.0 * z));
		z += z_inc;             /* In MM */
		IntegCurr = IntegCurr + Curr[j];
	}

	ckfree(Curr);
	return IntegCurr;
}

/** collects and writes the points in a file in order to plot.
 **/
OS_EXPORT void
plot_mah(double *convolution, int Size)

{
	FILE           *figure;
	figure = fopen("free.dat", "wb");
	fwrite(convolution, sizeof(double), Size, figure);
	fclose(figure);

}

