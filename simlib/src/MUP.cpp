/**
 ** MUP class -- store, recover and combine MFP's into MUPs
 ** with Jitter.
 **
 ** $Id: MUP.cpp 22 2017-11-17 18:02:31Z andrew $
 **/

#include "os_defs.h"

# ifndef    MAKEDEPEND
#  include    <stdio.h>
#  include    <string.h>
#  include    <math.h>
#  include    <errno.h>
#  ifdef    OS_WINDOWS_NT
#    include    <io.h>
#  else
#    include    <unistd.h>
#    include    <sys/types.h>
#    include    <fcntl.h>
#  endif
# endif

#include "SimulatorConstants.h"
#include "MUP.h"
#include "JitterDB.h"

#include "stringtools.h"
#include "pathtools.h"
#include "io_utils.h"
#include "listalloc.h"
#include "random.h"

#include "NRinterpolate.h"

#include "tclCkalloc.h"
#include "error.h"
#include "massert.h"
#include "filtertools.h"
#include "filetools.h"
#include "log.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#define         N_INTERPOLATION_CTRL_POINTS     4


struct MUP::JitterValue {
	long        jitterOffset_;
};

	/*
	 * acceleration threshold use to determine whether a MFP
	 * contributes to jitter
	 *
	 * 1.25 kv/ss -> uv/(sample_interval)^2
	 *
	 * multiply 1.250 kv to convert to uv (8 10^9), and divide by
	 * sampling-rate^ in s, so use DELTA_T_MUP in ms, and multiply
	 * DELTA_T_MUP by 10^3, so take 10^6 off of the top . . .
	 */
	generatedElement MUP::sAccelerationThreshold_ = 10.0; //(1.250);
		        // * (DELTA_T_MUP * DELTA_T_MUP);

	/*
	 * factor by which the jitterable buffers are expanded
	 */
int MUP::sExpansionFactor_ = 30;

MUP::MUP(const char *path, int id)
{
	char idBuffer[10], *tmpName;

	memset(this, 0, sizeof(MUP));

	dcoID_ = (-1);
	id_ = id;
	slnprintf(idBuffer, 10, "%0*d", MUP_ID_WIDTH, id_);

	if (path[strlen(path) - 1] == OS_PATH_DELIM)
	{
		tmpName = strconcat(path, "MUPData", idBuffer, ".dat", NULL);
	} else
	{
		tmpName = strconcat(path, OS_PATH_DELIM_STRING,
		        "MUPData", idBuffer, ".dat", NULL);
	}

	filename_ = osIndependentPath(tmpName);
	ckfree(tmpName);

	MUPAccelerationIsDirty_ = 1;
	MUPSlopeIsDirty_ = 1;

	nInterfaceDataPoints_ = (-1);
	expandedUnitsAlignmentOffset_ = (-1);
	MUPUnitsAlignmentPoint_ = (-1);
	alignmentMFP_ = (-1);

}

MUP::MUP(const char *filename)
{
	char *delim;

	memset(this, 0, sizeof(MUP));

	filename_ = ckstrdup(filename);
	delim = (char *) strstr(filename, "MUPData");
	if (delim == NULL)
	{
		id_ = (-1);
		filename_ = ckstrdup(filename);
	} else
	{
		delim += strlen("MUPData");
		id_ = strtol(delim, &delim, 10);
	}

	MUPAccelerationIsDirty_ = 1;
	MUPSlopeIsDirty_ = 1;

	nInterfaceDataPoints_ = (-1);
	expandedUnitsAlignmentOffset_ = (-1);
	MUPUnitsAlignmentPoint_ = (-1);
	alignmentMFP_ = (-1);

	load();
}

MUP::~MUP()
{
	unload();
	if (filename_ != NULL)      ckfree((void *) filename_);
}

generatedElement
MUP::sGetJitterAccelerationThreshold()
{
	return sAccelerationThreshold_;
}

void
MUP::sSetJitterAccelerationThreshold(generatedElement jitterThreshold)
{
	sAccelerationThreshold_ = jitterThreshold;
}

void
MUP::unload(void)
{
	int i;

	isLoaded_ = 0;

	if (MUPVector_ != NULL)
	{
		ckfree(MUPVector_);
		MUPVector_ = NULL;
	}

	if (MUPAccelerationVector_ != NULL)
	{
		ckfree(MUPAccelerationVector_);
		MUPAccelerationVector_ = NULL;
		MUPAccelerationIsDirty_ = 1;
	}

	if (MUPSlopeVector_ != NULL)
	{
		ckfree(MUPSlopeVector_);
		MUPSlopeVector_ = NULL;
		MUPSlopeIsDirty_ = 1;
	}

	if (jitterValue_ != NULL)
	{
		ckfree(jitterValue_);
		jitterValue_ = NULL;
	}
	if (jitterValueSums_ != NULL)
	{
		ckfree(jitterValueSums_);
		jitterValueSums_ = NULL;
	}

	if (nMFPs_ > 0)
	{
		if (mfapList_ != NULL)
		{
			for (i = 0; i < nMFPs_; i++)
			{
		        if (mfapList_[i] != NULL)
					delete mfapList_[i];
			}
			ckfree(mfapList_);
			mfapList_ = NULL;
		}
		nMFPs_ = 0;
	}

	if (hasCannulaMFP_ != 0 && cannulaMFP_ != NULL)
	{
		delete cannulaMFP_;
		cannulaMFP_ = NULL;
	}

	if (mfapLoadOffsets_ != NULL)
	{
		ckfree(mfapLoadOffsets_);
		mfapLoadOffsets_ = NULL;
	}

	if (mfapLoadFP_ != NULL)
	{
		closeFP(mfapLoadFP_);
		mfapLoadFP_ = NULL;
	}

	nInterfaceDataPoints_ = (-1);
	expandedUnitsAlignmentOffset_ = (-1);
	MUPUnitsAlignmentPoint_ = (-1);
	alignmentMFP_ = (-1);
}


MUP::MFP::MFP()
{
	memset(this, 0, sizeof(MFP));
}

MUP::MFP::~MFP()
{
	if (data_ != NULL)  ckfree(data_);
}



osInt32
MUP::calculateMaxSlopeAlignmentPoint__(
		MUPDataElement *data,
		int numDataPoints,
		int beginThresholdIndex
	)
{
	float *slopeData;
	float maxValue = (-1);
	osInt32 maxAmplitudeOffset = (-1);
	osInt32 maxSlopeOffset = (-1);
	int i;

	slopeData = (float *) ckalloc(sizeof(float) * numDataPoints);

	(void) calculateSlopeBuffer(
		        slopeData,
		        data,
		        numDataPoints,
		        1);

	maxValue = data[0];
	maxAmplitudeOffset = 0;
	for (i = 1; i < numDataPoints; i++)
	{
		if (data[i] > maxValue)
		{
		    maxValue = data[i];
		    maxAmplitudeOffset = i;
		}
	}

	if (maxAmplitudeOffset < 0)
		goto FAIL;


	maxValue = slopeData[0];
	maxSlopeOffset = 0;
	for (i = 1; i < maxAmplitudeOffset; i++)
	{
		if (slopeData[i] > maxValue)
		{
		    maxValue = slopeData[i];
		    maxSlopeOffset = i;
		}
	}

/*
	logFloatBuffer(data, numDataPoints,
		        "%s/debug-dump/MUP%d-mfap%d-data.txt",
		        g->muscle_dir, id_, nMFPs_);
	logFloatBuffer(slopeData, numDataPoints,
		        "%s/debug-dump/MUP%d-mfap%d-slope.txt",
		        g->muscle_dir, id_, nMFPs_);
*/

	ckfree(slopeData);

	return (maxSlopeOffset);


FAIL:
	ckfree(slopeData);
	return (-1);
}

void
MUP::addAsSeparateMFP__(
		generatedElement *data,
		int beginThresholdIndex,
		osInt32 *expandedSlopeAlignmentIndex,
		osInt32 fibreIdentifier
	)
{
	int status;
	int i, j, k, forceLinear;
	double lower, higher;

	status = listMkCheckSize(nMFPs_ + 1,
		        (void **) &mfapList_,
		        &nMFPBlocks_,
		        MFP_BLOCKSIZE,
		        sizeof(MFP *), __FILE__, __LINE__);

	MSG_ASSERT(status, "Allocation failed");
	mfapList_[nMFPs_] = new MFP();


	mfapList_[nMFPs_]->numPoints_ =
		        sExpansionFactor_ * nInterfaceDataPoints_;

	mfapList_[nMFPs_]->allocatedSize_ = mfapList_[nMFPs_]->numPoints_;
	mfapList_[nMFPs_]->expansionFactor_ = sExpansionFactor_;
	mfapList_[nMFPs_]->fibreIdentifier_ = fibreIdentifier;

	mfapList_[nMFPs_]->data_ = (MUPDataElement *)
		        ckalloc(mfapList_[nMFPs_]->allocatedSize_
		            * sizeof(MUPDataElement));

	memset(mfapList_[nMFPs_]->data_, 0,
		        mfapList_[nMFPs_]->allocatedSize_
		                * sizeof(MUPDataElement));


	for (i = 0; i < nInterfaceDataPoints_; i++)
	{

		forceLinear = 0;

		/** plug the matching elements in verbatim */
		mfapList_[nMFPs_]->data_[i * sExpansionFactor_] =
		            (MUPDataElement) data[i];

		lower = (MUPDataElement) data[i];


		/**
		 ** we handle the last 50 points differently to avoid
		 ** artifacts.  We inherit this value of 50 from
		 ** elsewhere in the code (makeMUP?)
		 **/
		if (i < nInterfaceDataPoints_ - 50)
		{
		    higher = (MUPDataElement) data[i + 1];
		} else
		{
		    higher = (MUPDataElement) data[i];
		    forceLinear = 1;
		}

		/** if we are at the beginning, force to linear */
		if (i < N_INTERPOLATION_CTRL_POINTS)
		{
		    forceLinear = 1;
		}


		/*
		 *    If we are not at (either) end of the data, check
		 *    if we are in a turn.  If we are not, interpolate
		 *    linearly, otherwise interpolate in a fancy way.
		 */
		if ( ( i > N_INTERPOLATION_CTRL_POINTS ) && ( ! forceLinear ) )
		{

		    cubicSplineInterpolation(
		                mfapList_[nMFPs_]->data_,
		                data,
		                nInterfaceDataPoints_,
		                sExpansionFactor_,
		                N_INTERPOLATION_CTRL_POINTS,
		                1.0,
		                i
		            );

		} else
		{
		    /** (linearly) interpolate for all of the other elements */
		    double linearStep = (((double) higher) - (double) lower)
		                / (double) sExpansionFactor_;

		    for (j = 1; j < sExpansionFactor_; j++)
			{
		        k = i * sExpansionFactor_ + j;
		        mfapList_[nMFPs_]->data_[k] =
		                (float) (lower + (j * linearStep));
		    }
		}
	}


	*expandedSlopeAlignmentIndex =
		        calculateMaxSlopeAlignmentPoint__(
		                mfapList_[nMFPs_]->data_,
		                mfapList_[nMFPs_]->numPoints_,
		                beginThresholdIndex
		            );

	// FIX -- debugging marker
//    mfapList_[nMFPs_]->data_[*expandedSlopeAlignmentIndex] = mfapList_[nMFPs_]->data_[*expandedSlopeAlignmentIndex] - 0.50f;

	/** dump the data we just added if desired */
#   ifdef      DUMP_MFP_DATA
	logFloatBuffer(
		        mfapList_[nMFPs_]->data_,
		        mfapList_[nMFPs_]->numPoints_,
		        "%s/mfap-dump/mfap%d-hifreq%ld.txt",
		        g->muscle_dir, id_, nMFPs_);
#   endif

	nMFPs_++;
}

void
MUP::addAsMergedMFP__(generatedElement *data)
{
	int i;


	/** if the storage isn't here, yet, make it appear */
	if (mfapList_[0] == NULL)
	{
		mfapList_[0] = new MFP();

		// note that this buffer is of low-sampling rate size
		mfapList_[0]->expansionFactor_ = 1;
		mfapList_[0]->numPoints_ = nInterfaceDataPoints_;
		mfapList_[0]->allocatedSize_ = mfapList_[0]->numPoints_;
		mfapList_[0]->data_ = (MUPDataElement *)
		        ckalloc(mfapList_[0]->allocatedSize_
		                * sizeof(MUPDataElement));
		// zero the vector so that we can accumulate data into
		// it below
		memset(mfapList_[0]->data_, 0,
		        nInterfaceDataPoints_ * sizeof(MUPDataElement));
	}

	// now we have a vector, potentially with data in it.
	// add in the new values as mods to the old ones.
	for (i = 0; i < nInterfaceDataPoints_; i++)
	{
		mfapList_[0]->data_[i] += (MUPDataElement) data[i];
	}


#   ifdef      DUMP_MFP_DATA
	logFloatBuffer(
		        mfapList_[0]->data_,
		        mfapList_[0]->numPoints_,
		        "%s/mfap-dump/mfap%d-lofreq.txt",
		        g->muscle_dir, id_);
#   endif
}

void
MUP::addMFP(
		int MUPIndex,
		int nElements,
		generatedElement *data,
		osInt32 fibreIdentifier
	)
{
	int status;
	int mfapThresholdIndex;
	osInt32 slopeAlignmentIndex;

	if (MUPIndex != 0)
		return;

	// ensure that the list of MFP's is initialized, so that
	// we always have room for the 0 element
	if (nMFPs_ == 0)
	{
		status = listMkCheckSize(MFP_BLOCKSIZE * 2,
		        (void **) &mfapList_,
		        &nMFPBlocks_,
		        MFP_BLOCKSIZE,
		        sizeof(MFP *), __FILE__, __LINE__);
		MSG_ASSERT(status, "Allocation failed");
		mfapList_[0] = NULL;
		nMFPs_ = 1;
		nInterfaceDataPoints_ = nElements;
	}

	MSG_ASSERT(nInterfaceDataPoints_ == nElements,
		        "MUP::addMFP - Vector size mismatch");


	mfapThresholdIndex = getOffsetWhereThresholdExceededDouble(
		        data, nInterfaceDataPoints_, 1,
		        sAccelerationThreshold_,
		        (float)(DELTA_T_MUP/1000.0),
		        (float)(1.0e-06), NULL
		    );


	/**
	 * if we are above threshold, keep this MFP separate,
	 * and determine whether it is the best (so far) candidate
	 * for the alignment MFP (that is, whether it occurs
	 * first
	 */
	if (mfapThresholdIndex >= 0)
	{
		addAsSeparateMFP__(
		                data,
		                mfapThresholdIndex,
		                &slopeAlignmentIndex,
		                fibreIdentifier
		            );

		/**
		 * If we are in the first (possibly only) iteration,
		 * or if our current guess is "better", then save
		 * the current MFP as the "alignment mfap", and
		 * keep track of the alignment point (returned as
		 * the firing time
		 */
		if (expandedUnitsAlignmentOffset_ < 0
		            || slopeAlignmentIndex
		                        < expandedUnitsAlignmentOffset_)
		{
		    alignmentMFP_ = (nMFPs_ - 1);
		    expandedUnitsAlignmentOffset_ = slopeAlignmentIndex;
		}
	} else
	{
		addAsMergedMFP__(data);
	}
}


void
MUP::addCannulaMFP(
		int MUPIndex,
		int nElements,
		generatedElement *data
	)
{
	int i;

	if (cannulaMFP_ == NULL)
	{
		cannulaMFP_ = new MFP();
		hasCannulaMFP_ = 1;

		cannulaMFP_->numPoints_ =
		        cannulaMFP_->allocatedSize_ = nElements;
		cannulaMFP_->expansionFactor_ = 1;
		cannulaMFP_->fibreIdentifier_ = (-1);

		cannulaMFP_->data_ = (MUPDataElement *)
		        ckalloc(cannulaMFP_->allocatedSize_
		            * sizeof(MUPDataElement));

		memset(cannulaMFP_->data_, 0,
		        cannulaMFP_->allocatedSize_ * sizeof(MUPDataElement));
	}

	/** accumulate the vector */
	for (i = 0; i < nElements; i++)
	{
		cannulaMFP_->data_[i] += (MUPDataElement) data[i];
	}


	/** dump the data we just added if desired */
#   ifdef      DUMP_MFP_DATA
	logFloatBuffer(
		        mfapList_[nMFPs_]->data_,
		        mfapList_[nMFPs_]->numPoints_,
		        "%s/mfap-dump/cannula-mfap.txt",
		        g->muscle_dir);
#   endif
}

/**
 **    set up the base MUP vector we will build the Jitter data on
 **/
void
MUP::initializeMUPVector__()
{
	/** Clear the whole MUP vector */
	memset(MUPVector_, 0,
		        nInterfaceDataPoints_ * sizeof(MUPDataElement));

	if (MUPAccelerationVector_ != NULL)
	{
		MUPAccelerationIsDirty_ = 1;
		memset(MUPAccelerationVector_, 0,
		            nInterfaceDataPoints_ * sizeof(MUPDataElement));
	}

	if (MUPSlopeVector_ != NULL)
	{
		MUPSlopeIsDirty_ = 1;
		memset(MUPSlopeVector_, 0,
		            nInterfaceDataPoints_ * sizeof(MUPDataElement));
	}
}

void MUP::resetJitterAccounting()
{
	if (jitterValueSums_ != NULL)
		memset(jitterValueSums_, 0, (nMFPs_ * sizeof(struct MUP::JitterValue)));
	nJitterValuesInSum_ = 0;
}


/**
 **    Generate some time offsets based on the Jitter values
 **/
void
MUP::selectJitterTimes__(
		int doJitter,
		float jitterVariance
	)
{
	double gaussValueInJitterUnits;
	double rawGaussValue;
	int i;

	/*
	 * Select some Jitter times
	 *
	 * Jitter is stored as an offset index used to shif the
	 * interpolated high-frequency buffer of "jitterable"
	 * MFPs.
	 */
	if (jitterValue_ == NULL)
	{
		jitterValue_ = (struct MUP::JitterValue *)
		        ckalloc(nMFPs_ * sizeof(struct MUP::JitterValue));
		jitterValueSums_ = (struct MUP::JitterValue *)
		        ckalloc(nMFPs_ * sizeof(struct MUP::JitterValue));
		memset(jitterValueSums_, 0,
		        (nMFPs_ * sizeof(struct MUP::JitterValue)));
		nJitterValuesInSum_ = 0;
	}

	memset(jitterValue_, 0,
		        (nMFPs_ * sizeof(struct MUP::JitterValue)));

	/* the first MFP (the composite one) doesn't Jitter */
	jitterValue_[0].jitterOffset_ = 0;


	/** generate Jitter values for all high-freq (> 1) MFPs */
	for (i = 1; i < nMFPs_; i++)
	{
		if ( ! doJitter )
		{
		    jitterValue_[i].jitterOffset_ = 0;
		} else
		{
		    // Generate data with a simple Gaussian distribution
		    //
		    // MCD estimates the std dev of the actual shift,
		    // so we take a factor of sqrt(2) in order to convert
		    // from straight variance.
		    rawGaussValue = gauss01();


		    // Scale the jitter value back by sqrt(2),
		    // in order to account for the fact that
		    // we are measuring the variance between
		    // jitter values
		    gaussValueInJitterUnits =
		                (rawGaussValue * jitterVariance
		                    * mfapList_[i]->expansionFactor_)
		                        / pow(2.0, 0.5);

		    // convert from the above double to an integer
		    // based offset, and store
		    jitterValue_[i].jitterOffset_ =
		                (long) gaussValueInJitterUnits;
		}

		jitterValueSums_[i].jitterOffset_ += jitterValue_[i].jitterOffset_;
		nJitterValuesInSum_++;
	}
}

/**
 **    Combine individual MFPs into a MUP with jitter info
 **    calculated above
 **
 **    Use:     private
 **/
void
MUP::combineMFPs__(
		MUPDataElement *loadBuffer,
		JitterIndividualOrTemplate jitterSourceSelection
	)
{
	long alignmentMFPFixupShift;
	int sourceBufferBaseIndex;
	int sourceBufferIndex;
	int mfapIndex;
	int i;

	/**
	 * Calculate how much "fixing" we will need to apply to the
	 * MFP buffers.  The "fix" is the amount we need to back-shift
	 * the buffer in order that the alignment buffer lines up
	 * with the final reported alignment time.
	 *
	 * We do this by ensuring that its Jitter is zero, and that
	 * the "alignmentMUPFixupOffset" from the alignment calculation
	 * is applied.
	 *
	 * Note that even though the Jitter will be zero for this value,
	 * we actually add this to all the other values, so that the
	 * Jitter is not lost; it is just applied to all the other
	 * units instead.
	 */
	if (alignmentMFP_ >= 0)
	{
		long jitterOffset, fullExpandedUnitOffset;

		if (jitterSourceSelection == JITTER_INDIVIDUAL)
			jitterOffset = jitterValue_[alignmentMFP_].jitterOffset_;
		else
			jitterOffset = (long)
					(jitterValueSums_[alignmentMFP_].jitterOffset_ / nJitterValuesInSum_);

		fullExpandedUnitOffset = expandedUnitsAlignmentOffset_ + jitterOffset;

		/**
		 * amount to shift Alignment MFP to have (Jittered) maximal
		 * slope point line up with the nearest, previous low-frequency
		 * sampling point
		 */
		alignmentMFPFixupShift = fullExpandedUnitOffset
		        % mfapList_[alignmentMFP_]->expansionFactor_;

		/**
		 * the number of low-frequency sampling points from the
		 * start of the buffer to the alignment point
		 */
		MUPUnitsAlignmentPoint_ = fullExpandedUnitOffset
		        / mfapList_[alignmentMFP_]->expansionFactor_;
	} else
	{
		alignmentMFPFixupShift = 0;
		MUPUnitsAlignmentPoint_ = (-1);
	}


	/**
	 * Iterate over all of the MFPs, adding them into the communal
	 * output buffer.
	 *
	 * Each MFP is added with an offset determined by its Jitter
	 * value, plus the shift used to have the MAUP align with a
	 * low frequency sample point (based on the alignment MFP)
	 *
	 * Now iterate over all of the MFPs, and add in the
	 * contributions for all of the remaining MFPs, Jittering
	 * them by their appropriate value if required.
	 *
	 * We only go to nInterfaceDataPoints_ - 1 in order to not
	 * run past the end of the interpolated buffer.
	 */
	for (mfapIndex = 1; mfapIndex < nMFPs_; mfapIndex++)
	{
		long jitterOffset;

		if (jitterSourceSelection == JITTER_INDIVIDUAL)
			jitterOffset = jitterValue_[mfapIndex].jitterOffset_;
		else
			jitterOffset = (long)
					(jitterValueSums_[mfapIndex].jitterOffset_ / nJitterValuesInSum_);

		/**
		 * a positive value of alignmentMFPFixupShift_ is
		 * a left-shift of the overall buffer, so we add
		 * the shift here.
		 */
		sourceBufferBaseIndex = (- jitterOffset) + alignmentMFPFixupShift;

		/** now iterate over all points in the output buffer */
		for (i = 0; i < nInterfaceDataPoints_; i++)
		{

		    sourceBufferIndex =
		                sourceBufferBaseIndex
		                + (i * mfapList_[mfapIndex]->expansionFactor_);


		    /**
		     * if the index into the source buffer is past the
		     * end of the buffer, then quit.
		     */
		    if (sourceBufferIndex >=
		                mfapList_[mfapIndex]->allocatedSize_)
			{
		        break;
		    }


		    /**
		     * If the point we want to grab is a valid point
		     * (in the MFP buffer) then use it.
		     *
		     * (If it is out of range, it is safe to ignore it
		     * as the output buffer was initialized to be all
		     * zeros anyway, and we are assuming that Jitter
		     * is never big enough to lose non-zero MFP data)
		     */
		    if (sourceBufferIndex >= 0)
			{
		        loadBuffer[i] += mfapList_[mfapIndex]->data_[
		                        sourceBufferIndex
		                    ];
		    }
		}
	}
}

/**
 **    Now add in the "low-frequency" data
 **
 **    Use:     private
 **/
void
MUP::addBaseMFPToSignal__(
		MUPDataElement *loadBuffer
	)
{
	int i;

	if (mfapList_[0] != NULL)
	{
		for (i = 0; i < nInterfaceDataPoints_; i++)
		{
		    MUPVector_[i] += mfapList_[0]->data_[i];
		}
	}
}

/**
 **    Now add in the "cannula" data
 **
 **    Use:     private
 **/
void
MUP::addCannulaMFPToSignal__(
		MUPDataElement *loadBuffer,
		ReferenceSetup referenceSetup
	)
{
	int i;

	if (hasCannulaMFP_)
	{
		if (referenceSetup == TIP_VERSUS_CANNULA)
		{
		    for (i = 0; i < nInterfaceDataPoints_; i++)
			{
		        MUPVector_[i] -= cannulaMFP_->data_[i];
		    }

		} else if (referenceSetup == CANNULA_ONLY)
		{
		    for (i = 0; i < nInterfaceDataPoints_; i++)
			{
		        MUPVector_[i] += cannulaMFP_->data_[i];
		    }
		}
	}
}

/**
 **    Generate a new MUP from the individual MFPs stored.
 **/
int
MUP::calcJitteredMUP(
		int MUPIndex,
		int doJitter,
		float jitterVariance,
		ReferenceSetup referenceSetup,
		JitterIndividualOrTemplate jitterSourceSelection
	)
{
	MSG_ASSERT(MUPIndex == 0,
		    "No code for moving needle in MUP structure currently\n");

	/* if we have no data, return NULL */
	if (nMFPs_ == 0)
	{
		LogInfo("We have no MFPS in MUP %d\n", id_);
		return 0;
	}

	if ( !  haveMFPsBeenLoaded__() )
		loadMFPs__();


	/** allocate a vector to return if we don't have enough space */
	if (MUPVector_ == NULL)
	{

		/*
		 * give ourselves a bit of room so we likely don't
		 * have to reallocate this too often
		 */
		MUPVector_ = (MUPDataElement *)
		        ckalloc(nInterfaceDataPoints_
		                * sizeof(MUPDataElement));
	}


	/** put the vector into initial state */
	initializeMUPVector__();


	if ((referenceSetup == TIP_VERSUS_CANNULA)
		        || (referenceSetup == TIP_ONLY))
	{
		/** generate some offsets based on the jitter information */
		if (jitterSourceSelection == JITTER_INDIVIDUAL)
			selectJitterTimes__(
					doJitter,
					jitterVariance
				);


		/** now generate the new composite MUP */
		combineMFPs__(MUPVector_, jitterSourceSelection);


		/** add the high and low frequency information */
		addBaseMFPToSignal__(MUPVector_);
	}

	if ((referenceSetup == TIP_VERSUS_CANNULA)
		        || (referenceSetup == CANNULA_ONLY))
	{

		/** add the cannula information */
		addCannulaMFPToSignal__(MUPVector_, referenceSetup);
	}


#   ifdef      DUMP_MUP_DATA
	logFloatBuffer(
		        MUPVector_,
		        nInterfaceDataPoints_,
		        "%s/MUP-dump/MUP%d-dump%d.txt",
		        g->muscle_dir, id_, MUPDumpId_++);
#   endif


	return 1;
}

/**
 **    return a MFP buffer
 **/
MUPDataElement *MUP::getMFP(
		        int index,
		        osInt32 *numberOfDataPoints,
		        osInt32 *expansionFactor,
		        osInt32 *fibreIdentifier
		    )
{
	if ( !  haveMFPsBeenLoaded__() )
		loadMFPs__();

	if (mfapList_[index] == NULL)
		return NULL;

	*numberOfDataPoints = mfapList_[index]->numPoints_;
	*expansionFactor = mfapList_[index]->expansionFactor_;
	*fibreIdentifier = mfapList_[index]->fibreIdentifier_;

	return mfapList_[index]->data_;
}

/**
 **    return the cannula buffer
 **/
MUPDataElement *MUP::getCannulaMFP(
		        osInt32 *numberOfDataPoints,
		        osInt32 *expansionFactor
		    )
{
	if ( !  haveMFPsBeenLoaded__() )
		loadMFPs__();

	if ( ! hasCannulaMFP_ )
		return NULL;

	*numberOfDataPoints = cannulaMFP_->numPoints_;
	*expansionFactor = cannulaMFP_->expansionFactor_;

	return cannulaMFP_->data_;
}

/**
 **    return where the alignment point should be.
 **/
int
MUP::getCurrentMUPAlignmentPoint(int MUPIndex) const
{
	MSG_ASSERT(MUPIndex == 0,
		"No code for moving needle in MUP structure currently\n");

	/**
	 * return the point we saved for alignment
	 */
	return MUPUnitsAlignmentPoint_;
}


/**
 **    Generate a new MUP from the individual MFPs stored.
 **/
MUPDataElement *
MUP::getCurrentMUP(int MUPIndex) const
{
	MSG_ASSERT(MUPIndex == 0,
		"No code for moving needle in MUP structure currently\n");

	/**
	 * return the MUP result, offset to beginning of
	 * first Jittered MFP
	 */
	return MUPVector_;
}


/**
 **    Generate a new MUP from the individual MFPs stored.
 **/
MUPDataElement *
MUP::getCurrentMUPAcceleration(int MUPIndex)
{
	MSG_ASSERT(MUPIndex == 0,
		"No code for moving needle in MUP structure currently\n");

	if (MUPAccelerationVector_ == NULL)
	{
		MUPAccelerationVector_ = (MUPDataElement *)
		        ckalloc(nInterfaceDataPoints_
		                * sizeof(MUPDataElement));
		MUPAccelerationIsDirty_ = 1;
	}

	if ( MUPAccelerationIsDirty_ )
	{
		/** Clear the acceleration vector */
		memset(MUPAccelerationVector_, 0,
		        nInterfaceDataPoints_ * sizeof(MUPDataElement));

		calculateAccelerationBuffer(
		        MUPAccelerationVector_,
		        MUPVector_,
		        nInterfaceDataPoints_, 1,
		        (float)(DELTA_T_MUP/1000.0),
		        (float)(1.0e-06));
	}

	/* return the result in the buffer */
	return MUPAccelerationVector_;
}

/**
 **    Generate a new MUP from the individual MFPs stored.
 **/
MUPDataElement *
MUP::getCurrentMUPSlope(int MUPIndex)
{
	MSG_ASSERT(MUPIndex == 0,
		"No code for moving needle in MUP structure currently\n");

	if (MUPSlopeVector_ == NULL)
	{
		MUPSlopeVector_ = (MUPDataElement *)
		        ckalloc(nInterfaceDataPoints_
		                * sizeof(MUPDataElement));
		MUPSlopeIsDirty_ = 1;
	}

	if ( MUPSlopeIsDirty_ )
	{
		/** Clear the acceleration vector */
		memset(MUPSlopeVector_, 0,
		        nInterfaceDataPoints_ * sizeof(MUPDataElement));

		calculateSlopeBuffer(
		        MUPSlopeVector_,
		        MUPVector_,
		        nInterfaceDataPoints_, 1);
	}

	/* return the result in the buffer */
	return MUPSlopeVector_;
}



off_t
MUP::headerOffsetSize__() const
{
	off_t result;

	result =  4 + MUP_ID_WIDTH + 2        // string at beginning
		+ sizeof(long) // (long) offset id
		+ sizeof(long) // (long) nMFP
		+ sizeof(long) // (long) nDataPoints in result
		+ sizeof(long)  // (long) alignmentMFP_
		+ sizeof(long)  // (long) expandedUnitsAlignmentOffset_
		+ sizeof(long)  // (long) flags (cannula present)
		+ sizeof(long) * nMFPs_;
	if (hasCannulaMFP_)
		result += sizeof(long);
	return result;
}

#define	MUP_ID_BUF_WIDTH (MUP_ID_WIDTH + 4 + 2 + 1)
int
MUP::writeHeader__(FP *fp, off_t *mfapOffsetTable) const
{
	char buffer[MUP_ID_BUF_WIDTH];
	int status = 1;
	int i;

	slnprintf(buffer, MUP_ID_BUF_WIDTH,
			"MUP %0*d; ", MUP_ID_WIDTH, id_);
	status &= wGeneric(fp, buffer, 4 + MUP_ID_WIDTH + 2);
	status &= w4byteInt(fp, (long) headerOffsetSize__());
	status &= w4byteInt(fp, nMFPs_);
	status &= w4byteInt(fp, nInterfaceDataPoints_);
	status &= w4byteInt(fp, alignmentMFP_);
	status &= w4byteInt(fp, expandedUnitsAlignmentOffset_);
	status &= w4byteInt(fp, (long) (hasCannulaMFP_ ? 1 : 0));
	for (i = 0; i < nMFPs_; i++)
	{
		status &= w4byteInt(fp, (long) mfapOffsetTable[i]);
	}
	if (hasCannulaMFP_)
	{
		status &= w4byteInt(fp, (long) mfapOffsetTable[nMFPs_]);
	}

	return status;
}


static const int READ_HEADER_BUFFER_SIZE = 64;
int
MUP::readHeader__(FP *fp, off_t **mfapOffsetTable)
{
	char buffer[READ_HEADER_BUFFER_SIZE];
	osInt32 headerSize, flags, loadLong;
	int status = 1;
	int i, c;

	if ( ! rGeneric(fp, buffer, 4) )                    return 0;
	if ((strncmp(buffer, "MUAP", 4) != 0) && (strncmp(buffer, "MUP ", 4) != 0))
	{
		Error("File does not begin with MUP header.\n");
		Error("    First 3 bytes [%s]\n", strunctrl(buffer, 4));
		return 0;
	}

	// load the id number portion into the buffer
	i = 0;
	do
	{
		c = getc(fp->fp);
		buffer[i++] = c;
	} while (c != 0 && c != ';' && i < READ_HEADER_BUFFER_SIZE);

	if (c == 0 || i >= READ_HEADER_BUFFER_SIZE)
	{
		Error("Cannot locate end of MUP header\n");
		return 0;
	}

	// terminate the buffer
	buffer[i] = 0;
	//if (sscanf(&buffer[4], "%0d;", &id_) < 0)
	if (sscanf(&buffer[4], "%d;", &id_) < 0)
	{
		Warning("Cannot convert id from '%s' in MUP file\n",
		        strunctrl(buffer, strlen(buffer)));
	}

	if ((c = getc(fp->fp)) != ' ')
	{
		Warning("Invalid delimiter character '%s' in MUP file\n",
		            chunctrl(c));
	}

	status &= r4byteInt(fp, &headerSize);
	status &= r4byteInt(fp, &nMFPs_);
	status &= r4byteInt(fp, &nInterfaceDataPoints_);
	status &= r4byteInt(fp, &alignmentMFP_);
	status &= r4byteInt(fp, &expandedUnitsAlignmentOffset_);
	status &= r4byteInt(fp, &flags);

	/** if flags are set, we will need a cannulaMFP */
	if (flags != 0)
	{
		hasCannulaMFP_ = 1;
	}

	/** ensure that the cast below will work */
	MSG_ASSERT(sizeof(osInt32) <= sizeof(off_t),
					"casting assumption broken");

	*mfapOffsetTable = (off_t *) ckalloc(sizeof(off_t) * (nMFPs_ + 1));
	for (i = 0; i < nMFPs_; i++)
	{
		status &= r4byteInt(fp, &loadLong);
		(*mfapOffsetTable)[i] = loadLong;
	}
	if (hasCannulaMFP_)
	{
		status &= r4byteInt(fp, &loadLong);
		(*mfapOffsetTable)[nMFPs_] = loadLong;
	} else
	{
		(*mfapOffsetTable)[nMFPs_] = (-1);
	}

	if (ftell(fp->fp) != headerSize)
	{
		fseek(fp->fp, headerSize, SEEK_SET);
	}
	return status;
}

int
MUP::writeData__(FP *fp, int index, MFP *curMFP, off_t *offset) const
{
	int status = 1;


	*offset = ftell(fp->fp);

	// if we are null, just write out a zero value;
	if (curMFP == NULL)
	{
		(void) w4byteInt(fp, 0); /** fibre id */
		return w4byteInt(fp, 0); /** num points */
	}

	// otherwise, write out all the data we have
	status &= w4byteInt(fp, curMFP->fibreIdentifier_);
	status &= w4byteInt(fp, curMFP->numPoints_);
	status &= wGeneric(fp, curMFP->data_,
		        curMFP->numPoints_ * sizeof(MUPDataElement));

	return status;
}


int
MUP::readData__(FP *fp, MFP **curMFP, off_t offset)
{
	osInt32 numPoints;
	osInt32 fibreIdentifier;
	int status = 1;

	fseek(fp->fp, offset, SEEK_SET);
	status &= r4byteInt(fp, &fibreIdentifier);
	status &= r4byteInt(fp, &numPoints);

	if (status)
	{
		if (numPoints == 0)
		{
		    (*curMFP) = NULL;
		} else
		{
		    (*curMFP) = new MFP();
		    (*curMFP)->fibreIdentifier_ = fibreIdentifier;
		    (*curMFP)->numPoints_ = numPoints;
		    (*curMFP)->allocatedSize_ = numPoints;
		    (*curMFP)->data_ = (MUPDataElement *)
		            ckalloc((*curMFP)->allocatedSize_
		                    * sizeof(MUPDataElement));

		    status &= rGeneric(fp, (*curMFP)->data_,
		            numPoints * sizeof(MUPDataElement));

		    (*curMFP)->expansionFactor_ =
		                numPoints / nInterfaceDataPoints_;

		    MSG_ASSERT((*curMFP)->expansionFactor_
		            * nInterfaceDataPoints_ == numPoints,
		            "Non-integral expansion factor found!");
		}
	}

	return status;
}

int
MUP::save() const
{
	off_t *mfapOffsets = NULL;
	FP *fp;
	int status = 1;
	int i;
	off_t headerSize;

	fp = openFP(filename_, "wb");

	MSG_ASSERT(fp != NULL, "Cannot open MUP data file");

	headerSize = headerOffsetSize__();
	if (nMFPs_ > 0)
	{
		mfapOffsets = (off_t *) ckalloc(sizeof(off_t) * (nMFPs_ + 1));

		fseek(fp->fp, headerSize, SEEK_SET);
		for (i = 0; i < nMFPs_; i++)
		{
		    status &= writeData__(fp, i, mfapList_[i], &mfapOffsets[i]);
		}

		if (hasCannulaMFP_)
		{
		    status &= writeData__(fp, 0, cannulaMFP_,
		                &mfapOffsets[nMFPs_]);
		}
	}
	fseek(fp->fp, 0, SEEK_SET);
	status &= writeHeader__(fp, mfapOffsets);

	if (nMFPs_ > 0)
		ckfree(mfapOffsets);

	closeFP(fp);

	return status;
}

int
MUP::load(int loadAllFlag)
{
	int status = 1;

	unload();

	mfapLoadFP_ = openFP(filename_, "rb");
	if (mfapLoadFP_ == NULL)
	{
		return 0;
	}

	status &= readHeader__(mfapLoadFP_, &mfapLoadOffsets_);

	if (loadAllFlag)
		loadMFPs__();

	isLoaded_ = 1;

	return status;
}

int
MUP::loadMFPs__()
{
	int status = 1;
	int i;

	if (nMFPs_ > 0)
	{
		mfapList_ = (MFP **) ckalloc(nMFPs_ * sizeof(MFP *));
		memset(mfapList_, 0, nMFPs_ * sizeof(MFP *));

		for (i = 0; i < nMFPs_; i++)
		{
		    status &= readData__(mfapLoadFP_,
							&mfapList_[i], mfapLoadOffsets_[i]);
		}
	}

	if (hasCannulaMFP_)
		status &= readData__(mfapLoadFP_,
						&cannulaMFP_, mfapLoadOffsets_[nMFPs_]);


	closeFP(mfapLoadFP_);
	mfapLoadFP_ = NULL;

	ckfree(mfapLoadOffsets_);
	mfapLoadOffsets_ = NULL;

	return status;
}

void
MUP::dump(FILE *fp) const
{
	fprintf(fp, "Dumping MUP Structure\n");
	fprintf(fp, "   id          : %d\n", id_);
	fprintf(fp, "   filename    :\n");
	fprintf(fp, "     '%s'\n", filename_);
	fprintf(fp, "   nMFPs      : %d\n", nMFPs_);
	fprintf(fp, "   nDataPoints : %d\n", nInterfaceDataPoints_);
}

void MUP::sSetExpansionFactor(int newExpansionFactor)
{
	sExpansionFactor_ = newExpansionFactor;
}

int MUP::sGetExpansionFactor()
{
	return sExpansionFactor_;
}

float MUP::getMUPSamplingRate() const
{
	return (float) DELTA_T_MUP;
}


