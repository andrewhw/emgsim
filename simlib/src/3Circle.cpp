/**
 ** This is a numerical method for calculating the area
 ** of intersection between the circles.
 ** It finds the smallest circle and then Counts the
 ** number of the points in the smallest circle which
 ** are common for the other circles too.
 ** This way, there would be an approximation of the area
 ** of intersection between circles.
 ** resolution :
 ** The parameter which determines how close this approximation
 ** to the real area value would be.
 **
 **
 ** $Id: 3Circle.cpp 4 2008-04-24 21:27:41Z andrew $
 **/
#include "3Circle.h"
#include "mathtools.h"

OS_EXPORT double
calculateAreaOfIntersectionOfThreeCircles(
  		double xCentreM, double yCentreM,
		double radiusM,
		double xCentreA, double yCentreA,
		double radiusA,
		double xCentreB, double yCentreB,
		double radiusB
	)
{

	double x, y;
	int RowIndex = 0;
	double Circ[3][3] = { {xCentreM, yCentreM, radiusM},
							{xCentreA, yCentreA, radiusA},
							{xCentreB, yCentreB, radiusB}
						};
	int i, j;
	int resolution = 1000;
	int AreaCount = 0;
	double CommonArea;


	if (radiusM < radiusA){
		if (radiusM < radiusB){
			RowIndex = 0;
		}else{
			RowIndex = 2;
		}
	}else{
		if (radiusA < radiusB){
			RowIndex = 1;
		}else{
			RowIndex = 2;
		}
	}

	for (i = 0 ; i < resolution ; i++){
		for (j = 0 ; j < resolution ; j++){

			x = (double) Circ[RowIndex][0] - Circ[RowIndex][2] + i * Circ[RowIndex][2] * 2.0 / resolution;
			y = (double) Circ[RowIndex][1] - Circ[RowIndex][2] + j * Circ[RowIndex][2] * 2.0 / resolution;

			if (
				(((x-xCentreM)*(x-xCentreM) + (y-yCentreM)*(y-yCentreM)) < radiusM*radiusM) &
				(((x-xCentreA)*(x-xCentreA) + (y-yCentreA)*(y-yCentreA)) < radiusA*radiusA) &
				(((x-xCentreB)*(x-xCentreB) + (y-yCentreB)*(y-yCentreB)) < radiusB*radiusB)
				){

				AreaCount++;
			}
		}
	}

	CommonArea = (double) AreaCount * 4.0 * Circ[RowIndex][2] * Circ[RowIndex][2] / (resolution*resolution);

	return CommonArea;
}

