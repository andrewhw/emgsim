/**
 ** MUP class -- store, recover and combine MFP's into MUPs
 ** with Jitter.
 **
 ** $Id: NeedleInfo.cpp 4 2008-04-24 21:27:41Z andrew $
 **/

#include "os_defs.h"

# ifndef    MAKEDEPEND
#  include    <stdio.h>
#  include    <math.h>
# endif

#include "NeedleInfo.h"
#include "attvalfile.h"
#include "pathtools.h"
#include "mathtools.h"
#include "massert.h"
#include "tclCkalloc.h"


NeedleInfo::NeedleInfo(
		float xTipPosition,
		float yTipPosition,
		float zTipPosition,
		float lengthInMM,
		float radius,
		float slope
	)
{
	set(xTipPosition, yTipPosition, zTipPosition,
		    lengthInMM, radius, slope);
}


NeedleInfo::NeedleInfo()
{
	xTip_ = 0;
	yTip_ = 0;
	z_ = 0;

	cannulaSlope_ = 0;
	cannulaLength_ = 0;

	xCannulaTerminus_ = 0;
	yCannulaTerminus_ = 0;
}

// Added Aug 2, 2002
NeedleInfo::NeedleInfo(const NeedleInfo& tmpNeedleInfo)
{
	xTip_ = tmpNeedleInfo.getXTipInMM();
	yTip_ = tmpNeedleInfo.getYTipInMM();
	z_ = tmpNeedleInfo.getZInMM();

	cannulaSlope_ = tmpNeedleInfo.getSlope();
	cannulaLength_ = tmpNeedleInfo.getCannulaLengthInMM();
	cannulaRadius_ = tmpNeedleInfo.getRadiusInMicrons();

	xCannulaTerminus_ = tmpNeedleInfo.getXCannulaTerminusInMM();
	yCannulaTerminus_ = tmpNeedleInfo.getYCannulaTerminusInMM();
}

NeedleInfo::~NeedleInfo()
{
}

void NeedleInfo::setDefaultValues(void)
{
	set(0, 0, 0, 10, 250.0, (float) DEFAULT_CANNULA_SLOPE);
}

void NeedleInfo::set(
		float xTipPosition,
		float yTipPosition,
		float zTipPosition,
		float lengthInMM,
		float radius,
		float slope
	)
{
	double theta;

	memset(this, 0, sizeof(*this));

	xTip_ = xTipPosition;
	yTip_ = yTipPosition;
	z_ = zTipPosition;

	cannulaSlope_ = slope;
	cannulaLength_ = lengthInMM;
	cannulaRadius_ = radius;

	/**
	 * now calculate where the other end of the
	 * shaft must be
	 *
	 *       /|
	 *     R/ | Y
	 *     /__|
	 *       X
	 *  known : slope = (Y/X), R
	 *
	 *    Y/X = tan(theta) -> theta = arctan(slope)
	 *    Y/R = sin(theta)
	 *    Y   = sin(theta) * R
	 *    X   = cos(theta) * R
	 */
	theta = atan(cannulaSlope_);

	xCannulaTerminus_ = (float) (xTip_ + (cos(theta) * cannulaLength_));
	yCannulaTerminus_ = (float) (yTip_ + (sin(theta) * cannulaLength_));
}

int NeedleInfo::isDifferent(
		float xTipPosition,
		float yTipPosition,
		float zTipPosition,
		float lengthInMM,
		float radius,
		float slope
	) const
{
	if (xTip_ != xTipPosition)				return 1;
	if (yTip_ != yTipPosition)				return 1;
	if (z_ != zTipPosition)				return 1;

	if (cannulaSlope_ != slope)				return 1;
	if (cannulaLength_ != lengthInMM)		return 1;
	if (cannulaRadius_ != radius)		return 1;

	return 0;
}


/**
 * The amount B will be above or below the
 * origin will be
 *     (radius in MM) / | cos ( -1 / slope ) |
 *
 *                    /m
 *                -- /-----
 *                m /|
 *                 /1|         /          /
 *                / -|        /          /
 *               /  m|       /          /
 *              /    |      /          /
 *             />    |Y    /          /
 *            / \    |    /          /
 *           /   \   |   /          /
 *          /    R\  |  /          /
 *         /       \b| /          /
 *        /m        \|/          /
 *        -----------0-----------
 *
 *   m is the needle slope
 *   arctan(m) gives us the angle at m,
 *       arctan(1/m) gives us the angle between m and the vertical.
 *
 *   .'. arctan(1/m) gives us an angle by which we can relate
 *       the cannula radius (R) to the desired hypotenuse (Y) via sin()
 *
 *       sin( arctan(1/m) )  =  R/Y
 *                        Y  =  R / sin( arctan(1/m) )
 */
double NeedleInfo::getNeedleYCrossSectionDistanceInMM(void) const
{
	return ( (getRadiusInMicrons() / 1000.0)
		        / sin( atan( 1 / getSlope() ) ) ) * 2.0;
}

double NeedleInfo::getNeedleXCrossSectionDistanceInMM(void) const
{
	return getNeedleYCrossSectionDistanceInMM() / getSlope();
}

void NeedleInfo::getLineEquation(
		double *M,
		double *B,
		LineEquationChoice lineChoice
	)
{
	float B_cannulaCentre;

	B_cannulaCentre = yTip_ - (getSlope() * xTip_);

	/** if we are below tip, then everything is simple */
	if (lineChoice == BelowTip)
	{

		*M = getSlope();
		*B = B_cannulaCentre;

	} else if (lineChoice == AboveTip_AboveCannula)
	{
		*M = getSlope();
		*B = B_cannulaCentre + (getNeedleYCrossSectionDistanceInMM() / 2.0);

	} else if (lineChoice == AboveTip_BelowCannula)
	{
		*M = getSlope();
		*B = B_cannulaCentre - (getNeedleYCrossSectionDistanceInMM() / 2.0);


	} else
	{
		MSG_ASSERT(0, "Invalid choice for lineEquation type");
	}
}

void NeedleInfo::getNeedleLocations(
		double *xTipInMM,
		double *yTipInMM,
		double *xEndInMM,
		double *yEndInMM,
		LineEquationChoice lineChoice
	)
{
	double M_adjustedCannula;
	double B_adjustedCannula;
	double xDifferenceAtTip;
	double yDifferenceAtTip;
	double xLocalTipInMM, yLocalTipInMM;

	/**
	 * get the line equation for the given choice
	 */
	getLineEquation(
		        &M_adjustedCannula,
		        &B_adjustedCannula,
		        lineChoice);


	/**
	 * Now all the calculations to the ends and offset of shaft
	 * are the same, regardless which co-ordinate system we are
	 * using.
	 */


	/**
	 * Figure out tip location -- the tip is flat on the bottom,
	 * so we assume that y_interesting == yTip_, and figure out
	 * where X is based on the equation of the line we are
	 * interested in.
	 */
	yLocalTipInMM = yTip_;
	xLocalTipInMM = ((yLocalTipInMM - B_adjustedCannula) / M_adjustedCannula);

	if (yTipInMM != NULL)
		(*yTipInMM) = yLocalTipInMM;

	if (xTipInMM != NULL)
		(*xTipInMM) = xLocalTipInMM;

	/**
	 * adjust the top of the needle by the same amount as
	 * the amount the bottom of the needle has been adjusted by,
	 * in order to keep the sides of the needle parallel
	 */
	xDifferenceAtTip = xLocalTipInMM - xTip_;
	yDifferenceAtTip = yLocalTipInMM - yTip_;

	if (xEndInMM != NULL)
		(*xEndInMM) = xCannulaTerminus_ + xDifferenceAtTip;
	if (yEndInMM != NULL)
		(*yEndInMM) = yCannulaTerminus_ + yDifferenceAtTip;
}


void NeedleInfo::getProjectedNeedleDistances(
		double *projDeltaToTipInMM,
		double *projDeltaToEndOfShaftInMM,
		double *distanceToShaftInMM,
		float fibreLocXInMM,
		float fibreLocYInMM
	)
{
	double M_cannulaPerpindicular;
	double B_cannulaPerpindicular;
	double M_cannula;
	double B_cannula;
	double distanceXInMM, distanceYInMM;
	double xTipInMM, yTipInMM;
	double xEndInMM, yEndInMM;
	double xIntersection;
	double yIntersection;


	/**
	 *  We do everything in this model relative
	 *  to an unmoving central line equation,
	 *  which is the same as "BelowTip" in the
	 *  other model
	 */
	M_cannula = getSlope();
	B_cannula = getYTipInMM();


	/**
	 * get the points at the end of the needle
	 */
	xTipInMM = getXTipInMM();
	yTipInMM = getYTipInMM();
	xEndInMM = getXCannulaTerminusInMM();
	yEndInMM = getYCannulaTerminusInMM();


	/**
	 * calculate distance to shaft line & calculate
	 * the intersection bewteen this line and the
	 * cannula
	 * --
	 * raise a perpendicular throuch fibreLoc
	 * to calculate the distance to the (adjusted)
	 * line of the shaft
	 */

	/**
	 *  y = M x + B
	 *  B = y - ( M x )
	 */
	M_cannulaPerpindicular = (-1.0 / getSlope());
	B_cannulaPerpindicular =
		    fibreLocYInMM - M_cannulaPerpindicular * fibreLocXInMM;


	/**
	 *  M_c x_intersection + B_c = M_p x_intersection + B_p
	 *                 B_c - B_p = x_intersection ( M_p - M_c )
	 *          x_intersection = (B_c - B_p) / (M_p - M_c)
	 */
	xIntersection = (B_cannula - B_cannulaPerpindicular)
		        / (M_cannulaPerpindicular - M_cannula);
	yIntersection = M_cannula * xIntersection + B_cannula;


	distanceXInMM = xIntersection - fibreLocXInMM;
	distanceYInMM = yIntersection - fibreLocYInMM;
	(*distanceToShaftInMM) = sqrt(SQR(distanceXInMM) + SQR(distanceYInMM));


	/**
	 * Now use the intersection to calculate the delta projected
	 * along the shaft to determine the relative fibre location
	 *
	 * The intersection point x,y is used as the projection onto
	 * the cannula line of the fibre position.  From this, we
	 * can generate a delta in the "X" direction of the cannula
	 * co-ordinates to get the projected "X" we desire for the
	 * current weight function equation.
	 */

	/** calculate distance to Tip */
	distanceXInMM = xTipInMM - xIntersection;
	distanceYInMM = yTipInMM - yIntersection;

	(*projDeltaToTipInMM) =
		        sqrt( SQR(distanceXInMM) + SQR(distanceYInMM) );

	if (distanceXInMM < 0)
		(*projDeltaToTipInMM) = (*projDeltaToTipInMM) * (-1);



	/** calculate distance to End */
	distanceXInMM = xEndInMM - xIntersection;
	distanceYInMM = yEndInMM - yIntersection;

	(*projDeltaToEndOfShaftInMM) =
		        sqrt( SQR(distanceXInMM) + SQR(distanceYInMM) );

	if (distanceXInMM < 0)
		(*projDeltaToEndOfShaftInMM) =
		        (*projDeltaToEndOfShaftInMM) * (-1);
}

void NeedleInfo::getNeedleDistances(
		double *distanceToTipInMM,
		double *distanceToEndOfShaftInMM,
		double *distanceToShaftInMM,
		float fibreLocXInMM,
		float fibreLocYInMM
	)
{
	double M_cannulaPerpindicular;
	double B_cannulaPerpindicular;
	double M_adjustedCannula;
	double B_adjustedCannula;
	double distanceXInMM, distanceYInMM;
	double xAdjustedTipInMM, yAdjustedTipInMM;
	double xAdjustedEndInMM, yAdjustedEndInMM;
	double xIntersection;
	double yIntersection;
	double testY;
	LineEquationChoice lineChoice;


	/** if we are below tip, then everything is simple */
	if (fibreLocYInMM < yTip_)
	{

		lineChoice = BelowTip;

	} else
	{

		/**
		 * calulate a test point at fibre X on the cannula,
		 * check if the fibre is above or below the point
		 * to see if the fibre is above or below the cannula
		 */
		testY = getSlope() * fibreLocXInMM;

		if (fibreLocYInMM > testY)
		{
		    lineChoice = AboveTip_AboveCannula;
		} else
		{
		    lineChoice = AboveTip_BelowCannula;
		}
	}

	/**
	 * get the line equation for the needle, which we
	 * can use to calculate distances for the given point
	 * to the shaft
	 */
	getLineEquation(
		        &M_adjustedCannula,
		        &B_adjustedCannula,
		        lineChoice
		    );

	/**
	 * get the points at the end of the needle, for the
	 * given lineChoice
	 */
	getNeedleLocations(
		        &xAdjustedTipInMM, &yAdjustedTipInMM,
		        &xAdjustedEndInMM, &yAdjustedEndInMM,
		        lineChoice
		    );

	/**
	 * Now all the calculations to the ends and offset of shaft
	 * are the same
	 */
	{
		/** calculate distance to Tip */

		distanceXInMM = fibreLocXInMM - xAdjustedTipInMM;
		distanceYInMM = fibreLocYInMM - yAdjustedTipInMM;

		(*distanceToTipInMM) =
		        sqrt( SQR(distanceXInMM) + SQR(distanceYInMM) );


		/** calculate distance to End */
		distanceXInMM = fibreLocXInMM - xAdjustedEndInMM;
		distanceYInMM = fibreLocYInMM - yAdjustedEndInMM;

		(*distanceToEndOfShaftInMM) =
		        sqrt( SQR(distanceXInMM) + SQR(distanceYInMM) );
	}


	/**
	 * calculate distance to shaft line
	 * --
	 * raise a perpendicular throuch fibreLoc
	 * to calculate the distance to the (adjusted)
	 * line of the shaft
	 */
	{

		/**
		 *  y = M x + B
		 *  B = y - ( M x )
		 */
		M_cannulaPerpindicular = (-1.0 / getSlope());
		B_cannulaPerpindicular =
		        fibreLocYInMM - M_cannulaPerpindicular * fibreLocXInMM;


		/**
		 *  M_c x_intersection + B_c = M_p x_intersection + B_p
		 *                 B_c - B_p = x_intersection ( M_p - M_c )
		 *          x_intersection = (B_c - B_p) / (M_p - M_c)
		 */
		xIntersection =
		            (B_adjustedCannula - B_cannulaPerpindicular)
		            / (M_cannulaPerpindicular - M_adjustedCannula);
		yIntersection =
		            M_adjustedCannula * xIntersection
		                    + B_adjustedCannula;


		distanceXInMM = xIntersection - fibreLocXInMM;
		distanceYInMM = yIntersection - fibreLocYInMM;
		(*distanceToShaftInMM) = sqrt(
		                    SQR(distanceXInMM) + SQR(distanceYInMM));
	}
}

int NeedleInfo::store(const char *filename) const
{
	struct attValList *list;
	FILE *ofp;

	ofp = fopenpath(filename, "w");
	if (ofp == NULL)
	{
		return 0;
	}

	list = (struct attValList *) ckalloc(sizeof(struct attValList));
	memset(list, 0, sizeof(struct attValList));

	addAttVal(list, createRealAttribute("xTip_mm", xTip_));
	addAttVal(list, createRealAttribute("yTip_mm", yTip_));

	addAttVal(list,
		        createRealAttribute("xCannulaTerminus_mm",
		        xCannulaTerminus_)
		    );
	addAttVal(list,
		        createRealAttribute("yCannulaTerminus_mm",
		        yCannulaTerminus_)
		    );

	addAttVal(list, createRealAttribute("z_mm", z_));
	addAttVal(list, createRealAttribute("cannulaSlope", cannulaSlope_));
	addAttVal(list,
		        createRealAttribute("cannulaLength_mm",
		        cannulaLength_)
		    );
	addAttVal(list,
		        createRealAttribute("cannulaRadius_um",
		        cannulaRadius_));

	writeAttValList(ofp, list, "Needle Information File");

	fclose(ofp);
	deleteAttValList(list);
	return 1;
}

int NeedleInfo::load(const char *filename)
{
	struct attValList *list;
	struct attVal *item;
	char *workingname;

	setDefaultValues();

	workingname = osIndependentPath(filename);
	list = loadAttValFile(workingname);
	ckfree(workingname);
	if (list == NULL)
		return 0;


	item = getAttVal(list, "xTip_mm");
	if (item != NULL)
		xTip_ = (float) item->data_.dval_;

	item = getAttVal(list, "yTip_mm");
	if (item != NULL)
		yTip_ = (float) item->data_.dval_;



	item = getAttVal(list, "xCannulaTerminus_mm");
	if (item != NULL)
		xCannulaTerminus_ = (float) item->data_.dval_;

	item = getAttVal(list, "yCannulaTerminus_mm");
	if (item != NULL)
		yCannulaTerminus_ = (float) item->data_.dval_;



	item = getAttVal(list, "z_mm");
	if (item != NULL)
		z_ = (float) item->data_.dval_;



	item = getAttVal(list, "cannulaSlope");
	if (item != NULL)
		cannulaSlope_ = (float) item->data_.dval_;

	item = getAttVal(list, "cannulaLength_mm");
	if (item != NULL)
		cannulaLength_ = (float) item->data_.dval_;

	item = getAttVal(list, "cannulaRadius_um");
	if (item != NULL)
		cannulaRadius_ = (float) item->data_.dval_;

	deleteAttValList(list);
	return 1;
}


