
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
 * This file defines the DQEmgData structure and interfaces
 *
 * $Id: DQEmgData.h 166 2016-11-28 15:12:43Z stashuk $
 */

#ifndef __EMG_DATA_DEFINITION_HEADER__
#define __EMG_DATA_DEFINITION_HEADER__

#include <time.h>

#if defined( _WIN32 )
# if defined( DQD_DYNAMIC )
#  define DQD_EXPORT __declspec(dllexport)
# elif defined( DQD_USE_DYNAMIC )
#  define DQD_EXPORT __declspec(dllimport)
# elif defined( DQD_STATIC )
#  define DQD_EXPORT
# else
#  define DQD_EXPORT
# endif
#else
#  define DQD_EXPORT
#endif

/**
 **	Forward declarations for internal classes
 **/
class DQEmgData;
class DQEmgChannelData;

class DQEmgDataContext;

/**	forward declaration for parent/child reference **/
class DQEmgChannelData;

/**
 **	Error list related constants
 **/
#define ERR_ERRNO					1
#define ERR_NOT_DQEMG_FILE			2
#define ERR_INCOMPATIBLE_VERSION	3
#define ERR_DATA_OVERFLOW			4
#define ERR_BAD_ENCODING			5
#define ERR_UNKNOWN_ENCODING		6
#define ERR_FILE_TOO_LARGE			7


/**
 **	Constants defining field sizes
 **/
#define	MAX_VENDOR_ID_LENGTH	8


/**
 **	API for hidden implementation classes which will
 **	actually write to the file.
 **
 **	These class encapsulates all of the information
 **	about the implementation which needs to be
 **	known to calling code.
 **/

/**
CLASS
	DQEmgData

	An entire contraction data set.  A contraction
	is made up of a global definition and a set
	of channel data structures.  Each channel data
	structure will store the values recorded by a
	given instrument (EMG, temperature, force, etc).

	<p>
	A contraction is the set of all of the channels
	recorded at the same time, plus the following
	information in the header to make the data
	useful for study purposes:
		<ul>
		<li>
		operatorName/Physician
			<ul>
			<li>
			This is the operator of the EMG collection
			equipment.  The operator field should be
			filled in a standard way to allow other
			contractions and studies by the same operator
			to be identified.
			</li>
			</ul>
		</li>
		<li>
		Subject
			<ul>
			<li>
			This will be a unique identifier for the
			subject.  Note that this is a freeform text
			field, so any identification number as well
			as, or instead of, the subject name may be
			included here.
			</li>
			</ul>
		</li>
		<li>
		Muscle
			<ul>
			<li>
			This is the muscle identification from which
			the contraction was recorded.  Laterality
			will identify the side of the body, allowing
			the identification of the particular muscle
			on the subject.
			</li>
			</ul>
		</li>
		<li>
		Muscle Side
			<ul>
			<li>
			This is the muscle laterality.
			</li>
			</ul>
		</li>

		<li>
		Contraction Type
			<ul>
			<li>
			AcquireEmg stores the contraction type of the
			signal. The voluntary signal can be one
			of the three following types: <br>
				o MVC   <br>
				o Spontaneous Activity <br>
				o Sub-Maximal   <br>
			Whereas the evoked signal has the type
			o Evoked.
			</li>
			</ul>
		</li>

		<li>
		General textual note information
			<ul>
			<li>
			This field is intended for the storage
			of any general textual information the physician
			wishes to attach to this contraction.
			<p>
			Note that this field may be further delimited
			by the user software in order to include any
			notational attributes which may be available
			in the program.
			</li>
			</ul>
		</li>
		<li>
		Vendor of recording equipment/software
			<ul>
			<li>
			This field is intended to allow the identification
			of the company and particular software platform
			which created the file.  This will primarily
			be of use when doing historical studies, where
			the particular platform may become a relevant
			factor.
			</li>
			</ul>
		</li>
		<li>
		Date & Time of the contraction
			<ul>
			<li>
			Date & time of the study, stored in seconds
			since the epoch of Jan 1, 1970, UTC.
			</li>
			</ul>
		</li>
		<li>
		`New' Flags
			<ul>
			<li>
			There are three `new' flags which can be set,
			to force the contraction to belong to a new
			muscle, subject or study.
			<p>
			These are useful when the file format is used
			as part of a data stream between an acquisition
			program &amp; subject database and the QEMG
			system.  The flags are used to indicate when
			a new data set is being introduced, when this
			information may not be apparant from inspection
			of the other data fields.  For instance, if the
			physician wishes to start a new study of the
			bicep muscle, this flag may be set for the
			first contraction, in order that this data not
			be collected with data from existing bicep contractions
			for the same subject.
			</li>
			</ul>
		</li>
		</ul>

	<p>
	The file format is described in the
	<a href="FileFormatProposal.txt">FileFormatProposal</a>
	text file.
 **/
class DQD_EXPORT DQEmgData
{
public:

		////////////////////////////////////////
		// Constants for the study flags
		enum Flags {
						FLAG_NEW_OPERATOR	= 0x01,
						FLAG_NEW_PATIENT	= 0x02,
						FLAG_NEW_MUSCLE		= 0x04
					};

		////////////////////////////////////////
		// Constants for the two genders
		enum Gender {
							// duplicate zero value as DQEMG uses
							// its own version of these constants
							// for some reason -- same case in Side
						UNDEFINED_GENDER	= 0x00,
						NOT_RECODED			= 0x00,
						FEMALE				= 0x01,
						MALE				= 0x02
					};

		////////////////////////////////////////
		// Constants to represent the two sides
		// of the body
		enum Side {
							// duplicate zero value as DQEMG uses
							// its own version of these constants
							// for some reason -- same case in Gender
						UNDEFINED_SIDE		= 0x00,
						NOT_RECORDED		= 0x00,
						LEFT				= 0x01,
						RIGHT				= 0x02
					};

		////////////////////////////////////////
		// Contraction Type
		enum Contraction {
						NOT_DEFINED			= 0x00,
						MVC					= 0x01,
						SPONTANEOUS			= 0x02,
						SUB_MAXIMAL			= 0x03,
						EVOKED				= 0x04
					};

		////////////////////////////////////////
		// Debug/Dump control
		enum DumpFlags {
						DUMP_NO_CHANNEL			= 0x00,
						DUMP_CHANNEL_HEADER		= 0x01,
						DUMP_SAMPLES			= 0x02
					};

public:
		////////////////////////////////////////
		// Construct an empty data set.
		// <p>
		// This is largely useful in conjunction
		// with the load() method below, which
		// will populate the data set from a file.
		DQEmgData();

		////////////////////////////////////////
		// Construct a data set for a specific
		// vendor
		// <p>
		// The resulting data set can be filled
		// in using the set methods below.  This
		// is also provided for cases where
		// all of the parameters to the next
		// constructor are unknown.
		DQEmgData(const char *vendorId);

		////////////////////////////////////////
		// Construct a data set to begin a
		// contraction set.  This will stamp the
		// global information into the data set
		// in preparation for adding in the
		// contraction data.
		// <p>
		// This is the primary constructor which
		// is intended for use when generating
		// data files.  It is assumed that all
		// of the files in a given study will
		// have the same operator and subject
		// description filled in.  This will
		// aid in their use when doing cross-subject
		// studies, in order to refer back to the
		// original data source.
		DQEmgData(
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
						const char *generalDescription = NULL
					);

		////////////////////////////////////////
		// Construct a data set to begin a
		// contraction set.  This will stamp the
		// global information into the data set
		// in preparation for adding in the
		// contraction data.
		DQEmgData(
						const char *vendorId,
						time_t acquisitionTime,
						const char *operatorDescription,
						const char *subjectDescripton,
						const char *muscleDescription,
						const char *generalDescription = NULL
					);

		////////////////////////////////////////
		// Delete a data set, including all
		// channels within.
		//
		// This is not virtual as we initialize
		// the object using memset(), so there
		// can be no virtual tables associated
		// with this object.  This is done to
		// allow data members to be added naively.
		//
		// This in turn implies that you <b>cannot</b>
		// inherit from this object
		~DQEmgData();


		////////////////////////////////////////
		// Set the vendor identifier string
		int setVendorIdentifier(const char *id);

		////////////////////////////////////////
		// Return the vendor identifier string
		// previously stored
		const char *getVendorIdentifier() const;


		////////////////////////////////////////
		// Set the operator/physician description string
		void setOperatorDescription(const char *newDescription);

		////////////////////////////////////////
		// Get the operator/physician description string
		const char *getOperatorDescription() const;


		////////////////////////////////////////
		// Returns whether this is a new operator
		// relative to the last time contraction
		// file sent
		unsigned long isNewOperator() const;

		////////////////////////////////////////
		// Set whether this is a new operator
		// relative to the last time contraction
		// file sent
		void setNewOperator(int newOperator);

		////////////////////////////////////////
		// Returns whether this is a new subject
		// relative to the last time contraction
		// file sent
		unsigned long isNewPatient() const;

		////////////////////////////////////////
		// Set whether this is a new subject
		// relative to the last time contraction
		// file sent
		void setNewPatient(int newPatient);

		////////////////////////////////////////
		// Returns whether this is a new muscle
		// relative to the last time contraction
		// file sent
		unsigned long isNewMuscle() const;

		////////////////////////////////////////
		// Set whether this is a new muscle
		// relative to the last time contraction
		// file sent
		void setNewMuscle(int newMuscle);

		////////////////////////////////////////
		// Set all the subject information at once
		void setAllSubjectInformation(
				const char *description,
				const char *SubjectID,
				int yearOfBirth,
				int monthOfBirth,
				int dayOfBirth,
				unsigned int age,
				Gender gender
			);


		////////////////////////////////////////
		// Set the subject string
		void setSubjectDescription(const char *newDescription);

		////////////////////////////////////////
		// Get the subject string
		const char *getSubjectDescription() const;


		////////////////////////////////////////
		// Set the subject ID string
		void setSubjectID(const char *newID);

		////////////////////////////////////////
		// Get the subject ID string
		const char *getSubjectID() const;


		// Set subject Age
		void setSubjectAge(unsigned int age);

		// Get subject Age
		unsigned int getSubjectAge();

		////////////////////////////////////////
		// Sets the subject D.O.B.  Returns
		// false if the year/month/day combination
		// is invalid in the Gregorian calendar
		// for that year (ie; the values of the
		// numbers are checked for consistency
		// relative to days in month, etc).
		int setSubjectDateOfBirth(
				int yearOfBirth,
				int monthOfBirth,
				int dayOfBirth
			);

		////////////////////////////////////////
		// Get the subject D.O.B
		int getSubjectDateOfBirth(
				unsigned long *year,
				unsigned long *month,
				unsigned long *day
			) const;

		////////////////////////////////////////
		// Get the date of birth in the Julian
		// calendar internal storage format
		long getSubjectJulianDateOfBirth() const;

		////////////////////////////////////////
		// Set the subject gender
		void setSubjectGender(Gender gender);

		////////////////////////////////////////
		// Get the subject gender
		int getSubjectGender() const;


		////////////////////////////////////////
		// Set the muscle description.
		void setMuscleDescription(const char *newDescription);

		////////////////////////////////////////
		// Get the muscle description
		const char *getMuscleDescription() const;


		////////////////////////////////////////
		// Set the side of the body the muscle
		// is on
		void setMuscleSide(Side laterality);

		////////////////////////////////////////
		// Get the laterality of the muscle
		int getMuscleSide() const;


		////////////////////////////////////////
		// Set the optional general description
		// field.  Used for recording notes,
		// etc.  Max field length is 65535 bytes
		void setGeneralDescription(const char *newDescription);

		////////////////////////////////////////
		// Return the general description
		// field.  This will be NULL if not
		// set.
		const char *getGeneralDescription() const;

		////////////////////////////////////////
		// Stamp the archive with a given time.
		// This time is stored as seconds since
		// the base epoch of Jan 1, 1970 UTC.
		//
		// <p>
		// This is a time suitable for conversion by
		// the ctime() function and related
		// functions.
		//
		// <p>
		// To generate values for this function,
		// the time() function may be used, which
		// generates UTC normalized values in
		// this scheme.
		void setAcquisitionTime(time_t newTime);

		////////////////////////////////////////
		// Return the time a data set was created.
		//
		// <p>
		// This time is stored as seconds since
		// the base epoch of Jan 1, 1970 UTC.
		//
		// <p>
		// This is a time suitable for conversion by
		// the ctime() function and related
		// functions.
		time_t getAcquisitionTime() const;

		////////////////////////////////////////
		// Return the number of channels which
		// are recorded in this data set.
		unsigned char getNumChannels() const;

		////////////////////////////////////////
		// Add a new channel.  In order to add
		// channel data, create and populate
		// an object of type DQEmgChannelData
		// and use this function to pass the
		// new channel in.
		//
		// <p>
		// There are a maximum of 256 channels
		// per data set.
		//
		// <p>
		// Returns 1 on success, and 0 on failure
		// (too many channels)
		int addChannel(DQEmgChannelData *newChannel);

		////////////////////////////////////////
		// Get a channel data structure.
		//
		// <p>
		// Valid values of index range from 0
		// to the value returned from
		// getNumChannels()
		DQEmgChannelData *getChannel(unsigned char index) const;


		///////////////////////////////////////
		//  AcquireEmg stores the contraction type of the
		//  signal. The voluntary signal can be one
		//  of the three following types: <br>
		//   o MVC   <br>
		//   o Spontaneous Activity <br>
		//   o Sub-Maximal   <br>
		//  Whereas the evoke signal has the type
		//   o Evoked.
		//  MPS files are all of the Evoked type
		void setContractionType(Contraction contractionType);

		///////////////////////////////////////
		// Get the type of contraction
		int  getContractionType();

		///////////////////////////////////////
		// Returns true if the contraction type is evoked
		int isEvoked();

		///////////////////////////////////////
		// Flag used to indicate that a CMAP evoked response is stored in the fiel
		void setCmapStored(int fStored);

		///////////////////////////////////////
		// Returns true if CMAP evoked response is stored in the file
		int isCmapStored();

		////////////////////////////////////////
		//Returns the number of stored Cmaps
		int numCmapsStored();
		////////////////////////////////////////
		//Sets the number of stored Cmaps
		void setNumCmapStored(int num); //ABDSAVE

				////////////////////////////////////////
		//Returns the number of auto stored Smups
		int numAutoSmupStored();
		////////////////////////////////////////
		//Sets the number of auto stored Smups
		void setNumAutoSmupStored(int num); //mauto

		///////////////////////////////////////
		// Flag used to indicate that SMUP evoked responses are stored in the file
		// This is used by MPS
		void set_mSmupStored(int fStored);

		/*MarzTest*/
		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setIndexOfFirstSmup(int index);
		//////////////////////////////////////////
		// Get evoked response negative peak area
		int getIndexOfFirstSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setIndexOfSecondSmup(int index);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		int getIndexOfSecondSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setIndexOfThirdSmup(int index);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		int getIndexOfThirdSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setIndexOfBaseSmup(int index);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		int getIndexOfBaseSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setFreqOfFirstSmup(float Freq);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		float getFreqOfFirstSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setFreqOfSecondSmup(float Freq);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		float getFreqOfSecondSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setFreqOfThirdSmup(float Freq);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		float getFreqOfThirdSmup();

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setFreqOfBaseSmup(float Freq);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		float getFreqOfBaseSmup();
/*MarzTest*/

		///////////////////////////////////////
		// Returns true if SMUP evoked responses are dtored in the file
		//
		int is_mSmupStored();


public:
	/**
	 ** File interface functions are below
	 **/

		////////////////////////////////////////
		// Get the filename under which the
		// data set was stored.
		const char *getFileName() const;

		////////////////////////////////////////
		// Get the version number of the format
		// recorded in the file, or -1 if buffer
		// has not been recovered from a file
		int getFileVersion() const;

		////////////////////////////////////////
		// Set the filename under which the
		// data set will be stored.
		void setFileName(const char *newFileName);

		////////////////////////////////////////
		// Load all data from the indicated data
		// file.
		// <p>
		// Returns 1 on success, and 0 on error.
		int load(const char *filename);

		////////////////////////////////////////
		// Store data to the indicated data
		// file.
		// <p>
		// Returns 1 on success, and 0 on error.
		int store(const char *filename);

		////////////////////////////////////////
		// Clear out all the contents.  Re-sets
		// all fields to NULL.
		void clear();

#ifdef DQD_DEBUG_DUMP
public:
		////////////////////////////////////////
		// Dump all the header info to the given
		// file pointer.  The flag controls whether
		// to also dump all channel header data
		void dump(
				FILE *fp,
				int channelSelection = (-1),
				int dumpChannelData = DUMP_CHANNEL_HEADER
			);
#endif

public:
	/**
	 * Error query interface
	 */

		////////////////////////////////////////
		// If any of the above funtions returned
		// an error status, the error state
		// is available with this function
		//
		// <p>
		// Error status 1 indicates that the
		// error is available through the errno
		// variable in the C standard library.
		int getError() const;

		////////////////////////////////////////
		// Short error messages are available
		// through passing an error state from
		// getError() into this function.
		//
		// <p>
		// If the error status is 1, this function
		// returns a message from strerror() in the
		// C standard library
		const char *getErrorMessage(int errorNumber) const;


		friend class DQEmgChannelData;

private:
		// set the current error state
		void setError(int errid);

		// Sets up default values for use in
		// constructors and read routines
		void setContractionDefaultValues_();

private:
		// internal utility functions
		int loadContractionDataHeader_(int fd);
		int storeContractionDataHeader_(int fd);
		int updateContractionDataHeader_(int fd);
		static int loadVariableStringValue_(
						int fd,
						char **string,
						DQEmgData *caller
					);
		static int storeVariableStringValue_(
						int fd,
						char *string,
						DQEmgData *caller
					);

		int load_(int fd);
		int store_(int fd);

private:
		// internal members
		char *filename_;
		char *vendorIdentifier_;
		int fileVersion_;

		time_t acquisitionTime_;
		unsigned long studyFlags_;

		char *operatorDescription_;

		char *subjectDescription_;
		long subjectJulianDateOfBirth_;
		unsigned int subjectAge_;
		char subjectGender_;
		char *subjectID_;

		char *muscleDescription_;
		char muscleSide_;

		char *generalDescription_;

		char contractionType_;
		unsigned char numChannels_;
		DQEmgChannelData **data_;

		int cmapStored_;
		int mSmupStored_;

		int numCmapStored_;	//ABDSAVE
		int numAutoSmupStored_;//mauto

		/*MarzTest*/
		int indexOfFirstSmup_;
		int indexOfSecondSmup_;
		int indexOfThirdSmup_;
		int indexOfBaseSmup_;
		float FreqOfFirstSmup_;
		float FreqOfSecondSmup_;
		float FreqOfThirdSmup_;
		float FreqOfBaseSmup_;
		/*MarzTest*/

		int errorId_;

		DQEmgDataContext *context_;
		static const char *FILE_TAG;
		static int   FILE_TAG_LEN;
		static int   DEFAULT_VERSION;
};


/**
CLASS
		DQEmgChannelData

	Channel Data -- this is the data recorded on a
	given channel during a single contraction.  The
	intention is that all channels recorded simultaneously
	will reside in the same data file.
	<p>
	For example, if an experiment is set up with an
	indwelling electrode for EMG, plus a surface electrode
	and a force transducer all recording at the same time,
	the resulting file would have three channels, each
	one containing the data from one device.  The channel
	description and units will identify the device stored
	in each channel.
	<p>
	The fields stored in the Channel Data structure are:
		<ul>

		<li>
		Channel Number
			<ul>
			<li>
			A small integer number indicating the channel
			on the physical recording equipment.
			</li>
			</ul>
		</li>

		<li>
		Channel Description
			<ul>
			<li>
			An optional textual field indicating any parameters
			which may need to be recorded on a channel by
			channel basis.  The primary purpose of this field
			will be to identify the source of the data stored
			in this channel structure.
			</li>
			</ul>
		</li>

		<li>
		High-Pass Cutoff (µHz)
			<ul>
			<li>
			The high-pass cutoff frequency of the filtering
			performed before storage.  These values are stored
			in µHz in order to avoid problems with the storage
			of fractional numbers.
			</li>
			</ul>
		</li>

		<li>
		Low-Pass Cutoff (µHz)
			<ul>
			<li>
			The low-pass cutoff frequency of the filtering
			performed before storage.
			</li>
			</ul>
		</li>

		<li>
		Sampling Rate
			<ul>
			<li>
			The rate, in Hz, at which the data was sampled.
			</li>
			</ul>
		</li>

		<li>
		Number of Samples
			<ul>
			<li>
			This is the length of the data vector.
			</li>
			</ul>
		</li>

		<li>
		Elapsed Time
			<ul>
			<li>
			Optional -- this will indicate the time elapsed
			during the contraction.  This can also be calculated
			by Sampling Rate * Number of Samples.
			</li>
			</ul>
		</li>

		<li>
		Units
			<ul>
			<li>
			This field will describe the units in which the
			data is encoded, as divided by the scale.
			<p>
			The intention is that the Units field is chosen
			as a metric unit string describing the storage
			units.  Examples: "mV" - milli Volts.  "C" degrees
			Centigrade, <i>etc</i>.
			</li>
			</ul>
		</li>

		<li>
		Scale
			<ul>
			<li>
			The <i>scale</i> is used in conjunction with the data
			vector to transform the vector back from an
			integer-based 16 or 32 bit representation into
			a floating-point representation.
			<p>
			In order to reconstruct the vector in the specified
			units, the scale will be multiplied by the value
			in the data vector.
			</li>
			</ul>
		</li>

		<li>
		Evoked Values
			<ul>
			<li>
			The <i>Evoked Values</i> are generated by the AcquireEmg or MPS
			Evoked window and contain the : <br>
			 o peakToPeakArea, <br>
			 o peakToPeakAmplitude and <br>
			 o negativePeakAmplitude <br>
			values. They are loaded if the contraction type
			of the data file was detected to be EVOKED.
			</li>
			</ul>
		</li>

		<li>
		Data
			<ul>
			<li>
			The data is a vector of 16 or 32 bit values.  The
			internal storage is defined by the setting of the
			encoding argument.
			<p>
			The data, when multiplied by the scale, will
			provide a vector of floating-point number in the
			units indicated in the units string.
			<p>
			This vector will be of the length indicated by
			the number of samples argument.
			</li>
			</ul>
		</li>

		<li>
		Data Encoding
			<ul>
			<li>
			Used internally to expand the file storage values
			from 16 to 32 bit storage.  Setting the encoding
			to 32-bit (4 byte) will double the file storage
			size requirements.
			</li>
			</ul>
		</li>

		</ul>
 **/
class DQD_EXPORT DQEmgChannelData
{
public:
		////////////////////////////////////////
		// Create an empty data channel object
		DQEmgChannelData();

		////////////////////////////////////////
		// Create a data channel object with
		// the attached description.  Use
		// setData() to attach an actual data
		// stream.
		DQEmgChannelData(
						unsigned char channelNumber,
						unsigned long highPassCutoff,
						unsigned long lowPassCutoff,
						const char *channelDescription = NULL
					);


		////////////////////////////////////////
		// Delete this channel data object.
	//
	// Not virtual; see discussion on
	// destructor for ~DQEMGData
		~DQEmgChannelData();

		////////////////////////////////////////
		// Set the data from a series of float
		// values.  The data is stored in the
		// file based on the setting of the
		// encoding value, which defaults to
		// short.  In order to convert to
		// the encoding based integral-data width,
		// a scale factor is calculated.  This
		// scale factor is formed by calculating
		// the number by which the data must
		// be divided in order to fit within
		// the provided range.  If a smaller
		// range than that provided by the
		// data encoding width is desired,
		// this can be passed in instead.
		// <p>
		// If the <b>maxUnscaledValue</b>
		// is set to zero, the default value
		// for the encoding width is used.
		// <p>
		// Note that the sampling rate is set
		// in samples per second
		int setData(
						unsigned long numSamples,
						float *data,
						float samplingRate,
						const char *unitDescription,
						int encodingWidth = 0x02,
						long maxUnscaledValue = 0
					);

		////////////////////////////////////////
		// Set the data from a series of short
		// values.  The data is stored in the
		// file based on the setting of the
		// encoding value, which defaults to
		// short.
		// <p>
		// Data passed into this routing shoule
		// have been divided by the scale factor
		// beforehand.  The intention is that
		// this will raise the data to relatively
		// large integer numbers, so that storage
		// in an integer format will not lose
		// undue precision.
		// <p>
		// Note that the sampling rate is set
		// in samples per second
		int setData(
						unsigned long numSamples,
						short *data,
						float samplingRate,
						const char *unitDescription,
						float scale
					);

		////////////////////////////////////////
		// Set the data from a series of long
		// values.  The data is stored in the
		// file based on the setting of the
		// encoding value, which defaults to
		// short.
		// <p>
		// Data passed into this routing shoule
		// have been divided by the scale factor
		// beforehand.  The intention is that
		// this will raise the data to relatively
		// large integer numbers, so that storage
		// in an integer format will not lose
		// undue precision.
		// <p>
		// Note that the sampling rate is set
		// in samples per second
		int setData(
						unsigned long numSamples,
						long *data,
						float samplingRate,
						const char *unitDescription,
						float scale
					);

		////////////////////////////////////////
		// Possible encodings supported by this
		// version of the file reader
		enum Encoding {
						SHORT	= 0x0002,
								// encode data
								// using short
								// (2-byte) values
						LONG	= 0x0004
								// encode data
								// using long
								// (4-byte) values
					} ;

		////////////////////////////////////////
		// Set the encoding of the data.  The
		// default is SHORT
		int setEncoding(Encoding encoding);

		////////////////////////////////////////
		// Return the flag portion of the
		// encoding for this data file.
		//
		// <p>
		// Note that in this initial implementation
		// of the file specification, there are no
		// valid flags.  This will be used in future
		// if any changes need to be made to the
		// data encoding which will make a future
		// file unreadable to this version of the
		// parser.  An example of such a change would
		// be the implementation of a compression
		// scheme, which is currently not in the
		// specification.
		int getEncodingFlags() const;

		////////////////////////////////////////
		// Return the width of the encoding of
		// the data in the file.
		//
		// <p>
		// Currently, the only two legal widths
		// are 2 and 4 bytes.
		int getEncodingWidth() const;

		////////////////////////////////////////
		// Return a pointer to the data in
		// <b>short</b> format.
		// <p>
		// In order to convert the data
		// back into the format described by
		// the unit description, the user is
		// expected to multiply the values
		// returned here by the value in
		// the <b>scale</b> parameter
		// <p>
		// Note that in order to provide the
		// data, the class may have to allocate
		// a new buffer and convert the values.
		// <p>
		// See also getDataAsLong(), and getEncodingWidth()
		short *getDataAsShort(
						unsigned long *numSamples,
						float *samplingRate,
						float *scale
					);

		////////////////////////////////////////
		// Return a pointer to the data in
		// <b>long</b> format.
		// <p>
		// In order to convert the data
		// back into the format described by
		// the unit description, the user is
		// expected to multiply the values
		// returned here by the value in
		// the <b>scale</b> parameter.
		// <p>
		// Note that in order to provide the
		// data, the class may have to allocate
		// a new buffer and convert the values.
		// For this reason, this function may
		// return failure if the 32-bit data values
		// cannot successfully be trimmed into
		// 16-bit values without overflow.
		//
		// <p>
		// See also getDataAsShort(), and getEncodingWidth()
		long *getDataAsLong(
						unsigned long *numSamples,
						float *samplingRate,
						float *scale
					);


		////////////////////////////////////////
		// Get the description of the units the
		// data was encoded with
		const char *getUnitDescription() const;


		////////////////////////////////////////
		// Set the (optional) text field describing
		// the channel settings
		void setChannelDescription(const char *newDescription);

		////////////////////////////////////////
		// Return the text field describing the
		// channel settings.  This is a pointer
		// to an internally managed buffer.  Do
		// not free() this pointer.
		const char *getChannelDescription() const;


		////////////////////////////////////////
		// Set the channel id number
		void setChannelNumber(unsigned char channelNumber);

		////////////////////////////////////////
		// Return the channel id number
		unsigned char getChannelNumber() const;


		////////////////////////////////////////
		// Record the frequency used for high-pass filtering
		// in µHz
		void setHighPassCutoff(unsigned long frequency);

		////////////////////////////////////////
		// Return the frequency used for high-pass filtering,
		// in µHz
		unsigned long getHighPassCutoff() const;

		////////////////////////////////////////
		// Record the frequency used for low-pass filtering
		// in µHz
		void setLowPassCutoff(unsigned long frequency);

		////////////////////////////////////////
		// Return the frequency used for low-pass filtering
		// in µHz
		unsigned long getLowPassCutoff() const;

		////////////////////////////////////////
		// Set the total time elapsed for this channel
		void setElapsedTime(unsigned long newTime);

		////////////////////////////////////////
		// Get the total time elapsed for this channel
		unsigned long getElapsedTime() const;

		/////////////////////////////////////////
		// Get the number of samples in this channel
		unsigned long getNumberOfSamples() const;

		//////////////////////////////////////////
		// Get the sample Rate in samples per second
		float getSamplingRate() const;

		//////////////////////////////////////////
		// Get the Scale factor
		float getScaleFactor() const;


	/**
	 ** These functions are for storing and reading evoked response
	 ** parameter values and are used to pass the information to other applications
	 ** such as betweem AcquireEmg and DQEMG
	 **/

		////////////////////////////////////////
		// Set onset position in evoked response
		void setEvokedResponseOnsetPos(short onset);
		//////////////////////////////////////////
		// Get evoked response onset position
		short getEvokedResponseOnsetPos() const;

		//////////////////////////////////////////
		// set evoked resopnse peak onset position
		void setEvokedResponsePeakOnsetPos(short peakOnset);
		//////////////////////////////////////////
		// get evoked response peak onset position
		short getEvokedResponsePeakOnsetPos() const;

		////////////////////////////////////////
		// Set negative peak position in evoked response
		void setEvokedResponseNegativePeakPos(short NegativePeak);
		//////////////////////////////////////////
		// Get evoked response negative peak position
		short getEvokedResponseNegativePeakPos() const;

		//////////////////////////////////////////
		// Set evoked reponse negative peak end position
		void setEvokedResponsePeakEndPos(short peakEnd);
		//////////////////////////////////////////
		// Get evoked reponse negative peak end position
		short getEvokedResponsePeakEndPos() const;

		////////////////////////////////////////
		// Set positive peak position in evoked response
		void setEvokedResponsePositivePeakPos(short posPeak);

		//////////////////////////////////////////
		// Get evoked response positive peak position
		short getEvokedResponsePositivePeakPos() const;

		////////////////////////////////////////
		// Set end position in evoked response
		void setEvokedResponseEndPos(short End);

		//////////////////////////////////////////
		// Get evoked response end position
		short getEvokedResponseEndPos() const;

		//////////////////////////////////////////
		// Set evoked resopnse peak to peak amplitude
		void setEvokedResponsePeakToPeakAmplitude(float peakToPeakAmplitude);

		//////////////////////////////////////////
		// Get evoked response peak to peak amplitude
		float getEvokedResponsePeakToPeakAmplitude() const;

		//////////////////////////////////////////
		// Set evoked response negative peak amplitude
		void setEvokedResponseNegativePeakAmplitude(float negativePeakAmplitude);

		//////////////////////////////////////////
		// Get evoked response negative peak amplitude
		float getEvokedResponseNegativePeakAmplitude() const;

		//////////////////////////////////////////
		// set evoked response duration
		void setEvokedResponseDuration(float duration);

		//////////////////////////////////////////
		// get evoked response duration
		float getEvokedResponseDuration() const;

		//////////////////////////////////////////
		// set evoked response negative peak duration
		void setEvokedResponseNegativePeakDuration(float negativePeakDuration);

		//////////////////////////////////////////
		// get evoked response negative peak duration
		float getEvokedResponseNegativePeakDuration() const;

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setEvokedResponseNegativePeakArea(float negativePeakArea);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		float getEvokedResponseNegativePeakArea() const;

		//////////////////////////////////////////
		// set evoked response area
		void setEvokedResponseArea(float area);

		//////////////////////////////////////////
		// get evoked response area
		float getEvokedResponseArea() const;


		//////////////////////////////////////////
		// Set evoked resopne validity flag
		// This flag is used to indicate if an
		// evoked response is to be included in a study or not
		void setEvokedResponseValidity(bool Valid);

		//////////////////////////////////////////
		// Get evoked resopne validity flag
		bool getEvokedResponseValidity() const;

		//////////////////////////////////////////
		// save the number of surface mups in
		// the mSMUP
		// This information is used in MPS when calculating the mSMUP
		void setNumberOfSmupsInAvg(int NumSmups);

		//////////////////////////////////////////
		// read the number of surface mups in
		// the mSMUP
		int getNumberOfSmupsInAvg() const;

		//////////////////////////////////////////
		// Set evoked response negative to peak area
		void setEvokedResponseIndex(int index);

		//////////////////////////////////////////
		// Get evoked response negative peak area
		int getEvokedResponseIndex() const;


public:
	/**
	 * Debugging functions
	 */

#ifdef DQD_DEBUG_DUMP
		////////////////////////////////////////
		// Dump all the header info to the given
		// file pointer.
		void dump(FILE *fp, int dumpSamples = 0);
#endif

		friend class DQEmgData;


private:

	/**
	 * Internal utility functions
	 */
		void deleteOldData_();

		// Sets up default values for use in
		// constructors and read routines
		void setChannelDefaultValues_();

		int loadChannelData_(int fd, DQEmgData *caller);
		int storeChannelData_(int fd, DQEmgData *caller);
		void setUnitDescription_(const char *newDescription);
		int convertDataToShort_();
		int convertDataToLong_();

private:
		char *channelDescription_;
		char *unitDescription_;
		unsigned char channelNumber_;
		unsigned long hipassCutoff_;
		unsigned long lopassCutoff_;
		unsigned long elapsedTime_;
		float samplingRate_;

		float scale_;

		unsigned long numSamples_;
		short *dataShortSample_;
		long *dataLongSample_;

		unsigned short encoding_;

		//  The variables are related to evoked responses such as CMAPs and SMUPs
		short onsetPos_;
		short peakOnsetPos_;
		short negativePeakPos_;
		short peakEndPos_;
		short positivePeakPos_;
		short endPos_;
		int	  index_; //ABDSAVE

		float negativePeakArea_;
		float peakToPeakAmplitude_;
		float negativePeakAmplitude_;
		float duration_;
		float negativePeakDuration_;
		float area_;

		bool isValid_;
		int numSmupsInAvg_;
};

inline int DQEmgData::getFileVersion() const
{
	return fileVersion_;
}

inline void DQEmgChannelData::setEvokedResponseOnsetPos(short onsetPos)
{
	onsetPos_ = onsetPos;
}
inline short DQEmgChannelData::getEvokedResponseOnsetPos() const
{
	return onsetPos_;
}

inline void DQEmgChannelData::setEvokedResponsePositivePeakPos(short positivePeakPos)
{
	positivePeakPos_ = positivePeakPos;
}
inline short DQEmgChannelData::getEvokedResponsePositivePeakPos() const
{
	return positivePeakPos_;
}

inline void DQEmgChannelData::setEvokedResponseNegativePeakPos(short negativePeakPos)
{
	negativePeakPos_ = negativePeakPos;
}
inline short DQEmgChannelData::getEvokedResponseNegativePeakPos() const
{
	return negativePeakPos_;
}

inline void DQEmgChannelData::setEvokedResponseEndPos(short endPos)
{
	endPos_ = endPos;
}
inline short DQEmgChannelData::getEvokedResponseEndPos() const
{
	return endPos_;
}
inline void DQEmgChannelData::setEvokedResponseIndex(int index)	//ABDSAVE
{
	index_ = index;
}
inline int DQEmgChannelData::getEvokedResponseIndex() const		 //ABDSAVE
{
	return index_;
}


#endif /* __EMG_DATA_DEFINITION_HEADER__ */

