
/*
 * Copyright (c) 2008
 * All rights reserved.
 *
 * Original Authors:
 *	Andrew Hamilton-Wright  andrew@QEMG.org
 *	Daniel Stashuk          stashuk@pami.uwaterloo.ca
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The original authors must be made aware of, and party to, any
 *    redistribution of this source code.
 * 2. Any updates or modification to this code shall be made with
 *    the full knowledge of the original authors, so that all
 *    software using this file format may remain interoperable.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 *
 * IN NO EVENT SHALL THE AUTHORS, CONTRIBUTORS OR THE UNIVERSITIES OF
 * MOUNT ALLISON OR WATERLOO BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Handle the reading and writing of files in the DQEMG Data File
 * Format.
 *
 * $Id: DQEmgData.cpp 166 2016-11-28 15:12:43Z stashuk $
 */



#if defined( _WIN32 )
#   define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef MAKEDEPEND
# include	<stdio.h>
# include	<string.h>
# include	<math.h>
# include	<errno.h>
# include	<assert.h>
# include	<sys/types.h>
# include	<sys/stat.h>
#if defined( _WIN32 )
#   include	<io.h>
#   include <float.h>
# else
#   include	<unistd.h>
# endif
# include	<fcntl.h>
# include	<stdlib.h>
# include	<limits.h>
#endif

#include "dqemgjulian.h"
#include "DQEmgData.h"


/**
 * if we are building a DLL, include the Windows code; otherwise do not
 */
#ifdef DQD_DLL
# include "Windows.h"
#endif


/* uncomment this to add some data to test "forward compatibility" */
// #define	FORWARD_COMPAT_TEST
// #define	RESTRICT_TO_BASE_VERSION



/**
 * Ensure that our offsets are the right size on all the appropriate
 * platforms
 */
#if defined( _WIN32 )
	typedef	int DQEmgData32bit;

#elif defined( __NetBSD__ ) || defined( __OpenBSD__) || defined( __FreeBSD__ )
	typedef	int DQEmgData32bit;

#elif defined( linux )
	typedef	int DQEmgData32bit;

#elif defined( __APPLE__ )
	typedef	int DQEmgData32bit;

#endif



const char *DQEmgData::FILE_TAG = "DQEMG";
int   DQEmgData::FILE_TAG_LEN = 5;
int   DQEmgData::DEFAULT_VERSION = 0x01;

static int sMaxError_ = (-1);
static const char *sErrors_[] = {
		"no error",
		"errno-error",
		"not a DQEMG file",
		"incompatible version",
		"data overflow for type",
		"bad encoding",
		"unknown encoding",
		NULL
};

#if defined( _WIN32 )
# define	read(fd, buffer, len)	_read(fd, buffer, len)
# define	write(fd, buffer, len)	_write(fd, buffer, len)
# define	lseek(fd, offset, whence)	_lseek(fd, offset, whence)
#endif

/* context class to hold temporary calculations */
class DQEmgDataContext
{
public:
	off_t headerChannelOffsetLocation_;
	DQEmgData32bit *dataOffset32bit_;

public:
	DQEmgDataContext();
	~DQEmgDataContext();
};

DQEmgDataContext::DQEmgDataContext()
{
	memset(this, 0, sizeof(DQEmgDataContext));
}

DQEmgDataContext::~DQEmgDataContext()
{
}

/*
 * Create a whole new data set
 */
DQEmgData::DQEmgData()
{
	memset(this, 0, sizeof(DQEmgData));
	context_ = new DQEmgDataContext();

	setContractionDefaultValues_();

	assert(sizeof(DQEmgData32bit) == 4);
	assert(sizeof(float) == 4);
}

DQEmgData::DQEmgData(
		const char *vendorId,
		time_t acquisitionTime,
		const char *operatorDescription,
		const char *subjectDescription,
		const char *subjectID,
		int yearOfBirth,
		int monthOfBirth,
		int dayOfBirth,
		unsigned int age,
		Gender subjectGender,
		const char *muscleDescription,
		Side muscleSide,
		const char *generalDescription
	)
{
	memset(this, 0, sizeof(DQEmgData));
	context_ = new DQEmgDataContext();

	setContractionDefaultValues_();

	setVendorIdentifier(vendorId);
	setAcquisitionTime(acquisitionTime);
	setOperatorDescription(operatorDescription);

	setSubjectDescription(subjectDescription);
	setSubjectAge(age);
	setSubjectDateOfBirth(yearOfBirth, monthOfBirth, dayOfBirth);
	setSubjectGender(subjectGender);
	setSubjectID(subjectID);

	setMuscleDescription(muscleDescription);
	setMuscleSide(muscleSide);

	setGeneralDescription(generalDescription);
}


DQEmgData::DQEmgData(
		const char *vendorId,
		time_t acquisitionTime,
		const char *operatorDescription,
		const char *subjectDescription,
		const char *muscleDescription,
		const char *generalDescription
	)
{
	memset(this, 0, sizeof(DQEmgData));
	context_ = new DQEmgDataContext();

	setContractionDefaultValues_();

	setVendorIdentifier(vendorId);
	setAcquisitionTime(acquisitionTime);
	setOperatorDescription(operatorDescription);
	setSubjectDescription(subjectDescription);
	setMuscleDescription(muscleDescription);
	setGeneralDescription(generalDescription);

}

DQEmgData::DQEmgData(const char *vendorId)
{
	memset(this, 0, sizeof(DQEmgData));
	context_ = new DQEmgDataContext();

	setContractionDefaultValues_();

	setVendorIdentifier(vendorId);
}

/*
 * delete one of these objects
 */
DQEmgData::~DQEmgData()
{
	clear();
}

/**
 ** This is called from the constructor functions
 **/
void DQEmgData::setContractionDefaultValues_()
{
	fileVersion_ = (-1);
	acquisitionTime_ = time(NULL);
	subjectJulianDateOfBirth_ = 0;
	subjectAge_ = 0;
	subjectGender_ = 0;
	studyFlags_ = 0;
	muscleSide_ = 0;
	contractionType_ = DQEmgData::NOT_DEFINED;
	cmapStored_ = 0;
	mSmupStored_ = 0;
	numCmapStored_ = 0; //ABDSAVE
	numAutoSmupStored_ =0;//mauto

	/*MarzTest*/
	indexOfFirstSmup_ = 1;
	indexOfSecondSmup_ = 1;
	indexOfThirdSmup_ = 1;
	indexOfBaseSmup_ = 1;
	FreqOfFirstSmup_ = 0;
	FreqOfSecondSmup_ = 0;
	FreqOfThirdSmup_ = 0;
	FreqOfBaseSmup_ = 0;
	/*MarzTest*/
}

void DQEmgData::clear()
{
	unsigned long i;

	if (context_ != NULL)
	{
		if (context_->dataOffset32bit_ != NULL)
		{
			delete [] context_->dataOffset32bit_;
		}
		delete context_;
		context_ = NULL;
	}

	if ( filename_ != NULL)
	{
		delete [] filename_;
		filename_ = NULL;
	}

	if ( vendorIdentifier_ != NULL)
	{
		delete [] vendorIdentifier_;
		vendorIdentifier_ = NULL;
	}

	if ( operatorDescription_ != NULL)
	{
		delete [] operatorDescription_;
		operatorDescription_ = NULL;
	}

	if ( subjectDescription_ != NULL)
	{
		delete [] subjectDescription_;
		subjectDescription_ = NULL;
	}

	if ( subjectID_ != NULL)
	{
		delete [] subjectID_;
		subjectID_ = NULL;
	}

	if ( muscleDescription_ != NULL)
	{
		delete [] muscleDescription_;
		muscleDescription_ = NULL;
	}

	if ( generalDescription_ != NULL)
	{
		delete [] generalDescription_;
		generalDescription_ = NULL;
	}

	if (data_ != NULL)
	{
		for (i = 0; i < numChannels_; i++)
		{
			delete data_[i];
		}
		delete [] data_;
		data_ = NULL;
	}
}


/*
 *	Create a Channel object
 */
DQEmgChannelData::DQEmgChannelData()
{
	memset(this, 0, sizeof(DQEmgChannelData));
	encoding_ = (int) SHORT;

	setChannelDefaultValues_();
}

DQEmgChannelData::DQEmgChannelData(
		unsigned char channelNumber,
		unsigned long highPassCutoff,
		unsigned long lowPassCutoff,
		const char *channelDescription
	)
{
	memset(this, 0, sizeof(DQEmgChannelData));
	encoding_ = (int) SHORT;

	setChannelDefaultValues_();

	setChannelNumber(channelNumber);
	setHighPassCutoff(highPassCutoff);
	setLowPassCutoff(lowPassCutoff);
	setChannelDescription(channelDescription);
}

/*
 * destroy one of these objects
 */
DQEmgChannelData::~DQEmgChannelData()
{
	if ( channelDescription_ != NULL)
	{
		delete [] channelDescription_;
		channelDescription_ = NULL;
	}

	if ( unitDescription_ != NULL)
	{
		delete [] unitDescription_;
		unitDescription_ = NULL;
	}

	if ( dataShortSample_ != NULL)
		delete [] dataShortSample_;

	if ( dataLongSample_ != NULL)
		delete [] dataLongSample_;
}

void DQEmgChannelData::setChannelDefaultValues_()
{
	negativePeakArea_ = 0;
	peakToPeakAmplitude_ = 0;
	negativePeakAmplitude_ = 0;
	onsetPos_ = 0;
	positivePeakPos_ = 0;
	endPos_ = 0;
	index_ = 0;

	duration_ = 0;
	negativePeakDuration_ = 0;
	area_ = 0;
	peakOnsetPos_ = 0;
	peakEndPos_ = 0;

	isValid_ = 0;   // Default is false
	numSmupsInAvg_ = 0;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/////////////////  Channel  ////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

int DQEmgChannelData::convertDataToShort_()
{
	unsigned long i;

	dataShortSample_ = new short[numSamples_];

	for (i = 0; i < numSamples_; i++)
	{
		if (dataLongSample_[i] >= (long) USHRT_MAX)
		{
			return 0;
		}
		dataShortSample_[i] = (short) dataLongSample_[i];
	}

	return 1;
}
int DQEmgChannelData::convertDataToLong_()
{
	unsigned long i;

	dataLongSample_ = new long[numSamples_];

	for (i = 0; i < numSamples_; i++)
	{
		dataLongSample_[i] = (long) dataShortSample_[i];
	}

	return 1;
}

short *DQEmgChannelData::getDataAsShort(
		unsigned long *numSamples,
		float *samplingRate,
		float *scale
	)
{
	*numSamples = numSamples_;
	*scale = scale_;
	*samplingRate = samplingRate_;
	if (dataShortSample_ == NULL)
	{
		if ( ! convertDataToShort_() )
		{
			return NULL;
		}
	}
	return dataShortSample_;
}

long *DQEmgChannelData::getDataAsLong(
		unsigned long *numSamples,
		float *samplingRate,
		float *scale
	)
{
	*numSamples = numSamples_;
	*scale = scale_;
	*samplingRate = samplingRate_;
	if (dataLongSample_ == NULL)
	{
		if ( ! convertDataToLong_() )
		{
			return NULL;
		}
	}
	return dataLongSample_;
}

void DQEmgChannelData::deleteOldData_()
{
	if (dataShortSample_ != NULL)
	{
		delete [] dataShortSample_;
	}
	if (dataLongSample_ != NULL)
	{
		delete [] dataLongSample_;
	}
	numSamples_ = 0;
}

int DQEmgChannelData::setData(
		unsigned long numSamples,
		short *data,
		float samplingRate,
		const char *unitDescription,
		float scale
	)
{
	deleteOldData_();
	numSamples_ = numSamples;
	dataShortSample_ = new short[numSamples_];
	memcpy(dataShortSample_, data, numSamples_ * sizeof(short));

	setUnitDescription_(unitDescription);
	samplingRate_ = samplingRate;
	scale_ = scale;

	return 1;
}

int DQEmgChannelData::setData(
		unsigned long numSamples,
		float *data,
		float samplingRate,
		const char *unitDescription,
		int encodingWidth,
		long maxUnscaledValue
	)
{
	unsigned long i;
	float absData, maxAbsValueRecorded = 0;
	double scaleRequired;

	deleteOldData_();

	if ( ! setEncoding(
				(DQEmgChannelData::Encoding)(
						(encoding_ & 0xFF00) | encodingWidth)
			) )
		return 0;

	if (maxUnscaledValue == 0)
	{
		if (encodingWidth == 2)
		{
			maxUnscaledValue = SHRT_MAX;
		} else if (encodingWidth == 4){
			maxUnscaledValue = LONG_MAX;
		} else
		{
			return 0;
		}
	}

	for (i = 0; i < numSamples; i++)
	{
		absData = (float) fabs(data[i]);
		if (maxAbsValueRecorded < absData)
			maxAbsValueRecorded = absData;
	}

	/**
	 * by definition the scale we require is (the inverse of)
	 * that which will adjust the maximum absolude value so that
	 * it will be the maximum unscaled value.  We want:
	 *		(max abs stored value) * scale = maxAbsValueRecorded
	 * which is:
	 *		maxUnscaledValue * scale = maxAbsValueRecorded
	 *		scale = maxAbsValueRecorded / maxUnscaledValue
	 */
	scaleRequired = (maxAbsValueRecorded / maxUnscaledValue);

	if (encodingWidth == 2)
	{
		short sValue;
		double convValue;
		dataShortSample_ = new short[numSamples];
		for (i = 0; i < numSamples; i++)
		{
			convValue =  ((double) data[i] / scaleRequired);
			sValue = (short) convValue;
			dataShortSample_[i] = sValue;
		}
		
	} else
	{
		long lValue;
		dataLongSample_ = new long[numSamples];
		for (i = 0; i < numSamples; i++)
		{
			lValue = (long) ((double) data[i] / scaleRequired);
			dataLongSample_[i] = lValue;
		}
	}

	numSamples_ = numSamples;
	setUnitDescription_(unitDescription);
	samplingRate_ = samplingRate;
	scale_ = (float) scaleRequired;

	return 1;
}


int DQEmgChannelData::setData(
		unsigned long numSamples,
		long *data,
		float samplingRate,
		const char *unitDescription,
		float scale
	)
{
	deleteOldData_();
	numSamples_ = numSamples;
	dataLongSample_ = new long[numSamples_];
	memcpy(dataLongSample_, data, numSamples_ * sizeof(long));

	setUnitDescription_(unitDescription);
	samplingRate_ = samplingRate;
	scale_ = scale;

	return 1;
}

int DQEmgChannelData::setEncoding(DQEmgChannelData::Encoding encoding)
{
	if (((int) encoding) == 0x0002 || ((int) encoding) == 0x0004)
	{
		encoding_ = encoding;
		return 1;
	}
	return 0;
}
int DQEmgChannelData::getEncodingFlags() const
{
	return ((encoding_ & 0xFF00) >> 8);
}
int DQEmgChannelData::getEncodingWidth() const
{
	return (encoding_ & 0x00FF);
}

void DQEmgChannelData::setChannelNumber(unsigned char channelNumber)
{
	channelNumber_ = channelNumber;
}
unsigned char DQEmgChannelData::getChannelNumber() const
{
	return channelNumber_;
}

unsigned long DQEmgChannelData::getHighPassCutoff() const
{
	return hipassCutoff_;
}
void DQEmgChannelData::setHighPassCutoff(unsigned long newFreq)
{
	hipassCutoff_ = newFreq;
}
unsigned long DQEmgChannelData::getLowPassCutoff() const
{
	return lopassCutoff_;
}
void DQEmgChannelData::setLowPassCutoff(unsigned long newFreq)
{
	lopassCutoff_ = newFreq;
}

unsigned long DQEmgChannelData::getElapsedTime() const
{
	return elapsedTime_;
}
void DQEmgChannelData::setElapsedTime(unsigned long newTime)
{
	elapsedTime_ = newTime;
}

void DQEmgChannelData::setEvokedResponseNegativePeakArea(float negativePeakArea)
{
	negativePeakArea_ = negativePeakArea;
}

float DQEmgChannelData::getEvokedResponseNegativePeakArea() const
{
	return negativePeakArea_ ;
}

void DQEmgChannelData::setEvokedResponsePeakToPeakAmplitude(float peakToPeakAmplitude)
{
	peakToPeakAmplitude_ = peakToPeakAmplitude;
}

float DQEmgChannelData::getEvokedResponsePeakToPeakAmplitude() const
{
	return  peakToPeakAmplitude_ ;
}

void DQEmgChannelData::setEvokedResponseNegativePeakAmplitude(float negativePeakAmplitude)
{
	negativePeakAmplitude_ = negativePeakAmplitude;
}

float DQEmgChannelData::getEvokedResponseNegativePeakAmplitude() const
{
	return negativePeakAmplitude_ ;
}

void DQEmgChannelData::setEvokedResponseValidity(bool Valid)
{
	isValid_= Valid;
}

bool DQEmgChannelData::getEvokedResponseValidity() const
{
	return isValid_;
}

void DQEmgChannelData::setNumberOfSmupsInAvg(int NumSmups)
{
	numSmupsInAvg_ = NumSmups;
}

int DQEmgChannelData::getNumberOfSmupsInAvg() const
{
	return numSmupsInAvg_;
}

unsigned long DQEmgChannelData::getNumberOfSamples() const
{
	return numSamples_;
}

// Additional statistics used for evoked responses 
void DQEmgChannelData::setEvokedResponseDuration(float duration)
{
	duration_ = duration;
}
float DQEmgChannelData::getEvokedResponseDuration() const
{
	return duration_;
}

void DQEmgChannelData::setEvokedResponseNegativePeakDuration(float negativePeakDuration)
{
	negativePeakDuration_ = negativePeakDuration;
}

float DQEmgChannelData::getEvokedResponseNegativePeakDuration() const
{
	return negativePeakDuration_;
}

void DQEmgChannelData::setEvokedResponseArea(float area)
{
	area_ = area;
}

float DQEmgChannelData::getEvokedResponseArea() const
{
	return area_;
}

void DQEmgChannelData::setEvokedResponsePeakOnsetPos(short peakOnsetPos)
{
	peakOnsetPos_ = peakOnsetPos;
}

short DQEmgChannelData::getEvokedResponsePeakOnsetPos() const
{
	return peakOnsetPos_;
}

void DQEmgChannelData::setEvokedResponsePeakEndPos(short peakEndPos)
{
	peakEndPos_ = peakEndPos;
}

short DQEmgChannelData::getEvokedResponsePeakEndPos() const
{
	return peakEndPos_;
}

float DQEmgChannelData::getSamplingRate() const
{
	return samplingRate_;
}


float DQEmgChannelData::getScaleFactor() const
{
	return scale_;
}

const char * DQEmgChannelData::getChannelDescription() const
{
	return channelDescription_;
}
void DQEmgChannelData::setChannelDescription(
		const char *newDescription
	)
{
	if (channelDescription_ != NULL)
	{
		delete [] channelDescription_;
	}

	if (newDescription != NULL)
	{
		int len;
		len = (int) strlen(newDescription) + 1;
		channelDescription_ = new char[ len ];
		memcpy(channelDescription_, newDescription, len);
		
	} else
	{
		channelDescription_ = NULL;
	}

}

const char * DQEmgChannelData::getUnitDescription() const
{
	return unitDescription_;
}
void DQEmgChannelData::setUnitDescription_(
		const char *newDescription
	)
{
	if (unitDescription_ != NULL)
	{
		delete [] unitDescription_;
	}

	if (newDescription != NULL)
	{
		int len;
		len = (int) strlen(newDescription) + 1;
		unitDescription_ = new char[ len ];
		memcpy(unitDescription_, newDescription, len);
		
	} else
	{
		unitDescription_ = NULL;
	}
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/////////////////  Data  ///////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


const char *DQEmgData::getVendorIdentifier() const
{
	return vendorIdentifier_;
}
int DQEmgData::setVendorIdentifier(const char *newId)
{
	if (vendorIdentifier_ != NULL)
	{
		delete [] vendorIdentifier_;
		vendorIdentifier_ = NULL;
	}

	if (newId == NULL)
		return 0;

	if (strlen(newId) <= MAX_VENDOR_ID_LENGTH)
	{
		/**
		 * the entire string will fit within the specified
		 * storage length, so save the whole thing
		 */
		int len;
		len = (int) strlen(newId) + 1;
		vendorIdentifier_ = new char[ len ];
		memcpy(vendorIdentifier_, newId, len);
	} else
	{
		/**
		 * the string is longer than the specified length
		 * so we create a buffer long enough for the specifier
		 * and copy the specified number of characters into
		 * the buffer
		 */
		vendorIdentifier_ = new char[ MAX_VENDOR_ID_LENGTH + 1];
		vendorIdentifier_[MAX_VENDOR_ID_LENGTH] = 0;
		memcpy(vendorIdentifier_, newId, MAX_VENDOR_ID_LENGTH);
	}

	return 1;
}

const char *DQEmgData::getOperatorDescription() const
{
	return operatorDescription_;
}
void DQEmgData::setOperatorDescription(const char *newDesc)
{
	if (operatorDescription_ != NULL)
	{
		delete [] operatorDescription_;
	}


	if (newDesc != NULL)
	{
		int len;
		len = (int) strlen(newDesc) + 1;
		operatorDescription_ = new char[ len ];
		memcpy(operatorDescription_, newDesc, len);

	} else
	{
		operatorDescription_ = NULL;
	}
}

/** set all the subject info in one go */
void DQEmgData::setAllSubjectInformation(
		const char *description,
		const char *id,
		int yearOfBirth,
		int monthOfBirth,
		int dayOfBirth,
		unsigned int age,
		DQEmgData::Gender gender
	)
{
	setSubjectDescription(description);
	setSubjectID(id);
	setSubjectDateOfBirth(yearOfBirth, monthOfBirth, dayOfBirth);
	setSubjectAge(age);
	setSubjectGender(gender);
}

const char *DQEmgData::getSubjectDescription() const
{
	return subjectDescription_;
}
void DQEmgData::setSubjectDescription(const char *newDesc)
{
	if (subjectDescription_ != NULL)
	{
		delete [] subjectDescription_;
	}

	if (newDesc != NULL)
	{
		int len;
		len = (int) strlen(newDesc) + 1;
		subjectDescription_ = new char[ len ];
		memcpy(subjectDescription_, newDesc, len);
		
	} else
	{
		subjectDescription_ = NULL;
	}
}

const char *DQEmgData::getSubjectID() const
{
	return subjectID_;
}
void DQEmgData::setSubjectID(const char *newID)
{
	if (subjectID_ != NULL)
	{
		delete [] subjectID_;
	}

	if (newID != NULL)
	{
		int len;
		len = (int) strlen(newID) + 1;
		subjectID_ = new char[ len ];
		memcpy(subjectID_, newID, len);
		
	} else
	{
		subjectID_ = NULL;
	}
}

int DQEmgData::getSubjectDateOfBirth(
		unsigned long *year,
		unsigned long *month,
		unsigned long *day
	) const
{
	return dqGetGregorianDate(year, month, day,
			subjectJulianDateOfBirth_);
}
long DQEmgData::getSubjectJulianDateOfBirth() const
{
	return subjectJulianDateOfBirth_;
}
int DQEmgData::setSubjectDateOfBirth(
		int yearOfBirth,
		int monthOfBirth,
		int dayOfBirth
	)
{
	int status;
	status = dqGetJulianDate(
			&subjectJulianDateOfBirth_,
			yearOfBirth, monthOfBirth, dayOfBirth);

	if ( ! status )
		subjectJulianDateOfBirth_ = 0;

	return status;
}

void DQEmgData::setSubjectAge(unsigned int age)
{
	subjectAge_ = age;
}

unsigned int DQEmgData::getSubjectAge()
{
	return subjectAge_;
}

int DQEmgData::getSubjectGender() const
{
	return (int) subjectGender_;
}
void DQEmgData::setSubjectGender(DQEmgData::Gender gender)
{
	subjectGender_ = (char) gender;
}

int DQEmgData::isCmapStored()
{
	return (int) cmapStored_;
}
void DQEmgData::setCmapStored(int isStored)
{
	cmapStored_ = isStored;
}
int DQEmgData::numCmapsStored() //ABD
{
	return numCmapStored_;
}
void DQEmgData::setNumCmapStored(int num) //ABD
{
	numCmapStored_ = num;
}
int DQEmgData::numAutoSmupStored() //mauto
{
	return numAutoSmupStored_;
}
void DQEmgData::setNumAutoSmupStored(int num) //mauto
{
	numAutoSmupStored_ = num;
}

/*MarzTest*/
void DQEmgData::setIndexOfFirstSmup(int index)   
{
	indexOfFirstSmup_ = index;
}
int DQEmgData::getIndexOfFirstSmup() 				 
{
	return indexOfFirstSmup_;
}
void DQEmgData::setIndexOfSecondSmup(int index)   
{
	indexOfSecondSmup_ = index;
}
int DQEmgData::getIndexOfSecondSmup() 				 
{
	return indexOfSecondSmup_;
}
void DQEmgData::setIndexOfThirdSmup(int index)   
{
	indexOfThirdSmup_ = index;
}
int DQEmgData::getIndexOfThirdSmup()				 
{
	return indexOfThirdSmup_;
}
void DQEmgData::setIndexOfBaseSmup(int index)   
{
	indexOfBaseSmup_ = index;
}
int DQEmgData::getIndexOfBaseSmup() 		 
{
	return indexOfBaseSmup_;
}


void DQEmgData::setFreqOfFirstSmup(float Freq)   
{
	FreqOfFirstSmup_ = Freq;
}
float DQEmgData::getFreqOfFirstSmup() 		 
{
	return FreqOfFirstSmup_;
}
void DQEmgData::setFreqOfSecondSmup(float Freq)   
{
	FreqOfSecondSmup_ = Freq;
}
float DQEmgData::getFreqOfSecondSmup() 				 
{
	return FreqOfSecondSmup_;
}
void DQEmgData::setFreqOfThirdSmup(float Freq)   
{
	FreqOfThirdSmup_ = Freq;
}
float DQEmgData::getFreqOfThirdSmup() 		 
{
	return FreqOfThirdSmup_;
}
void DQEmgData::setFreqOfBaseSmup(float Freq)   
{
	FreqOfBaseSmup_ = Freq;
}
float DQEmgData::getFreqOfBaseSmup() 				 
{
	return FreqOfBaseSmup_;
}
/*MarzTest*/
int DQEmgData::is_mSmupStored()
{
	return (int) mSmupStored_;
}
void DQEmgData::set_mSmupStored(int isStored)
{
	mSmupStored_ = isStored;
}

unsigned long DQEmgData::isNewOperator() const
{
	return (studyFlags_ & FLAG_NEW_OPERATOR) != 0;
}
void DQEmgData::setNewOperator(int newOperator)
{
	studyFlags_ = studyFlags_ & ~FLAG_NEW_OPERATOR;
	if (newOperator)
		studyFlags_ = studyFlags_ | FLAG_NEW_OPERATOR;
}

unsigned long DQEmgData::isNewPatient() const
{
	return (studyFlags_ & FLAG_NEW_PATIENT) != 0;
}

void DQEmgData::setNewPatient(int newPatient)
{
	studyFlags_ = studyFlags_ & ~FLAG_NEW_PATIENT;
	if (newPatient)
		studyFlags_ = studyFlags_ | FLAG_NEW_PATIENT;
}

unsigned long DQEmgData::isNewMuscle() const
{
	return (studyFlags_ & FLAG_NEW_MUSCLE) != 0;
}
void DQEmgData::setNewMuscle(int newMuscle)
{
	studyFlags_ = studyFlags_ & ~FLAG_NEW_MUSCLE;
	if (newMuscle)
		studyFlags_ = studyFlags_ | (FLAG_NEW_MUSCLE);
}

const char *DQEmgData::getMuscleDescription() const
{
	return muscleDescription_;
}
void DQEmgData::setMuscleDescription(const char *newDesc)
{
	if (muscleDescription_ != NULL)
	{
		delete [] muscleDescription_;
	}

	if (newDesc != NULL)
	{
		int len;
		len = (int) strlen(newDesc) + 1;
		muscleDescription_ = new char[ len ];
		memcpy(muscleDescription_, newDesc, len);
		
	} else
	{
		muscleDescription_ = NULL;
	}
}

int DQEmgData::getMuscleSide() const
{
	return (int) muscleSide_;
}
void DQEmgData::setMuscleSide(DQEmgData::Side side)
{
	muscleSide_ = (char) side;
}

const char *DQEmgData::getGeneralDescription() const
{
	return generalDescription_;
}
void DQEmgData::setGeneralDescription(const char *newDesc)
{
	if (generalDescription_ != NULL)
	{
		delete [] generalDescription_;
	}

	if (newDesc != NULL)
	{
		int len;
		len = (int) strlen(newDesc) + 1;
		generalDescription_ = new char[ len ];
		memcpy(generalDescription_, newDesc, len);
		
	} else
	{
		generalDescription_ = NULL;
	}
}

void DQEmgData::setFileName(const char *newName)
{
	if (filename_ != NULL)
	{
		delete [] filename_;
	}
	int len;
	len = (int) strlen(newName) + 1;
	filename_ = new char[ len ];
	memcpy(filename_, newName, len);
}

const char *DQEmgData::getFileName() const
{
	return filename_;
}

time_t DQEmgData::getAcquisitionTime() const
{
	return acquisitionTime_;
}
void DQEmgData::setAcquisitionTime(time_t newTime)
{
	acquisitionTime_ = newTime;
}

unsigned char DQEmgData::getNumChannels() const
{
	return numChannels_;
}
int DQEmgData::addChannel(DQEmgChannelData *newChannel)
{
	DQEmgChannelData **oldList;
	unsigned long i;

	if (numChannels_ == 255)
		return 0;

	oldList = data_;

	data_ = new DQEmgChannelData *[ numChannels_ + 1 ];
	memset(data_, 0, sizeof(DQEmgChannelData *) * (numChannels_ + 1));

	/**
	 * move over all the existing channels.  If there
	 * are none, then numChannels will be zero, and the
	 * loop will do nothing other than initialize i
	 */
	for (i = 0; i < numChannels_; i++)
	{
		data_[i] = oldList[i];
	}
	data_[numChannels_++] = newChannel;

	if (oldList != NULL)
		delete [] oldList;

	return 1;
}
DQEmgChannelData *DQEmgData::getChannel(unsigned char index) const
{
	if (data_ != NULL)
	{
		if (index < numChannels_)
		{
			return data_[index];
		}
	}

	return NULL;
}

void DQEmgData::setContractionType(Contraction contractionType)
{
	contractionType_ = (char)contractionType;
}
int  DQEmgData::getContractionType()
{
	return (int)contractionType_;
}
int DQEmgData::isEvoked()
{
	if (contractionType_== EVOKED)
		return true;
	else
		return false;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/////////////////  I/O  ////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

int DQEmgData::load(const char *filename)
{
	int fd, result;;

	setFileName(filename);
#if defined( _WIN32 )
	fd = _open(filename_, O_RDONLY|O_BINARY, 0);
#else
	fd = open(filename_, O_RDONLY, 0);
#endif
	if (fd < 0)
		return 0;

	result = load_(fd);

#if defined( _WIN32 )
	result = (_close(fd) >= 0) && result;
#else
	result = (close(fd) >= 0) && result;
#endif

	return result;
}


int DQEmgData::store(const char *filename)
{
	int fd, result;;
	setFileName(filename);

#if defined( _WIN32 )
	fd = _open(filename_, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, 0666);
#else
	fd = open(filename_, O_CREAT|O_TRUNC|O_WRONLY, 0666);
#endif
	if (fd < 0)
		return 0;

	result = store_(fd);

#if defined( _WIN32 )
	result = (_close(fd) >= 0) && result;
#else
	result = (close(fd) >= 0) && result;
#endif

	return result;
}

int DQEmgData::loadContractionDataHeader_(int fd)
{
	char ibuf[MAX_VENDOR_ID_LENGTH + 1];
	char version;
	DQEmgData32bit int32buffer;
	long i;
	int acquisitionTime32bit;
	off_t firstDataOffset = 0;
	off_t curOffset;
	size_t l;
	int s;

	/* read in the DQEMG tag */
	l = FILE_TAG_LEN;
	s = read(fd, ibuf, l);

	if (s != FILE_TAG_LEN)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (memcmp(ibuf, FILE_TAG, FILE_TAG_LEN) != 0)
	{
		setError(ERR_NOT_DQEMG_FILE);
		return 0;
	}

	/**
	 * set up default values for all extended (optional)
	 * header values
	 */
	setContractionDefaultValues_();


	/* read out vendor id */
	l = MAX_VENDOR_ID_LENGTH;
	s = read(fd, ibuf, l);
	if (s != MAX_VENDOR_ID_LENGTH)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	ibuf[MAX_VENDOR_ID_LENGTH] = 0;
	i = MAX_VENDOR_ID_LENGTH - 1;
	while (i >= 0 && ibuf[i] == ' ')
	{
		ibuf[i] = 0;
		i--;
	}
	setVendorIdentifier(ibuf);


	/* acquisition time */
	l = sizeof(acquisitionTime32bit);
	s = read(fd, &acquisitionTime32bit, l);
	if (l != (unsigned int) s)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	acquisitionTime_ = 0 + acquisitionTime32bit;


	/* file version */
	l = sizeof(version);
	s = read(fd, &version, l);
	if (s != sizeof(version))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	fileVersion_ = (int) version;


	/* num channels */
	l = sizeof(numChannels_);
	s = read(fd, &numChannels_, l);
	if (s != sizeof(numChannels_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	if (numChannels_ > 0)
	{
		/**
		 * load the offsets into the structure in context
		 */
		context_->dataOffset32bit_ = new DQEmgData32bit[numChannels_];
		
		l = sizeof(DQEmgData32bit) * numChannels_;
		s = read(fd, context_->dataOffset32bit_, l);
		if (s != (int) (sizeof(DQEmgData32bit) * numChannels_))
		{
			setError(ERR_ERRNO);
			return 0;
		}
		
		firstDataOffset = context_->dataOffset32bit_[0];
		for (i = 1; i < numChannels_; i++)
		{
			if (firstDataOffset > (off_t) context_->dataOffset32bit_[i])
				firstDataOffset = (off_t) context_->dataOffset32bit_[i];
		}
	} else
	{
		/**
		 * if there were no channels, we can use the size
		 * of the file (which is just the size of the header)
		 * to calculate the firstDataOffset variable
		 */
		off_t tmp;

		/** save current loc, calc EOF, come back */
		tmp = lseek(fd, 0, SEEK_CUR);
		firstDataOffset = lseek(fd, 0, SEEK_END);
		(void) lseek(fd, tmp, SEEK_SET);
	}

	if ( ! loadVariableStringValue_(fd, &operatorDescription_, this) )
		return 0;

	if ( ! loadVariableStringValue_(fd, &subjectDescription_, this) )
		return 0;

	if ( ! loadVariableStringValue_(fd, &muscleDescription_, this) )
		return 0;

	if ( ! loadVariableStringValue_(fd, &generalDescription_, this) )
		return 0;

	/**
	 * The information from this point on is not guaranteed to be
	 * in all files.  We therefore check the offset at each
	 * step to be sure that we have not exceeded the header length.
	 *
	 * Default values are provided at the top of the function so
	 * we already know these fields are populated with a sensible value
	 *
	 * Note that although these values are "optional", they are in
	 * a fixed order, so for later values to be present, earlier
	 * values must exist
	 */

	/* study flags */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	studyFlags_ = int32buffer;

	/* subject date of birth */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;

	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	subjectJulianDateOfBirth_ = int32buffer;


	/* subject gender */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	s = read(fd, &subjectGender_, sizeof(subjectGender_));
	if (s != sizeof(subjectGender_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* muscle laterality */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	s = read(fd, &muscleSide_, sizeof(muscleSide_));
	if (s != sizeof(muscleSide_))
	{
		setError(ERR_ERRNO);
		return 0;
	}



	/* Contraction Type */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	s = read(fd, &contractionType_, sizeof(contractionType_));
	if (s != sizeof(contractionType_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* subject ID */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	if ( ! loadVariableStringValue_(fd, &subjectID_, this) )
		return 0;

	// Waveforms are stored in the following order:
	// 1)  Cmap - cmapStored_
	// 2)  mSmup - mSmupStored_
	// 3)  Smups
	// 4)  Cmaps - Only for new versions

	// If the CMAP or mSMUP flag is 0, then it was not stored
	// These 2 flags, along with NumChannels, determine how many
	// SMUP's are in the mSMUP

	/* CMAP Stored Flag */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	cmapStored_ = int32buffer;

	/* mSMUP Stored Flag */
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	if (curOffset >= firstDataOffset)   goto END;
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	mSmupStored_ = int32buffer;
	
	// Read back subject age instead of date of birth
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	subjectAge_ = int32buffer;
		
		// Read the number of stored cmaps	//ABDSAVE
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	numCmapStored_ = int32buffer;

		// Read the number of stored cmaps	//mauto
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	numAutoSmupStored_ = int32buffer;

	/*MarzTest*/
	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	indexOfFirstSmup_ = int32buffer;

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	indexOfSecondSmup_ = int32buffer;

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	indexOfThirdSmup_ = int32buffer;

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	indexOfBaseSmup_ = int32buffer;

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &FreqOfFirstSmup_, sizeof(FreqOfFirstSmup_));
	if (s != sizeof(FreqOfFirstSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &FreqOfSecondSmup_, sizeof(FreqOfSecondSmup_));
	if (s != sizeof(FreqOfSecondSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &FreqOfThirdSmup_, sizeof(FreqOfThirdSmup_));
	if (s != sizeof(FreqOfThirdSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	curOffset = lseek(fd, 0, SEEK_CUR);
	if (curOffset < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}
	s = read(fd, &FreqOfBaseSmup_, sizeof(FreqOfBaseSmup_));
	if (s != sizeof(FreqOfBaseSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	/*MarzTest*/



END:
	/* return success */
	return 1;
}

int DQEmgData::storeContractionDataHeader_(int fd)
{
	char obuf[MAX_VENDOR_ID_LENGTH];
	char version;
	int s;
	DQEmgData32bit acquisitionTime32bit;
	DQEmgData32bit int32buffer;
	unsigned long i;
	size_t l;

	version = (char) DEFAULT_VERSION;
	
	/* write out the DQEMG tag */
	l = FILE_TAG_LEN;
	s = write(fd, FILE_TAG, l);
	if (l != (unsigned int) s)
	{
		setError(ERR_ERRNO);
		return 0;
	}


	/* write out vendor id */
	for (i = 0; i < MAX_VENDOR_ID_LENGTH && vendorIdentifier_[i]; i++)
	{
		obuf[i] = vendorIdentifier_[i];
	}
	while (i < MAX_VENDOR_ID_LENGTH)
	{
		obuf[i++] = ' ';
	}
	l = MAX_VENDOR_ID_LENGTH;
	s = write(fd, obuf, l);
	if (l != (unsigned int) s)
	{
		setError(ERR_ERRNO);
		return 0;
	}


	/* time of acquisition */
	acquisitionTime32bit = (int) acquisitionTime_;
	l = sizeof(acquisitionTime32bit);
	s = write(fd, &acquisitionTime32bit, l);
	if (l != (unsigned int) s)
	{
		setError(ERR_ERRNO);
		return 0;
	}


	/* file version */
	l = sizeof(version);
	s = write(fd, &version, l);
	if (l != (unsigned int) s)
	{
		setError(ERR_ERRNO);
		return 0;
	}


	/* num channels */
	l = sizeof(numChannels_);
	s = write(fd, &numChannels_, l);
	if (l != (unsigned int) s)
	{
		setError(ERR_ERRNO);
		return 0;
	}


	/**
	 * remember where the offsets will go so that we can come
	 * back and record them later, if there are any channels
	 * to record
	 */
	context_->headerChannelOffsetLocation_ = lseek(fd, 0, SEEK_CUR);
	if (numChannels_ > 0)
	{
		context_->dataOffset32bit_ = new DQEmgData32bit[numChannels_];
		memset(context_->dataOffset32bit_,
				0, numChannels_ * sizeof(DQEmgData32bit));

		/**
		 * skip the values for now.
		 */
		l = numChannels_ * sizeof(DQEmgData32bit);
		s = lseek(fd, l, SEEK_CUR);
		if (s < 0)
		{
			setError(ERR_ERRNO);
			return 0;
		}
	}


	if ( ! storeVariableStringValue_(fd, operatorDescription_, this) )
		return 0;

	if ( ! storeVariableStringValue_(fd, subjectDescription_, this) )
		return 0;

	if ( ! storeVariableStringValue_(fd, muscleDescription_, this) )
		return 0;

	if ( ! storeVariableStringValue_(fd, generalDescription_, this) )
		return 0;


	/**
	 * The information from this point on is not guaranteed to be
	 * in all files, so default values must be provided
	 */


	/* study flags */
	int32buffer = (DQEmgData32bit) studyFlags_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* subject date of birth */
	int32buffer = (DQEmgData32bit) subjectJulianDateOfBirth_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}


	/* subject gender */
	s = write(fd, &subjectGender_, sizeof(subjectGender_));
	if (s != sizeof(subjectGender_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* muscle laterality */
	s = write(fd, &muscleSide_, sizeof(muscleSide_));
	if (s != sizeof(muscleSide_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

#ifndef		RESTRICT_TO_BASE_VERSION

	/* Contraction Type */
	s = write(fd, &contractionType_, sizeof(contractionType_));
	if (s != sizeof(contractionType_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* subject ID */
	if ( ! storeVariableStringValue_(fd, subjectID_, this) )
		return 0;

	/* cmapStored_ */
	int32buffer = (DQEmgData32bit) cmapStored_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* smupStored */
	int32buffer = (DQEmgData32bit) mSmupStored_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/** Simply store subject age in years */
	int32buffer = (DQEmgData32bit) subjectAge_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* numCmapsStored */	//ABDSAVE
	int32buffer = (DQEmgData32bit) numCmapStored_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/* numAutoSmupStored */	//mauto
	int32buffer = (DQEmgData32bit) numAutoSmupStored_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	/*MarzTest*/
	int32buffer = (DQEmgData32bit) indexOfFirstSmup_;
	s = write(fd, &int32buffer, sizeof(int32buffer)); 
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	int32buffer = (DQEmgData32bit) indexOfSecondSmup_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	int32buffer = (DQEmgData32bit) indexOfThirdSmup_;
	s = write(fd, &int32buffer, sizeof(int32buffer)); 
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	int32buffer = (DQEmgData32bit) indexOfBaseSmup_;
	s = write(fd, &int32buffer, sizeof(int32buffer)); 
	if (s != sizeof(int32buffer))
	{
		setError(ERR_ERRNO);
		return 0;
	}
		
	s = write(fd, &FreqOfFirstSmup_, sizeof(FreqOfFirstSmup_)); 
	if (s != sizeof(FreqOfFirstSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	s = write(fd, &FreqOfSecondSmup_, sizeof(FreqOfSecondSmup_));
	if (s != sizeof(FreqOfSecondSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	s = write(fd, &FreqOfThirdSmup_, sizeof(FreqOfThirdSmup_)); 
	if (s != sizeof(FreqOfThirdSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}

	s = write(fd, &FreqOfBaseSmup_, sizeof(FreqOfBaseSmup_)); 
	if (s != sizeof(FreqOfBaseSmup_))
	{
		setError(ERR_ERRNO);
		return 0;
	}
	/*MarzTest*/

#endif /* RESTRICT_TO_BASE_VERSION */

#	ifdef FORWARD_COMPAT_TEST
	if ( ! storeVariableStringValue_(fd,
		"FORWARD COMPATIBILITY TEST DATA IN HEADER",
		this) )
		return 0;
#	endif

	return 1;
}

int DQEmgData::updateContractionDataHeader_(int fd)
{
	int s;

	/**
	 * get back to where we want to put the offsets
	 */
	s = lseek(fd, context_->headerChannelOffsetLocation_, SEEK_SET);
	if (s < 0)
	{
		setError(ERR_ERRNO);
		return 0;
	}

	if (numChannels_ > 0)
	{

		s = write(fd, context_->dataOffset32bit_,
				sizeof(DQEmgData32bit) * numChannels_);
		if (s != (int) (sizeof(DQEmgData32bit) * numChannels_))
		{
			setError(ERR_ERRNO);
			return 0;
		}
	}
	return 1;
}

int
DQEmgData::loadVariableStringValue_(
		int fd,
		char **string,
		DQEmgData *caller
	)
{
	int s;
	unsigned short length;
	size_t l;

	l = sizeof(unsigned short);
	s = read(fd, &length, l);
	if (l != (unsigned int) s)
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if ((*string) != NULL)
		delete [] (*string);

	if (length == 0)
	{
		(*string) = NULL;
		return 1;
	}

	(*string) = new char[length + 1];
	(*string)[length] = 0;

	s = read(fd, (*string), length);
	if (s != length)
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	return 1;
}

int
DQEmgData::storeVariableStringValue_(
		int fd,
		char *string,
		DQEmgData *caller
	)
{
	int s, l;
	size_t ll;
	unsigned short length;

	if (string != NULL)
		l = (int) strlen(string);
	else
		l = 0;

	if (l > (int) USHRT_MAX)
	{
		caller->setError(ERR_DATA_OVERFLOW);
		return 0;
	}

	length = (unsigned short) l;

	ll = sizeof(unsigned short);
	s = write(fd, &length, ll);
	if (ll != (unsigned int) s)
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	if (length != 0)
	{
		s = write(fd, string, length);
		if (s != l)
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
	}

	return 1;
}

int DQEmgChannelData::loadChannelData_(int fd, DQEmgData *caller)
{
	DQEmgData32bit int32buffer;

	/**
	 * load data in the historically correct length (4 bytes)
	 */
	DQEmgData32bit firstDataOffset;
	size_t s;


	/* data offset */
	s = read(fd, &firstDataOffset, sizeof(firstDataOffset));
	if (s != sizeof(firstDataOffset))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* channel number */
	s = read(fd, &channelNumber_, sizeof(channelNumber_));
	if (s != sizeof(channelNumber_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* hipass cutoff */
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	hipassCutoff_ = int32buffer;

	/* lopass cutoff */
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	lopassCutoff_ = int32buffer;


	/* encoding */
	s = read(fd, &encoding_, sizeof(short));
	if (s != sizeof(short))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* first check if we can handle this encoding */
	if (((encoding_ & 0xFF00) >> 8) != 0)
	{
		caller->setError(ERR_UNKNOWN_ENCODING);
		return 0;
	}


	/*
	 * sampling rate -- for backwards compatibility we read
	 * from a long here and cast, and then overwrite with a
	 * float value if we see one later
	 */
	{
		s = read(fd, &int32buffer, sizeof(int32buffer));
		if (s != sizeof(int32buffer))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		samplingRate_ = (float) int32buffer;
	}

	/* num samples */
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	numSamples_ = (long) int32buffer;

	/* elapsed time */
	s = read(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	elapsedTime_ = (long) int32buffer;

	/* units */
	if ( ! DQEmgData::loadVariableStringValue_(
				fd,
				&unitDescription_,
				caller
			) )
		return 0;


	/* scale */
	s = read(fd, &scale_, sizeof(scale_));
	if (s != sizeof(scale_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if (caller->getFileVersion() != 0x00) //MABD		
	{
		if ( ! DQEmgData::loadVariableStringValue_(
				fd, &channelDescription_,
				caller) )
			return 0;
	}


	/**
	* every operation from here in needs to be prefaced by this 
	 * so that we can read older files
	 */
	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;


	// if ContractionType = Evoked
	if (caller->isEvoked())
	{

		/* get negativePeakArea_ */
		s = read(fd, &negativePeakArea_, sizeof(negativePeakArea_));
		if (s != sizeof(negativePeakArea_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		/* get peakToPeakAmplitude */
		s = read(fd, &peakToPeakAmplitude_,
				sizeof(peakToPeakAmplitude_));
		if (s != sizeof(peakToPeakAmplitude_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		/* get negativePeakAmplitude */
		s = read(fd, &negativePeakAmplitude_,
				sizeof(negativePeakAmplitude_));
		if (s != sizeof(negativePeakAmplitude_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		/* onset position */
		s = read(fd, &onsetPos_, sizeof(onsetPos_));
		if (s != sizeof(onsetPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		/* positivePeak position */
		s = read(fd, &positivePeakPos_, sizeof(positivePeakPos_));
		if (s != sizeof(positivePeakPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		/* negativePeak position */
		s = read(fd, &negativePeakPos_, sizeof(negativePeakPos_));
		if (s != sizeof(negativePeakPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		/* end position */
		s = read(fd, &endPos_, sizeof(endPos_));
		if (s != sizeof(endPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
	}

	if (caller->getFileVersion() == 0x00) //MABD
	{
		//channelDescription 
		if ( ! DQEmgData::loadVariableStringValue_(
				fd, &channelDescription_,
				caller) )
			return 0;
	}

	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;
	/* isValid - added for MPS smup waveforms */
	s = read(fd, &isValid_, sizeof(isValid_));
	if (s != sizeof(isValid_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;
	/* Number of Smups in the mSmup */
	s = read(fd, &numSmupsInAvg_, sizeof(numSmupsInAvg_));
	if (s != sizeof(numSmupsInAvg_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/////////// Extras for Evoked responses //////////////
	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;
	/* duration_ */
	s = read(fd, &duration_, sizeof(duration_));
	if (s != sizeof(duration_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;
	/* negativePeakDuration_ */
	s = read(fd, &negativePeakDuration_, sizeof(negativePeakDuration_));
	if (s != sizeof(negativePeakDuration_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;
	/* area_ */
	s = read(fd, &area_, sizeof(area_));
	if (s != sizeof(area_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;
	/* peak Onset position */
	s = read(fd, &peakOnsetPos_, sizeof(peakOnsetPos_));
	if (s != sizeof(peakOnsetPos_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
		goto END_OF_HEADER;

	/* peak End position */
	s = read(fd, &peakEndPos_, sizeof(peakEndPos_));
	if (s != sizeof(peakEndPos_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	if (caller->getFileVersion() != 0x00)
	{
		 if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER; //MABD

		/* index */
		s = read(fd, &int32buffer, sizeof(int32buffer));
		if (s != sizeof(int32buffer))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		index_ = int32buffer;

		/**
		 * sampling rate (again) -- if there is a floating point
		 * value for sampling rate, it will be here
		 */
		if (lseek(fd, 0, SEEK_CUR) >= (off_t) firstDataOffset)
			goto END_OF_HEADER;
		s = read(fd, &samplingRate_, sizeof(samplingRate_));
		if (s != sizeof(samplingRate_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
	}

END_OF_HEADER:
	/* skip any remaining fields and go to the data */
	if ( lseek(fd, (off_t) firstDataOffset, SEEK_SET) < 0)
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	/**
	 * read in all the data values, based on the encoding
	 */
	if ((encoding_ & 0x00FF) == 2)
	{
		dataShortSample_ = new short[numSamples_];
		s = read(fd, dataShortSample_, numSamples_ * sizeof(short));
		if (s != (size_t) (numSamples_ * sizeof(short)))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		
	} else if ((encoding_ & 0x00FF) == 4)
	{
		dataLongSample_ = new long[numSamples_];
		s = read(fd, dataLongSample_, numSamples_ * sizeof(long));
		if (s != (size_t) (numSamples_ * sizeof(long)))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		
	} else
	{
		caller->setError(ERR_UNKNOWN_ENCODING);
		return 0;
	}

	return 1;
}

int DQEmgChannelData::storeChannelData_(int fd, DQEmgData *caller)
{
	DQEmgData32bit int32buffer;
	off_t startOfHeader, startOfData = 0;
	size_t s;


	/**
	 * save where we are so we can come back when we know
	 * where the data will start
	 */
	startOfHeader = lseek(fd, 0, SEEK_CUR);
	if (startOfHeader < 0)
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* data offset */
	int32buffer = (DQEmgData32bit) startOfData;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* channel number */
	s = write(fd, &channelNumber_, sizeof(channelNumber_));
	if (s != sizeof(channelNumber_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* hipass cutoff */
	int32buffer = hipassCutoff_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* lopass cutoff */
	int32buffer = lopassCutoff_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	/* encoding */
	s = write(fd, &encoding_, sizeof(short));
	if (s != sizeof(short))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	/* sampling rate -- as a long for backwards compatibility */
	{
		int32buffer = (unsigned long) samplingRate_;
		s = write(fd, &int32buffer, sizeof(int32buffer));
		if (s != sizeof(int32buffer))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
	}

	/* num samples */
	int32buffer = numSamples_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* elapsed time */
	int32buffer = elapsedTime_;
	s = write(fd, &int32buffer, sizeof(int32buffer));
	if (s != sizeof(int32buffer))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* units */
	if ( ! DQEmgData::storeVariableStringValue_(
			fd, unitDescription_,
			caller) )
		return 0;


	/* scale */
	s = write(fd, &scale_, sizeof(scale_));
	if (s != sizeof(scale_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	if (caller->getFileVersion() != 0x00) //MABD
	{
		/* channelDescription */
		if ( ! DQEmgData::storeVariableStringValue_(
				fd, channelDescription_,
				caller) )
			return 0;
	}

#ifndef		RESTRICT_TO_BASE_VERSION

	/* if Contraction Type is "evoked" */
	if (caller->isEvoked())
	{
		/* Negative peak area */
		s = write(fd, &negativePeakArea_, sizeof(negativePeakArea_));
		if (s != sizeof(negativePeakArea_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		/* peak to peak amplitude*/
		s = write(fd, &peakToPeakAmplitude_,
				sizeof(peakToPeakAmplitude_));
		if (s != sizeof(peakToPeakAmplitude_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		
		/* negative peak amplitude */
		s = write(fd, &negativePeakAmplitude_,
				sizeof(negativePeakAmplitude_));
		if (s != sizeof(negativePeakAmplitude_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		/* onset position */
		s = write(fd, &onsetPos_, sizeof(onsetPos_));
		if (s != sizeof(onsetPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		/* positive peak position */
		s = write(fd, &positivePeakPos_, sizeof(positivePeakPos_));
		if (s != sizeof(positivePeakPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		/* negative peak position */
		s = write(fd, &negativePeakPos_, sizeof(negativePeakPos_));
		if (s != sizeof(negativePeakPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		/* end position */
		s = write(fd, &endPos_, sizeof(endPos_));
		if (s != sizeof(endPos_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
	}
		
	if (caller->getFileVersion() == 0x00) //MABD
	{
		/* channelDescription */
		if ( ! DQEmgData::storeVariableStringValue_(
				fd, channelDescription_,
				caller) )
			return 0;
	}

	/* Store validity of SMUP waveform - added for MPS */
	s = write(fd, &isValid_, sizeof(isValid_));
	if (s != sizeof(isValid_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

	/* Number of Smups in mSmup */
	s = write(fd, &numSmupsInAvg_, sizeof(numSmupsInAvg_));
	if (s != sizeof(numSmupsInAvg_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}



	/////////// Evoked potential statistics //////////////
	s = write(fd, &duration_, sizeof(duration_));
	if (s != sizeof(duration_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	s = write(fd, &negativePeakDuration_,
			sizeof(negativePeakDuration_));
	if (s != sizeof(negativePeakDuration_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	s = write(fd, &area_, sizeof(area_));
	if (s != sizeof(area_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	s = write(fd, &peakOnsetPos_, sizeof(peakOnsetPos_));
	if (s != sizeof(peakOnsetPos_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	s = write(fd, &peakEndPos_, sizeof(peakEndPos_));
	if (s != sizeof(peakEndPos_))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

		
	if (caller->getFileVersion() != 0x00) //MABD
	{
		int32buffer = index_;
		s = write(fd, &int32buffer, sizeof(int32buffer)); //MABD
		if (s != sizeof(int32buffer))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
		
		/* sampling rate as a float */
		s = write(fd, &samplingRate_, sizeof(samplingRate_));
		if (s != sizeof(samplingRate_))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}
	}

#endif /* RESTRICT_TO_BASE_VERSION */

	/**
	 * THIS MUST BE AFTER ALL DATA VALUES ARE STORED
	 */

	/* remember where the data is about to start */
	startOfData = lseek(fd, 0, SEEK_CUR);
	if (startOfData < 0)
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	/* go back and update the header offset value */
	if ( lseek(fd, startOfHeader, SEEK_SET) < 0 )
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}
	s = write(fd, &startOfData, sizeof(off_t));
	if (s != sizeof(off_t))
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}

#	ifdef FORWARD_COMPAT_TEST
	if ( ! DQEmgData::storeVariableStringValue_(
				fd,
				"-- SECOND BLOCK OF FORWARD COMPATIBILITY TEST DATA --",
				caller) )
		return 0;
#	endif

	/* now come back and actually write the data */
	if ( lseek(fd, startOfData, SEEK_SET) < 0 )
	{
		caller->setError(ERR_ERRNO);
		return 0;
	}


	/* write out all the data values */
	if ((encoding_ & 0x00FF) == 2)
	{
		if (dataShortSample_ == NULL)
		{
			if ( ! convertDataToShort_() )
			{
				caller->setError(ERR_DATA_OVERFLOW);
				return 0;
			}
		}
		s = write(fd, dataShortSample_, numSamples_ * sizeof(short));
		if (s != (size_t) (numSamples_ * sizeof(short)))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

	} else if ((encoding_ & 0x00FF) == 4)
	{
		if (dataLongSample_ == NULL)
		{
			if ( ! convertDataToLong_() )
			{
				caller->setError(ERR_DATA_OVERFLOW);
				return 0;
			}
		}
		s = write(fd, dataLongSample_, numSamples_ * sizeof(long));
		if (s != (size_t) (numSamples_ * sizeof(long)))
		{
			caller->setError(ERR_ERRNO);
			return 0;
		}

	} else
	{
		caller->setError(ERR_UNKNOWN_ENCODING);
		return 0;
	}

	return 1;
}



int DQEmgData::load_(int fd)
{
	unsigned long i;


	setError(0);
	if ( ! loadContractionDataHeader_(fd) )
		return 0;

	if (numChannels_ > 0)
	{
		data_ = new DQEmgChannelData *[ numChannels_ ];
		memset(data_, 0, sizeof(DQEmgChannelData *) * numChannels_);

		for (i = 0; i < numChannels_; i++)
		{

			data_[i] = new DQEmgChannelData();

			if ( lseek(fd, context_->dataOffset32bit_[i], SEEK_SET ) < 0)
			{
				setError(ERR_ERRNO);
				goto FAIL;
			}

			if ( ! data_[i]->loadChannelData_(fd, this) )
				goto FAIL;
		}
	} else
	{
		data_ = NULL;
	}

	return 1;

FAIL:
	/* clean up, and then return false */
	int j;
	for (j = 0; j <= (int) i; j++)
	{
		delete data_[j];
	}
	delete [] data_;
	data_ = NULL;
	return 0;
}

int DQEmgData::store_(int fd)
{
	off_t lseekOffset;
	DQEmgChannelData *curChannel;
	unsigned long i;


	setError(0);
	if ( ! storeContractionDataHeader_(fd) )
		return 0;

	if (numChannels_ > 0)
	{
		for (i = 0; i < numChannels_; i++)
		{
			/**
			 * load offset using an off_t, and then check
			 * that the loaded value does not overflow
			 * a 4-byte quantity, or we will not be able
			 * to read this file back in, due to our
			 * historical size limits
			 */
			lseekOffset = lseek(fd, 0, SEEK_CUR);
			if (lseekOffset > LONG_MAX)
			{
				setError(ERR_FILE_TOO_LARGE);
				return 0;
			}
			context_->dataOffset32bit_[i] = lseekOffset;
			if (context_->dataOffset32bit_[i] < 0)
			{
				setError(ERR_ERRNO);
				return 0;
			}
			curChannel = data_[i];
			if ( ! curChannel->storeChannelData_(fd, this) )
				return 0;
		}
	}

	if ( ! updateContractionDataHeader_(fd) )
		return 0;

	return 1;
}


void DQEmgData::setError(int id)
{
	if (sMaxError_ < 0)
	{
		int i;
		for (i = 0; sErrors_[i] != NULL; i++)
			;
		sMaxError_ = i;
	}

	if (id >= sMaxError_)
	{
		fprintf(stderr, "Internal error setting errid %d\n", id);
	}

	errorId_ = id;
}

const char *DQEmgData::getErrorMessage(int errid) const
{
	/* if we aren't initialized, there is no error */
	if (sMaxError_ < 0) return sErrors_[0];

	if (errid < 0 || errid >= sMaxError_)
		return "Error Number Out of Bounds";

	if (errid == 1)
		return strerror(errno);

	return sErrors_[errid];
}

int DQEmgData::getError() const
{
	return errorId_;
}

#define TAGLEN  20

#ifdef DQD_DEBUG_DUMP
void DQEmgChannelData::dump(FILE *fp, int dumpSamples)
{
	unsigned long numSamples;
	float samplingRate;
	float scale;
	short *data;

	fprintf(fp, "        %-*s: %d\n",
				TAGLEN, "Channel Number",
				getChannelNumber());

	fprintf(fp, "        %-*s: %ld\n",
				TAGLEN, "HighPass Cutoff",
				getHighPassCutoff());

	fprintf(fp, "        %-*s: %ld\n",
				TAGLEN, "LowPass Cutoff",
				getLowPassCutoff());

	fprintf(fp, "        %-*s: %ld\n",
				TAGLEN, "Elapsed Time",
				getElapsedTime());

	fprintf(fp, "        %-*s: [%s]\n",
				TAGLEN, "Channel Description",
				getChannelDescription());

	fprintf(fp, "        %-*s: [%s]\n",
				TAGLEN, "Units",
				getUnitDescription());

	data = getDataAsShort(
				&numSamples,
				&samplingRate,
				&scale
			);

	fprintf(fp, "        %-*s: %ld\n",
				TAGLEN, "Num Samples",
				numSamples);

	fprintf(fp, "        %-*s: %g\n",
				TAGLEN, "Sampling Rate",
				samplingRate);

	fprintf(fp, "        %-*s: %f\n",
				TAGLEN, "Scale",
				scale);

	fprintf(fp, "        %-*s: %f\n",
				TAGLEN, "negativePeakArea",
				getEvokedResponseNegativePeakArea());

	fprintf(fp, "        %-*s: %f\n",
				TAGLEN, "peakToPeakAmplitude",
				getEvokedResponsePeakToPeakAmplitude());

	fprintf(fp, "        %-*s: %f\n",
				TAGLEN, "negativePeakAmplitude",
				getEvokedResponseNegativePeakAmplitude());

	fprintf(fp, "        %-*s: %d\n",
				TAGLEN, "Onset Pos",
				getEvokedResponseOnsetPos());

	fprintf(fp, "        %-*s: %d\n",
				TAGLEN, "Pos-Peak Pos",
				getEvokedResponsePositivePeakPos());

	fprintf(fp, "        %-*s: %d\n",
				TAGLEN, "Neg-Peak Pos",
				getEvokedResponseNegativePeakPos());

	fprintf(fp, "        %-*s: %d\n",
				TAGLEN, "End Pos",
				getEvokedResponseEndPos());

	if (dumpSamples > 0)
	{
		unsigned long i;

		fprintf(fp, "        >>>> Begin Data Samples (%f)\n", scale);
		for (i = 0; i < numSamples; i++)
		{
			fprintf(fp, "            %f\n", (((float) data[i]) * scale));
		}
		fprintf(fp, "        <<<< End Data Samples\n");
	}
}

void DQEmgData::dump(FILE *fp, int channelSelection, int dumpChannelDataFlags)
{
	DQEmgChannelData *channel;
	time_t acquisitionTime;
	int gender, side, contraction;
	unsigned long year, month, day;
	unsigned char i;

	fprintf(fp, "%-*s: %d\n",
				TAGLEN, "Version",
				getFileVersion());

	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "Vendor",
				getVendorIdentifier());

	acquisitionTime = getAcquisitionTime();
	fprintf(fp, "%-*s: %s",
				TAGLEN, "Acquisition Time",
				ctime(&acquisitionTime));


	fprintf(fp, "%-*s: %d\n",
				TAGLEN, "Num Channels",
				(int) getNumChannels());


	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "Operator Name",
				getOperatorDescription());

	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "Subject Name",
				getSubjectDescription());

	if (getSubjectJulianDateOfBirth() == 0)
	{
		fprintf(fp, "%-*s: <unknown>\n",
				TAGLEN, "Subject D.O.B.");
	} else
	{
		getSubjectDateOfBirth(&year, &month, &day);
		fprintf(fp, "%-*s: %ld-%ld-%ld\n",
				TAGLEN, "Subject D.O.B.",
				day, month, year);
	}

	fprintf(fp, "%-*s: %s\n",
				TAGLEN, "Is New Operator? ",
				isNewOperator() ? "YES" : "NO");

	fprintf(fp, "%-*s: %s\n",
				TAGLEN, "Is New Patient? ",
				isNewPatient() ? "YES" : "NO");

	fprintf(fp, "%-*s: %s\n",
				TAGLEN, "Is New Muscle? ",
				isNewMuscle() ? "YES" : "NO");

	gender = getSubjectGender();
	fprintf(fp, "%-*s: %s\n",
				TAGLEN, "Subject Gender",
				(gender == 0) ? "<not recorded>" :
					(gender == 1) ? "FEMALE" :
						(gender == 2) ? "MALE" : "ERROR");


	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "Muscle",
				getMuscleDescription());

	side = getMuscleSide();
	fprintf(fp, "%-*s: %s\n",
				TAGLEN, "Muscle Side",
				(side == 0) ? "<not recorded>" :
					(side == 1) ? "LEFT" :
						(side == 2) ? "RIGHT" : "ERROR");

	contraction = getContractionType();
	fprintf(fp, "%-*s: %s\n",
				TAGLEN, "Contraction Type",
				(contraction == 0) ? "<not recorded>" :
					(contraction == 1) ? "MVC" :
						(contraction == 2) ? "Spontaneous Activity" :
						(contraction == 3) ? "Sub-Maximal" :
						(contraction == 4) ? "Evoked" :"ERROR");

	// Added for MPS
	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "is CMAP Stored :",
				isCmapStored() ? "TRUE" : "FALSE");

	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "is SMUP Stored :",
				is_mSmupStored() ? "TRUE" : "FALSE");

	fprintf(fp, "%-*s: [%s]\n",
				TAGLEN, "General",
				getGeneralDescription());

	if (dumpChannelDataFlags != 0)
	{
		int dumpSamples;

		dumpSamples = (dumpChannelDataFlags & DUMP_SAMPLES) != 0 ? 1 : 0;
		
		for (i = 0; i < getNumChannels(); i++)
		{
			/** if we only want one channel, just process that one */
			if ((channelSelection < 0) || (i == channelSelection))
			{
				channel = getChannel(i);
				fprintf(fp, "    %s: %d\n", "Channel", i);
				channel->dump(fp, dumpSamples);
			}
		}
	}
}
#endif


/**
 * DLL specific code
 */

#ifdef DQD_DLL
BOOL APIENTRY DllMain(
		HANDLE hModule, 
		DWORD  ul_reason_for_call, 
		LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

#endif
