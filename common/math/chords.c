/**
 * ------------------------------------------------------------
 * Calculate areas of chords.
 *  ------------------------------------------------------------
 *  $Id: chords.c 44 2009-12-27 15:17:00Z andrew $
 */

#ifndef MAKEDEPEND
#include <stdio.h>
#include <math.h>
#endif

#include "os_defs.h"
#include "mathtools.h"
#include "massert.h"


OS_EXPORT double
calculateChordArea(
		double theta,
		double radius
    )
{
	double K;

	/*
	 *                        /.
	 *            ***********     .
	 *        ****         /|****   .
	 *       ***          / |..***    .
	 *     **            /  |.....**   .
	 *    *            r/   |.......*   .
	 *   *             /    |...K....*  .
	 *   *            /     c........*   .
	 *  *            /\     |.........*  .
	 *  *-----------+-t--d--+---h-----*  s
	 *  *            \/     |.........*  .
	 *   *            \     |........*   .
	 *   *             \    |........*  .
	 *    *             \   |.......*   .
	 *     **            \  |.... **   .
	 *       ***          \ |..***    .
	 *        ****         \|****   .
	 *            ***********     .
	 *                        \.
	 * (t = theta)
	 * s = r t
	 * d = r cos(t/2)
	 * h = r - d
	 * c = 2r sin(t/2)
	 * K = r^2 [ t - sin(t) ] / 2
	 */

	K = ((radius * radius) * ((theta - sin(theta)) / 2.0));

	/** calculate another way and compare */
	/*
	{
		double circleArea;
		double propCircle;
		double propCircleArea;
		double triangleArea;
		double c, d;
		double K2;

		circleArea = M_PI * radius * radius;
		propCircle = (theta / (2.0 * M_PI));
		propCircleArea = propCircle * circleArea;

		d = radius * cos(theta / 2.0);
		c = (2.0 * radius) * sin(theta/2.0);
		triangleArea = (c / 2.0) * d;

		K2 = propCircleArea - triangleArea;
		MSG_ASSERT(fabs(K - K2) < 0.00001,
				"chord calculation mismatch");
	}
	*/

	return K;
}


/**
 * To get the angle of subsection between two circles,
 * we first find the points of intersection of the
 * circle bounds, double the inner angle, and then
 * we can just use trig.
 *
 *            ***********   N
 *        ****           **oooooooooo
 *       ***            ooo/ \ ***....oooo
 *     **             oo  /   \   **......oo
 *    *             oo   /     \    *.......oo
 *   *             o    /A      \C    *........o
 *   *             o   /         \    *........o
 *  *             o   /c   B      \    *........o
 *  *-------------o--+-------------+---*--------o
 *  *             o  M\           /O   *........o
 *   *             o   \         /    *........o
 *   *             o    \       /     *........o
 *    *             oo   \     /    *.......oo
 *     **             oo  \   /   **......oo
 *       ***            ooo\ / ***....oooo
 *        ****           **oooooooooo
 *            ***********   P
 *
 *  (Lovely ASCII-art picture courtesy of Dr. Peterson, The Math Forum)
 *
 *
 * We know the lengths of the two radii A and C, and we can find
 * the length between them B.
 *
 * This gives us the length of three sides of the triangle M,N,O.
 *
 * We wish to find the angle c, which is 1/2 of the angle of
 * subsection.
 *
 * We do this simply by using the law of cosines:
 *
 *     C^2 = A^2 + B^2 - 2ABcos(c)
 *
 * rewriting it for c:
 *
 *     c = cos^-1( (A^2 + B^2 - C^2) / (2AB) )
 */
OS_EXPORT double
calculateAngleOfSubsectionByTwoCircles(
		double xCenterMain, double yCenterMain,
		double radiusMain,
		double xCenterSubsector, double yCenterSubsector,
		double radiusSubsector
    )
{
	double halfTheta;
	double lengthBetweenCentres;
	double diffX, diffY;
	double A, B, C;


	diffX = xCenterMain - xCenterSubsector;
	diffY = yCenterMain - yCenterSubsector;

	lengthBetweenCentres = sqrt((diffX * diffX) + (diffY * diffY));

	A = radiusMain;
	C = radiusSubsector;
	B = lengthBetweenCentres;

	/* c = cos^-1( (A^2 + B^2 - C^2) / (2AB) ) */
	halfTheta = acos((A * A + B * B - C * C) / (2.0 * A * B));

	return halfTheta * 2.0;
}

OS_EXPORT double
calculateAngleOfSubsectionByRadiiForTwoCircles(
		double separation,
		double radiusMain,
		double radiusSubsector
    )
{
	double halfTheta;
	double A, B, C;

	A = radiusMain;
	C = radiusSubsector;
	B = separation;

	/* c = cos^-1( (A^2 + B^2 - C^2) / (2AB) ) */
	halfTheta = acos((A * A + B * B - C * C) / (2.0 * A * B));

	return halfTheta * 2.0;
}

OS_EXPORT double
calculateAreaOfIntersectionOfTwoCircles(
		double xCenterA, double yCenterA,
		double radiusA,
		double xCenterB, double yCenterB,
		double radiusB
    )
{
	double thetaSubsectionCenteredInA, thetaSubsectionCenteredInB;
	double distanceBetweenCenters, xDiff, yDiff;
	double chordOfA, chordOfB;

	xDiff = xCenterA - xCenterB;
	yDiff = yCenterA - yCenterB;
	distanceBetweenCenters = sqrt(SQR(xDiff) + SQR(yDiff));

	/** if the circles do not intersect, the area is 0 */
	if (distanceBetweenCenters >= (radiusA + radiusB))
	{
		return 0;
	}
	/** if A is inside of B, area is that of A */
	if ((distanceBetweenCenters + radiusA) <= radiusB)
	{
		return M_PI * SQR(radiusA);
	}
	/** if B is inside of A, area is that of B */
	if ((distanceBetweenCenters + radiusB) <= radiusA)
	{
		return M_PI * SQR(radiusB);
	}
	/**
	 * Otherwise, we have a real intersection of circles
	 *
	 * determine the two relative angles of subsection
	 */
	thetaSubsectionCenteredInA =
					calculateAngleOfSubsectionByTwoCircles(
						xCenterA, yCenterA, radiusA,
						xCenterB, yCenterB, radiusB
		);

	thetaSubsectionCenteredInB =
			calculateAngleOfSubsectionByTwoCircles(
						xCenterB, yCenterB, radiusB,
						xCenterA, yCenterA, radiusA
		);

	chordOfA = calculateChordArea(
						thetaSubsectionCenteredInA,
						radiusA
		);

	chordOfB = calculateChordArea(
						thetaSubsectionCenteredInB,
						radiusB
		);

	return chordOfA + chordOfB;
}


OS_EXPORT double
calculateAreaOfIntersectionByRadiiOfTwoCircles(
		double separation,
		double radiusA,
		double radiusB
    )
{
    double area;

	/** if separation is greater than radii, there is no intersection */
	if (separation >= (radiusA + radiusB))
		return 0;

	/** now test for enclosure */
    if (separation <= fabs(radiusA-radiusB)) {
		/* if enclosed, return the area of the smaller circle */
        if (radiusA <= radiusB) {
            area = M_PI * SQR(radiusA);
        } else {
            area = M_PI * SQR(radiusB);
        }

    } else {
		/**
		 * we have incomplete intersection
		 */
        area = SQR(radiusB) * acos((SQR(separation) - SQR(radiusA) + SQR(radiusB))
					/ (2 * separation * radiusB))
			+ SQR(radiusA) * acos((SQR(separation) + SQR(radiusA) - SQR(radiusB))
			 		/ (2 * separation * radiusA))
			- 0.5 * sqrt(
					(-separation+radiusA+radiusB)
					* (separation-radiusA+radiusB)
					* (separation+radiusA-radiusB)
					* (separation+radiusA+radiusB)
				);
    }
    return(area);
}

