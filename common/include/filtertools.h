/**
 ** Routines used in the convolution tools file
 **
 ** $Id: filtertools.h 85 2011-03-29 18:05:57Z andrew $
 **/

#ifndef __FILTERING_HEADER__
#define __FILTERING_HEADER__

#include "os_defs.h"


# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT void  four1( double *points, int nn, int isign);

OS_EXPORT void  realft( double *data, int n, int isign);

OS_EXPORT int   convolve(
                                double *fft,
                                double *data, int n,
                                double *respns, int m,
                                int isign, double *ans, double deltaT
                        );


OS_EXPORT int adjustConvArtifact(
				int NI,
				int convSize,
				int N_right,
				int N_left,	
				float d_z,
				/*float  diameterInMM,*/
				double *data_left,
				double *data_right,
				double *current,
				double *wfn_left,double *wfn_right
				);
/** collects and writes the points in a file in order to plot. **/
OS_EXPORT void plot_mah(double *convolution,int Size);


OS_EXPORT double optZinit(
			  float z_inc, 
			  double currentfn_const, 
			  int MUPLength
			  );



OS_EXPORT void  twofft( double *data1, double *data2,
                                double *fft1, double *fft2, int n
                        );


#define         FILTAB_O8_31250_10_10000HZ              ((int)0x01)
#define         FILTAB_O8_31250_50_10000HZ              ((int)0x02)
#define         FILTAB_O8_31250_100_10000HZ             ((int)0x03)
#define         FILTAB_O8_31250_500_10000HZ             ((int)0x04)
#define         FILTAB_O8_31250_1000_10000HZ            ((int)0x05)


OS_EXPORT int   getABParams(
                                double **b, int *nB,
                                double **a, int *nA,
                                int key,
                                double sampleRate
                        );

OS_EXPORT int   filtfilt(
                                double *y,
                                double *x, int nX,
                                double *b, int nB,
                                double *a, int nA
                        );

OS_EXPORT int   filter(
                                double *y,
                                double *x, int nX,
                                double *b, int nB,
                                double *a, int nA
                        );
OS_EXPORT int calculateAccelerationBufferDouble(
                double *target, double *src, int nElements, int sampleDelta,
                float deltaTime, float conversionFactor
            );

OS_EXPORT int getOffsetWhereThresholdExceededDouble(
                double *src, int nElements, int sampleDelta, double threshold,
                float deltaTime, float conversionFactor,
                double *maxValueSeen
            );

OS_EXPORT int calculateAccelerationBuffer(
                float *target, float *src, int nElements, int sampleDelta,
                float deltaTime, float conversionFactor
            );

OS_EXPORT int getOffsetWhereThresholdExceeded(
                float *src, int nElements, int sampleDelta, float threshold,
                float deltaTime, float conversionFactor,
                float *maxValueSeen
            );

OS_EXPORT int calculateSlopeBuffer(
                float *target, float *src, int nElements, int sampleDelta
            );

OS_EXPORT int calculateSlopeBufferDouble(
                double *target, double *src, int nElements, int sampleDelta
            );

OS_EXPORT float calcPeakToPeakDifferenceFloat(
                float *buffer, int length
            );

OS_EXPORT double calcPeakToPeakDifferenceDouble(
                double *buffer, int length
            );

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif


