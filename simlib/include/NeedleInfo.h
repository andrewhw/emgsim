/**
 ** This class defines the needle location.
 **
 ** $Id: NeedleInfo.h 4 2008-04-24 21:27:41Z andrew $
 **/
#ifndef __NEEDLE_POSITION_CLASS_HEADER__
#define __NEEDLE_POSITION_CLASS_HEADER__

#ifndef MAKEDEPEND
# include <math.h>
#endif

#define DEFAULT_CANNULA_SLOPE   (float) (2.0 / 7.0)

/**
CLASS
		NeedleInfo

	This class defines the physical positioning of the
	needle.
 **/
class NeedleInfo
{
public:
		float xTip_;
		float yTip_;

		float xCannulaTerminus_;
		float yCannulaTerminus_;

		float z_;

		float cannulaSlope_;
		float cannulaLength_;

		float cannulaRadius_;

public:
		enum LineEquationChoice {
		                BelowTip,
		                AboveTip_AboveCannula,
		                AboveTip_BelowCannula
		        };
public:
		////////////////////////////////
		// create a needle tip at a base position
		NeedleInfo(
		        float xTipPosition,
		        float yTipPosition,
		        float zTipPosition,
		        float lengthInMM,
		        float radius,
		        float slope
		    );

		////////////////////////////////
		// null constructor -- creates an empty needl
		// record
		NeedleInfo();

		////////////////////////////////
		// Copy Constructor    (Added Aug 2, 2002)
		NeedleInfo(const NeedleInfo& tmpNeedleInfo);

		////////////////////////////////
		// Destructor
		~NeedleInfo();

public:
		////////////////////////////////
		// return the X position of the needle tip
		float getXTipInMM() const;

		////////////////////////////////
		// return the Y position of the needle tip
		float getYTipInMM() const;

		////////////////////////////////
		// return the X position of the
		// end of the (modelled) cannula
		float getXCannulaTerminusInMM() const;

		////////////////////////////////
		// return the X position of the
		// end of the (modelled) cannula
		float getYCannulaTerminusInMM() const;

		////////////////////////////////
		// return the Z position of the needle
		// (the entire shaft has the same Z position)
		float getZInMM() const;

		////////////////////////////////
		// return the slope of the cannula
		float getSlope() const;

		////////////////////////////////
		// return the length of the cannula
		float getCannulaLengthInMM() const;

		////////////////////////////////
		// return the radius of the cannula
		float getRadiusInMicrons() const;

		////////////////////////////////
		// save our state to disk
		int store(const char *filename) const;

		////////////////////////////////
		// load from a file
		int load(const char *filename);

		////////////////////////////////
		// sensible defaults
		void setDefaultValues(void);

		////////////////////////////////
		// set all the relevant values
		void set(
		        float xTipPosition,
		        float yTipPosition,
		        float zTipPosition,
		        float lengthInMM,
		        float radius,
		        float slope
		    );

		////////////////////////////////
		// return true if any values differ
		int isDifferent(
		        float xTipPosition,
		        float yTipPosition,
		        float zTipPosition,
		        float lengthInMM,
		        float radius,
		        float slope
		    ) const;

		////////////////////////////////
		// calculation tools
		double getNeedleXCrossSectionDistanceInMM(void) const;
		double getNeedleYCrossSectionDistanceInMM(void) const;

		void getNeedleDistances(
		        double *distanceToTipInMM,
		        double *distanceToEndInMM,
		        double *distanceToShaftInMM,
		        float fibreLocXInMM,
		        float fibreLocYInMM
		    );

		////////////////////////////////
		// get the distance to the needle
		// under a fixed shaft projection
		void getProjectedNeedleDistances(
		        double *xProjDeltaToTipInMM,
		        double *xProjDeltaToEndInMM,
		        double *distanceToShaftInMM,
		        float fibreLocXInMM,
		        float fibreLocYInMM
		    );

		void getNeedleLocations(
		        double *xTipInMM,
		        double *yTipInMM,
		        double *xEndInMM,
		        double *yEndInMM,
		        LineEquationChoice lineChoice
		    );

		void getLineEquation(
		        double *M,
		        double *B,
		        LineEquationChoice lineChoice
		    );
};

inline float NeedleInfo::getXTipInMM() const
{
	return xTip_;
}

inline float NeedleInfo::getYTipInMM() const
{
	return yTip_;
}

inline float NeedleInfo::getXCannulaTerminusInMM() const
{
	return xCannulaTerminus_;
}

inline float NeedleInfo::getYCannulaTerminusInMM() const
{
	return yCannulaTerminus_;
}
inline float NeedleInfo::getZInMM() const
{
	return z_;
}

inline float NeedleInfo::getSlope() const
{
	return cannulaSlope_;
}

inline float NeedleInfo::getCannulaLengthInMM() const
{
	return cannulaLength_;
}

inline float NeedleInfo::getRadiusInMicrons() const
{
	return cannulaRadius_;
}

#endif

