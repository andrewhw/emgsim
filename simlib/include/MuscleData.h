/**
 ** This class holds the definition of a muscle data structure
 **
 ** $Id: MuscleData.h 4 2008-04-24 21:27:41Z andrew $
 **/
#ifndef __MUSCLE_DATA_CLASS_HEADER__
#define __MUSCLE_DATA_CLASS_HEADER__

#include "os_defs.h"

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

#define		CELLS_PER_MM						20.0f
#define		UM_PER_CELL						((1.0 / CELLS_PER_MM) * 1000.0)

#ifndef PRIVATE
#define PRIVATE private
#endif

class MotorUnit;
class MuscleFibre;
class NeedleInfo;

struct Node;

/**
CLASS
		MuscleData

	This class holds the physical definition of the
	current muscle.

	See also MotorUnit and MuscleFibre.
 **/
class MuscleData
{
PRIVATE:
		void allocateNMotorUnits(int nMotorUnits);

private:
		int readMotorUnit(
		        FILE *fp,
		        MotorUnit **masterMUList
		    );

		int loadMFdata(const char *filename);
		int loadMUdata(const char *filename);
		int loadAMUdata(const char *filename);
		int loadAMUInDetectData(const char *filename);

public:
		////////////////////////////////
		// Create a muscle data object
		MuscleData();

		////////////////////////////////
		// Destructor
		~MuscleData();

public:
		////////////////////////////////
		// return the width of a quadrant of the detection area
		int getXDetect() const;

		////////////////////////////////
		// return the height of a quadrant of the detection area
		int getYDetect() const;

		////////////////////////////////
		// return the number of motor units recorded
		int getNumMotorUnits() const;

		////////////////////////////////
		// return the number of motor units in the detection area
		int getNumMotorUnitsInDetectionArea() const;

		////////////////////////////////
		// return the diameter of the overall muscle
		float getMuscleDiameter() const;

		////////////////////////////////
		// return the max possible motor unit territory diameter
		float getMaxMotorUnitDiameter() const;

		////////////////////////////////
		// return the min possible motor unit territory diameter
		float getMinMotorUnitDiameter() const;

		////////////////////////////////
		// return a motor unit which intersects the
		// detection area
		MotorUnit *getMotorUnitInDetectionArea(int index) const;

		////////////////////////////////
		// return a motor unit from the master motor unit list
		MotorUnit *getMotorUnitFromMasterList(int index) const;

		////////////////////////////////
		// get the count of the active (recorded)
		// motor units
		int getNumActiveMotorUnits() const;

		////////////////////////////////
		// get one of the active (recorded)
		// motor units
		MotorUnit *getActiveMotorUnit(int index) const;

		////////////////////////////////
		// get the count of the active (recorded)
		// motor units
		int getNumActiveInDetectMotorUnits() const;

		////////////////////////////////
		// get one of the active (recorded)
		// motor units
		MotorUnit *getActiveInDetectMotorUnit(int index) const;

		////////////////////////////////
		// return the neelde position info structure
		NeedleInfo *getNeedleInfo() const;

		////////////////////////////////
		// write the output file for the muscle fibre data
		int writeMFInfo(const char *filename) const;

		////////////////////////////////
		// write the output file for number of fibers per mu
		int writeMUParam(const char *filename) const;

		////////////////////////////////
		// write the output file for needle related info per mu
		int writeNeedleRelatedMUInfo(const char *filename) const;

		////////////////////////////////
		// write the output file for the motor unit data
		int writeMUInfo(const char *filename) const;

		////////////////////////////////
		// write an output file to record the
		// motor unit ids which are in the detection
		// area and are active
		int writeAMUInDetectInfo(const char *filename) const;

		////////////////////////////////
		// load the persisted data
		int loadData(
		        const char *muscleDir,
		        const char *outputDir = NULL,
				int contractionId = 1,
				int usePlowedFibres = 0
		    );

		////////////////////////////////
		// determine if a particular motor unit is active
		int isMotorUnitActive(MotorUnit *pMotorUnit);


		////////////////////////////////
		// add a fibre (which will also be in a motor unit)
		// to the master list of fibres for easy index-based
		// access
		//
		// Return value is the index of the new fibre in
		// the list, beginning at 0
		int addFibre(MuscleFibre *fibre);

		////////////////////////////////
		// remove a given fibre from the master
		// list.  Note that this does not move the
		// list down; it will insert a NULL into the
		// list itself as a tombstone (this is to
		// ensure that other indexes into this list
		// remain valid).
		int removeFibre(MuscleFibre *fibre);

		////////////////////////////////
		// return the size of the master fibre list
		int getTotalNumberOfFibres() const;

		////////////////////////////////
		// return the max size of the (healthy) master fibre list
		int getOriginalNumberOfFibres() const;

		////////////////////////////////
		// return a fibre from the master list by index
		MuscleFibre *getFibre(int index) const;

		////////////////////////////////
		// Get the Fibre RTree
		Node *getFibreRTreeRoot() const;

public:
		const static int PRINT_MU_DATA;
		const static int PRINT_FIBRE_DATA;
		const static int PRINT_FULL_MU_LIST_DATA;
		const static int PRINT_ALL_DATA;

		////////////////////////////////
		// dump contents to file pointer
		int dump(FILE *fp, int printFlags = 0x00) const;

		////////////////////////////////
		// Check that things are still ok
		int validate() const;

PRIVATE:
		int xDetect_;
		int yDetect_;

		int nMotorUnitsInMuscle_;
		MotorUnit **motorUnit_;

		int nMotorUnitsInDetectionArea_;
		float muscleDiameter_;
		float maxMotorUnitDiameter_;
		float minMotorUnitDiameter_;

		MotorUnit **motorUnitInDetect_;
		int motorUnitInDetectBlocks_;

		MotorUnit **activeMotorUnit_;
		int nActiveMotorUnits_;
		int nActiveMotorUnitBlocks_;

		MotorUnit **activeInDetectMotorUnit_;
		int nActiveInDetectMotorUnits_;
		int nActiveInDetectMotorUnitBlocks_;

		NeedleInfo *needle_;

		Node *fibreRTreeRoot_;

		int nTotalFibres_;
		int nMaxFibres_;

		int nFibreBlocks_;
		MuscleFibre **masterFibreList_;
};

inline int MuscleData::getXDetect() const {
	return xDetect_;
}

inline int MuscleData::getYDetect() const {
	return yDetect_;
}

inline int MuscleData::getNumMotorUnits() const {
	return nMotorUnitsInMuscle_;
}

inline int MuscleData::getNumMotorUnitsInDetectionArea() const {
	return nMotorUnitsInDetectionArea_;
}

inline MotorUnit *MuscleData::getMotorUnitInDetectionArea(
		int index
	) const {
	return motorUnitInDetect_[index];
}

inline MotorUnit *MuscleData::getMotorUnitFromMasterList(
		int index
	) const {
	return motorUnit_[index];
}


inline int MuscleData::getNumActiveMotorUnits() const {
	return nActiveMotorUnits_;
}

inline MotorUnit *MuscleData::getActiveMotorUnit(
		int index
	) const {
	return activeMotorUnit_[index];
}


inline int MuscleData::getNumActiveInDetectMotorUnits() const {
	return nActiveInDetectMotorUnits_;
}

inline MotorUnit *MuscleData::getActiveInDetectMotorUnit(
		int index
	) const {
	return activeInDetectMotorUnit_[index];
}

inline float MuscleData::getMuscleDiameter() const {
	return muscleDiameter_;
}

inline float MuscleData::getMaxMotorUnitDiameter() const {
	return maxMotorUnitDiameter_;
}

inline float MuscleData::getMinMotorUnitDiameter() const {
	return minMotorUnitDiameter_;
}

inline NeedleInfo *MuscleData::getNeedleInfo() const {
	return needle_;
}

inline int MuscleData::getTotalNumberOfFibres() const {
	return nTotalFibres_;
}

inline int MuscleData::getOriginalNumberOfFibres() const {
	return nMaxFibres_;
}

inline MuscleFibre *MuscleData::getFibre(int index) const {
	return masterFibreList_[index];
}

inline Node *MuscleData::getFibreRTreeRoot() const {
	return fibreRTreeRoot_;
}

////////////////////////////////////////////////////////////////

/**
CLASS
		MotorUnit

	This class defines the location and fibres for
	a single motor unit.

	See also MuscleFibre and MuscleData.
 **/
class MotorUnit
{
public:
		////////////////////////////////
		// Create a muscle data object
		MotorUnit();

		////////////////////////////////
		// Destructor
		~MotorUnit();

public:
		////////////////////////////////
		// return the number of motor units recorded
		int getNumFibres() const;

		////////////////////////////////
		// return the number of motor units recorded
		MuscleFibre *getFibre(int id) const;

		////////////////////////////////
		// return the motor unit id we were generated with
		int getID() const;

		////////////////////////////////
		// return the motor unit territory diameter
		float getDiameter() const;

		////////////////////////////////
		// return the motor unit territory radius
		float getRadius() const;

		////////////////////////////////
		// return the motor unit territory radial position in the muscle
		float getLocationRadius() const;

		////////////////////////////////
		// return the motor unit territory theta co-ordinate
		// in the muscle
		float getLocationTheta() const;

		////////////////////////////////
		// return the motor unit X location in MM
		float getXLocationInMM() const;

		////////////////////////////////
		// return the motor unit Y location in MM
		float getYLocationInMM() const;

		////////////////////////////////
		// return the number of firings
		int getNumFirings() const;

		////////////////////////////////
		// return the firing time N of getNumFirings()
		long getFiringTimeN(int index) const;

PRIVATE:
		////////////////////////////////
		// add a new fibre to this MU
		int addFibre(MuscleFibre *newFibre);

		////////////////////////////////
		// place a new centroid based on the average
		// of our fibre locations
		int recalculateCentroid();

		////////////////////////////////
		// remove a fibre from this MU
		int removeFibre(MuscleFibre *newFibre);

		////////////////////////////////
		// remove a fibre from this MU by index
		int removeFibre(int index);

		////////////////////////////////
		// Check that things are still ok
		int validate() const;

PRIVATE:
		int mu_id_;

		float mu_loc_r_mm_;
		float mu_loc_theta_;
		float mu_diameter_mm_;
		float mu_expected_diameter_mm_;

		int mu_nHealthyFibres_;
		int mu_nFibres_;
		int mu_nFibreAllocationBlocks_;
		MuscleFibre **mu_fibre_;

		long *mu_firingTime_;
		int mu_nFirings_;
		int mu_nFiringBlocks_;

		int mu_expectedNumFibres_;
};

inline int MotorUnit::getID() const {
	return mu_id_;
}

inline float MotorUnit::getLocationRadius() const {
	return mu_loc_r_mm_;
}

inline float MotorUnit::getLocationTheta() const {
	return mu_loc_theta_;
}

inline float MotorUnit::getDiameter() const {
	return mu_diameter_mm_;
}

inline float MotorUnit::getRadius() const {
	return (float) (mu_diameter_mm_ / 2.0);
}

inline int MotorUnit::getNumFibres() const {
	return mu_nFibres_;
}

inline MuscleFibre *MotorUnit::getFibre(int id) const {
	return mu_fibre_[id];
}

inline int MotorUnit::getNumFirings() const {
	return mu_nFirings_;
}

inline long MotorUnit::getFiringTimeN(int id) const {
	return mu_firingTime_[id];
}

////////////////////////////////////////////////////////////////


/**
CLASS
		MuscleFibre

	This class defines the location, motor unit,
	neural attachment point in Z (jShift) and
	diameter of a single muscle fibre

	See also MotorUnit and MuscleData.
 **/
class MuscleFibre
{
public:
		////////////////////////////////
		// Create a muscle data object
		MuscleFibre();

		////////////////////////////////
		// Create a muscle data object
		MuscleFibre(float xCell, float yCell);

		////////////////////////////////
		// Destructor
		~MuscleFibre();

public:
		////////////////////////////////
		// return the X position of the cell this fibre is in
		float getXCell() const;

		////////////////////////////////
		// return the X position of the cell this fibre is in
		float getYCell() const;

		////////////////////////////////
		// return the X position of the fibre in MM
		float getXLocationInMM() const;

		////////////////////////////////
		// return the X position of the fibre in MM
		float getYLocationInMM() const;

		////////////////////////////////
		// Set a new cell location -- used in plowing
		void setCellLocation(float newX, float newY);

		////////////////////////////////
		// return the neuro-muscular-junction shift
		float getJShift() const;

		////////////////////////////////
		// return the diameter
		float getDiameter() const;

		////////////////////////////////
		// return the motor unit we are attached to
		int getMotorUnit() const;

		////////////////////////////////
		// Check that things are still ok
		int validate() const;

PRIVATE:
		int mf_motorUnit_;

		float mf_xCell_;
		float mf_yCell_;

		float mf_jShift_;
		float mf_diameter_;
		float mf_healthyDiameter_;
};

inline float MuscleFibre::getXCell() const {
	return mf_xCell_;
}

inline float MuscleFibre::getYCell() const {
	return mf_yCell_;
}

inline float MuscleFibre::getXLocationInMM() const {
	return getXCell() / CELLS_PER_MM;
}

inline float MuscleFibre::getYLocationInMM() const {
	return getYCell() / CELLS_PER_MM;
}

inline void MuscleFibre::setCellLocation(float newX, float newY) {
	mf_xCell_ = newX;
	mf_yCell_ = newY;
}

inline float MuscleFibre::getJShift() const {
	return mf_jShift_;
}

inline float MuscleFibre::getDiameter() const {
	return mf_diameter_;
}

inline int MuscleFibre::getMotorUnit() const {
	return mf_motorUnit_;
}

#endif

