/**
 ** This class controls a definition of a single MUP, and
 ** contains the interface to save/restore it to a file
 ** store.
 **
 ** This class is used within the Jitter code, as the
 ** individual MFP's managed within this code are accessed
 ** in order to generate a Jitter'ed MUP when required.
 **
 ** $Id: MUP.h 22 2017-11-17 18:02:31Z andrew $
 **/
#ifndef __MUP_CLASS_HEADER__
#define __MUP_CLASS_HEADER__

#include "os_defs.h"
#include "os_types.h"

# ifndef        MAKEDEPEND
# include       <stdio.h>
#  ifdef        OS_WINDOWS_NT
#    include    <io.h>
#  else
#    include    <unistd.h>
#  endif
#  include      <sys/types.h>
#  include      <fcntl.h>
# endif

#include "io_utils.h"

/** conditional debug dump control flags */
/*
#define         DUMP_MFP_DATA
#define         DUMP_MUP_DATA
*/


#define         MFP_BLOCKSIZE          2
#define         MUP_ID_WIDTH           4

typedef float MUPDataElement;
typedef double generatedElement;

/**
CLASS
		MUP

	This class controls a definition of a single MUP, and
	contains the interface to save/restore it to a file
	store.
	<p>
	This class is used within the Jitter code, as the
	individual MFP's managed within this code are accessed
	in order to generate a Jitter'ed MUP when required.
	<p>
	This class contains the data to generate a MUP train.  An
	instance of this object will contain a set of MFP firings,
	which can be used with a given Jitter value to create
	a MUP train.
	<p>
	See the calls:
	<br>
	int calcJitteredMUP()
	<br>
	and
	<br>
	MUPDataElement *getCurrentMUP();
	<br>
	for details.
 **/
class MUP
{
public:
		////////////////////////////////////////////////////////////////
		// threshold at which the MFP's will be stored in their
		// own "Jitterable" buffers.  Any data which has
		// accelerations consistently less than this threshold
		// will be merged into a single, composite MUP
		static generatedElement sAccelerationThreshold_;

		////////////////////////////////////////////////////////////////
		// Constants for cannula/tip combination selection
		enum ReferenceSetup {
		                TIP_ONLY                = 0x01,
		                CANNULA_ONLY            = 0x02,
		                TIP_VERSUS_CANNULA      = 0x03
		        };

		////////////////////////////////////////////////////////////////
		// Jitter the MFP either individually, for a single firing,
		// or using the sum information
		enum JitterIndividualOrTemplate {
		                JITTER_INDIVIDUAL		= 0x01,
		                JITTER_TEMPLATE
		        };

private:
		////////////////////////////////////////////////////////////////
		// expansion factor by which we expand the "Jitterable"
		// MFPs in order to be able to select data when shifted
		// by units of less than integer sampling rate in the
		// jitter shift.
		static int      sExpansionFactor_;

private:
#       ifdef   DUMP_MUP_DATA
		int MUPDumpId_;
#       endif

private:
		class MFP {
		public:
		        MUPDataElement *data_;
		        osInt32 allocatedSize_;
		        osInt32 numPoints_;
		        osInt32 expansionFactor_;
		        osInt32 fibreIdentifier_;

		         MFP();
		        ~MFP();
		};


		struct JitterValue;

		int id_;

		const char *filename_;

		int isLoaded_;

		int dcoID_;

		/**
		 ** the number of data points in the "base" mfap.
		 ** This is equal to the number of points in the
		 ** output vector if there is no jitter
		 **/
		osInt32 nInterfaceDataPoints_;


		/** various vectors of MUP data */
		MUPDataElement *MUPVector_;
		MUPDataElement *MUPAccelerationVector_;
		MUPDataElement *MUPSlopeVector_;

		/**
		 ** used to manage the re-creation of slope, acceleration
		 ** when needed
		 **/
		int MUPAccelerationIsDirty_;
		int MUPSlopeIsDirty_;


		/**
		 ** MFP 0 is special.  It is constructed of
		 ** all of the MFP's which are too "low frequency"
		 ** for us to bother Jittering.  The remainder
		 ** of the list contains the individual contributions
		 ** from fibres which contain a high-acceleration
		 ** component.
		 **/
		osInt32 nMFPs_;
		int nMFPBlocks_;
		MFP **mfapList_;
		MFP *cannulaMFP_;
		int hasCannulaMFP_;

		struct JitterValue *jitterValue_;
		struct JitterValue *jitterValueSums_;
		int nJitterValuesInSum_;


		/** MFP which we are using for alignment */
		osInt32 alignmentMFP_;

		/** point in expanded buffer where we want alignment to be */
		osInt32 expandedUnitsAlignmentOffset_;

		/** point where we will report the firing time */
		int MUPUnitsAlignmentPoint_;

		/** used internally to manage load state */
		off_t *mfapLoadOffsets_;
		FP *mfapLoadFP_;

private:
		// internal functions
		osInt32 calculateMaxSlopeAlignmentPoint__(
		        MUPDataElement *data,
		        int nDataPoints,
		        int beginThresholdIndex
		    );
		void addAsSeparateMFP__(
		        generatedElement *data,
		        int beginThresholdIndex,
		        osInt32 *expandedSlopeAlignmentIndex,
		        osInt32 fibreIdentifier
		    );
		void addAsMergedMFP__(generatedElement *data);
		off_t headerOffsetSize__() const;
		int writeHeader__(FP *fp, off_t *mfapOffsetTable) const;
		int readHeader__(FP *fp, off_t **mfapOffsetTable);
		int writeData__(
		                        FP *fp,
		                        int index,
		                        MFP *curMFP,
		                        off_t *offset
		                ) const;
		int readData__(
		                        FP *fp,
		                        MFP **curMFP,
		                        off_t offset
		                );

		void initializeMUPVector__();
		void selectJitterTimes__(
		                        int doJitter,
		                        float jitterVariance
		                );
		void combineMFPs__(
		                        MUPDataElement *loadBuffer,
								JitterIndividualOrTemplate jitterSourceSelection
		                );

		void addBaseMFPToSignal__(
		                        MUPDataElement *loadBuffer
		                );

		void addCannulaMFPToSignal__(
		                        MUPDataElement *loadBuffer,
		                        ReferenceSetup referenceSetup
		                );


		void interpolateTurn__(
		                        MUPDataElement *resultVector,
		                        generatedElement *sourceVector,
		                        int sign
		                );

		int loadMFPs__();
		int haveMFPsBeenLoaded__() const;

public:

		////////////////////////////////////////////////////////////////
		// create an (empty) MUP structure
		MUP(const char *path, int id);

		////////////////////////////////////////////////////////////////
		// create and load the MUP based on the path
		MUP(const char *filename);

		////////////////////////////////////////////////////////////////
		// clean up a MUP structure
		~MUP();

		////////////////////////////////////////////////////////////////
		// Set the jitter threshold.  This is static so that it will
		// affect all instances of the MUP class
		static void sSetJitterAccelerationThreshold(
		                generatedElement jitterThreshold
		            );

		////////////////////////////////////////////////////////////////
		// Return the jitter threshold */
		static generatedElement sGetJitterAccelerationThreshold();

		////////////////////////////////////////////////////////////////
		// Set the factor by which the high-acceleration values are
		// expanded.  By default, this is 30.
		// <p>
		// Again, this is static, and so affects all instances of this
		// class
		static void sSetExpansionFactor(int newExpansionFactor);

		////////////////////////////////////////////////////////////////
		// Return the current static expansion factor
		static int sGetExpansionFactor();

		////////////////////////////////////////////////////////////////
		// the file we will be using as the file back-end store
		const char *getFileName() const;

		////////////////////////////////////////////////////////////////
		// the id number used to generate the file name.  This is
		// the id of the motor-unit which generated this data structure
		int getId() const;

		////////////////////////////////////////////////////////////////
		// return the number of MFPs we have stored.
		int getNMFPs() const;

		////////////////////////////////////////////////////////////////
		// return the MFP <i>index</i> of N, where N is returned by
		// getNMFPs().
		//
		// The expansionFactor may differ between MFP 0 and the
		// rest of the MFPs.
		//
		// The <b>fibreIdentifier</b> field is the identifier of
		// the fibre within the motor unit which generated this
		// MUP.
		MUPDataElement *getMFP(
		        int index,
		        osInt32 *numberOfDataPoints,
		        osInt32 *expansionFactor,
		        osInt32 *fibreIdentifier
		    );

		////////////////////////////////////////////////////////////////
		// return the MFP recorded for the cannula
		//
		MUPDataElement *getCannulaMFP(
		        osInt32 *numberOfDataPoints,
		        osInt32 *expansionFactor
		    );

		////////////////////////////////////////////////////////////////
		// add in a new MFP
		void addMFP(
		                int MUPIndex,
		                int nElements,
		                generatedElement *data,
		                osInt32 fibreIdentifier
		            );

		////////////////////////////////////////////////////////////////
		// add in the cannula MFP
		void addCannulaMFP(
		                int MUPIndex,
		                int nElements,
		                generatedElement *data
		            );

		////////////////////////////////////////////////////////////////
		// calculate the MUP data.  This must be called before
		// MUPDataElement *getCurrentMUP(int MUPIndex)
		// contains any useful values
		//
		// The arguments to this function are used in the following
		// way:
		// <ul>
		// <li>MUPIndex : select needle position (currently
		// must always be zero</li>
		//
		// <li>doJitter : enables jitter calculation internally</li>
		//
		// <li>jitterVariance : if doJitter is non-zero, this is
		// used in the calculation of the Jitter values</li>
		//
		// <li>referenceSetup : indicates the bias of the differential
		// amplifier used with the cannula recording.  Can be
		// set with the following results:
		// <table>
		// <tr><td>TIP_ONLY</td>
		//     <td>calls to getCurrentMUP() will return only
		//     the tip MFP recordings</td></tr>
		// <tr><td>CANNULA_ONLY</td>
		//     <td>calls to getCurrentMUP() will return only
		//     the cannula MFP recordings</td></tr>
		// <tr><td>TIP_VERSUS_CANNULA</td>
		//     <td>calls to getCurrentMUP() will return
		//     the tip recordings MINUS the cannula recordings</td></tr>
		// </table>
		// </li>
		// </ul>
		int calcJitteredMUP(
		                int MUPIndex,
		                int doJitter,
		                float jitterVariance,
		                ReferenceSetup referenceSetup
		                        = TIP_VERSUS_CANNULA,
						JitterIndividualOrTemplate jitterSourceSelection
								= JITTER_INDIVIDUAL
		            );

		////////////////////////////////////////////////////////////////
		// reset all of the jitter structures
		void resetJitterAccounting();

		////////////////////////////////////////////////////////////////
		// return the currently calculated MUP.  Call
		// calcJitteredMUP()
		// in order to set up the value in this function.
		// <p>
		// The getCurrentMUP() function can be called many
		// times for each set of the calculate function, without
		// change to the internal (randomly generated) values.
		MUPDataElement *getCurrentMUP(int MUPIndex) const;

		////////////////////////////////////////////////////////////////
		// return the acceleration for the currently calculated MUP
		// as a vector of values
		MUPDataElement *getCurrentMUPAcceleration(
		                int MUPIndex
		            );

		////////////////////////////////////////////////////////////////
		// return the slope for the currently calculated MUP
		// as a vector of values
		MUPDataElement *getCurrentMUPSlope(
		                int MUPIndex
		            );

		////////////////////////////////////////////////////////////////
		// get the mfap alignment point in the current MUP
		int getCurrentMUPAlignmentPoint(int MUPIndex) const;

		////////////////////////////////////////////////////////////////
		// how long is the MUP buffer retured in getMUP?
		osUint32 getMUPBufferLength() const;

		////////////////////////////////////////////////////////////////
		// Base sampling rate in output buffer.
		// For MFP buffers, this must be multiplied
		// by the expansion factor.
		float getMUPSamplingRate() const;

		////////////////////////////////////////////////////////////////
		// save to the file store
		int save() const;

		////////////////////////////////////////////////////////////////
		// load the file from the file store
		int load(int loadAllFlag = 0);

		////////////////////////////////////////////////////////////////
		// clears the loaded data.  Filename is preserved.
		void unload(void);

		////////////////////////////////////////////////////////////////
		// Returns true when load has been called.  Cleared by clear()
		int isLoaded(void) const;

		////////////////////////////////////////////////////////////////
		// Sets the DCO ID if this MUP gets a number in the DCO file
		void setDCOID(int id);

		////////////////////////////////////////////////////////////////
		// Gets the DCO ID -- set if this MUP got a number in the DCO file
		int getDCOID();

		////////////////////////////////////////////////////////////////
		// dump for debugging
		void dump(FILE *fp) const;
};

inline const char *MUP::getFileName() const
{
	return filename_;
}

inline int MUP::getId() const
{
	return id_;
}

inline int MUP::getNMFPs() const
{
	return nMFPs_;
}

inline osUint32 MUP::getMUPBufferLength() const
{
	return (osUint32) nInterfaceDataPoints_;
}

inline int MUP::isLoaded() const
{
	return isLoaded_;
}

inline int MUP::haveMFPsBeenLoaded__() const
{
	return (mfapLoadOffsets_ == NULL) ? 
			((filename_ == NULL) ? 0 : 1)
		: 0;
}

inline void MUP::setDCOID(int id)
{
	this->dcoID_ = id;
}

inline int MUP::getDCOID()
{
	return this->dcoID_;
}

#endif

