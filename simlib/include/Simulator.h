/**
 ** Simulator Interface File
 ** Contains the definition of the simulator object, and the
 ** results it deals in
 **
 ** $Id: Simulator.h 25 2019-06-12 12:16:58Z andrew $
 **/

#ifndef __SIMULATOR_HEADER__
#define __SIMULATOR_HEADER__

#include <stdio.h>

#ifndef PRIVATE
#define PRIVATE private
#endif


class SimulationResult;
class MuscleData;
class DQEmgData;
class MUP;
class NeedleInfo;
class SMUP;

struct dcoData;

struct globals;

/**
CLASS
		Simulator
	
	This object wraps up all the simulator functionality -- calling
	run on this object will return a result object which can be used
	to get all the data from a single simulator run.
**/
class Simulator
{
PRIVATE:
	char *configFile_;

public:
	////////////////////////////////////////////////////////////////
	// constant bitflag
	static const int FLAG_USE_LAST_MUSCLE;
	static const int FLAG_USE_OLD_FIRING_TIMES;
	static const int FLAG_RUN_SURFACE;

public:

	////////////////////////////////////////////////////////////////
	// Constructor.
	Simulator();

	////////////////////////////////////////////////////////////////
	// Destroy the Simulator object
	~Simulator();

	////////////////////////////////////////////////////////////////
	// Get a pointer to the global config structure, initializing
	// it from the given file, or the default file if none is given.
	//
	struct globals *initializeGlobals(
			const char *configFile,
			const char *outputRoot
		);

	////////////////////////////////////////////////////////////////
	// Run the simulator.  Possible flags are:
	// <ul>
	//     <li>FLAG_USE_LAST_MUSCLE</li>
	//     <li>FLAG_USE_OLD_FIRING_TIMES</li>
	//     <li>FLAG_RUN_SURFACE</li>
	// </ul>
	SimulationResult *run(int flags = 0x00);

	////////////////////////////////////////////////////////////////
	// Run the simulator for surface simulation only.  No flags
	// at the moment
	SimulationResult *runSurface(int flags = 0x00);

	////////////////////////////////////////////////////////////////
	// Run the simulator.  Possible flags are:
	// <ul>
	//     <li>FLAG_USE_LAST_MUSCLE</li>
	//     <li>FLAG_USE_OLD_FIRING_TIMES</li>
	//     <li>FLAG_RUN_SURFACE</li>
	// </ul>
	SimulationResult *open(
		        const char *path,
		        int contractionId = 1,
		        int usePlowedFibres = 1
		    );

	////////////////////////////////////////////////////////////////
	// Get the config file name
	const char *getConfigFileName();

	////////////////////////////////////////////////////////////////
	// Set the config file name
	int setConfigFileName(const char *configFileName);

	////////////////////////////////////////////////////////////////
	// Get the simulator version id string
	static const char *sGetVersion();

private:
	////////////////////////////////////////////////////////////////
	// read in configuation info from file
	int readConfigInfo();
};



/**
CLASS
		SimulationResult
	
	A wrapper for all the result data from a simulator run.
	<p>
	This class will contain all the resulting data for a simulation
	run including:
	<ul>
	<li>The EMG signal, via getEmgData()</li>
	<li>Motor unit and muscle fibre data, via getMuscleData()</li>
	<li>MUP structures, via getMUP()</li>
	<li>DCO (timing) data via getDcoData()</li>
	</ul>
**/
class SimulationResult
{
protected:
	int errorState_;
	const char *errorMessage_;
	char *outputDirectory_;
	char *baseDirectory_; // Added on Aug 7, 2002 to ensure that this string is deleted properly
	int fileId_;

PRIVATE:
	MUP **MUPList_;
	char *MUPPath_;
	int *MUPIdList_;
	int nMUPs_;
	
	SMUP **sMUPList_;
	int nSMUPs_;

	MuscleData *muscleData_;

	DQEmgData *emgData_;

	dcoData *dcoData_;

	NeedleInfo *needleInfo_;

public:
	////////////////////////////////////////////////////////////////
	// Constructor -- private as these objects can be created
	// only within the Simulator object.
	SimulationResult();


public:

	// The Simulator is our friend, as we are the private result
	// class for it.
	friend class Simulator;

	////////////////////////////////////////////////////////////////
	// Destroy the SimulationResult object
	~SimulationResult();

	////////////////////////////////////////////////////////////////
	// Returns 0 if the run was successful, and non-zero if there
	// was a failure.  See getErrorMessage() for a description of
	// the failure
	int getErrorState() const;

	////////////////////////////////////////////////////////////////
	// Returns an error message describing the failure for cases
	// where getErrorState() returns a non-zero value
	const char *getErrorMessage() const;

	////////////////////////////////////////////////////////////////
	// Get the file id.  This is the value used to tag the result
	// files in the output directory (contration<ID>.dat, <i>et cetera</i>).
	int getFileId() const;

	////////////////////////////////////////////////////////////////
	// Get the DQEmgData object which describes the EMG signal generated.
	// <p>
	// Note that the single DQEmgData object contains all the channels
	// (micro and macro) of data collected.
	DQEmgData *getEmgData() const;

	////////////////////////////////////////////////////////////////
	// Get the DcoData object which describes the firing times of
	// the generated EMG signal.
	dcoData *getDcoData() const;

	////////////////////////////////////////////////////////////////
	// Get the NeedleInfo object attached to this simulation
	NeedleInfo *getNeedleInfo() const;

	////////////////////////////////////////////////////////////////
	// Get the MuscleData object describing the muscle layout and fibre
	// assignments used to generate the MFPs.
	// <p>
	// Note that the single MuscleData structure contains all the info
	// about the muscle layout.
	MuscleData *getMuscleData() const;

	////////////////////////////////////////////////////////////////
	// Returns the number of MUP structures this result contains
	int getNumMUPs() const;

	////////////////////////////////////////////////////////////////
	// Frees a MUP allocated in getMUP(int index) or getMUPList()
	void freeMUP(int index);

	////////////////////////////////////////////////////////////////
	// Frees all MUPs allocated in getMUP(int index) or getMUPList()
	void freeMUPList();

	////////////////////////////////////////////////////////////////
	// Returns a particular MUP structure from 0 to getNumMUPs().
	// Each of these structures will contain both the resultant
	// MUP template, as well as each individual MFP they are composed
	// of.
	//
	// See also freeMUP(int index)
	MUP *getMUP(int index);

	////////////////////////////////////////////////////////////////
	// Returns an array of MUP pointers.
	// <p>
	// The length of the array is available through the
	// getNumMUPs() method.
	// <p>
	// Each of these structures will contain both the resultant
	// MUP template, as well as each individual MFP they are composed
	// of.
	MUP ** getMUPList();

	////////////////////////////////////////////////////////////////
	// Get the directory in which the output is saved.
	const char *getOutputDirectory();

	////////////////////////////////////////////////////////////////
	// Get the base directory 
	const char *getBaseDirectory();


	///////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////
	// Added by R.Tollola, 25/09/2002
	
	// Get number of SMUPs in the list
	int getNumSMUPs();
	
	// Get a SMUP pointer from the list
	SMUP *getSMUP(int index);
	
	// Get a pointer to the list of SMUPs
	SMUP **getSMUPList();
	
	
	///////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////



public:
	////////////////////////////////////////////////////////////////
	// dump all the possible data
	const static int PRINT_ALL_DATA;

	////////////////////////////////////////////////////////////////
	// dump the EMG data when printing (default - enabled)
	const static int PRINT_EMG_DATA;

	////////////////////////////////////////////////////////////////
	// dump the MUP data when printing (default - enabled)
	const static int PRINT_MUP_DATA;

	////////////////////////////////////////////////////////////////
	// dump the muscle data when printing (default - disabled)
	const static int PRINT_MUSCLE_DATA;

	////////////////////////////////////////////////////////////////
	// Print out the contents to the file pointer for debugging
	// purposes.
	// <p>
	// See flags:
	// <ul>
	// <li>PRINT_EMG_DATA</li>
	// <li>PRINT_MUP_DATA</li>
	// <li>PRINT_MUSCLE_DATA</li>
	// <ul>
	void dump(
		        FILE *fp,
		        int flags = PRINT_EMG_DATA|PRINT_MUP_DATA
		    );


protected:
	////////////////////////////////////////////////////////////////
	// Set the current file id.  Used within the Simulator::run() method
	void setFileId(int id);

	////////////////////////////////////////////////////////////////
	// Set the error state.  Used within the Simulator::run() method.
	void setState(int error);
};

inline DQEmgData *SimulationResult::getEmgData() const {
	return emgData_;
}

inline dcoData *SimulationResult::getDcoData() const {
	return dcoData_;
}

inline MuscleData *SimulationResult::getMuscleData() const {
	return muscleData_;
}

inline int SimulationResult::getNumMUPs() const {
	return nMUPs_;
}

#endif /* __SIMULATOR_HEADER__ */

