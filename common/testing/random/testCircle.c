#include <stdio.h>
#include <stdlib.h>

#include "mathtools.h"
#include "random.h"

#include "testutils.h"

#define	NPOINTS	10000
#define	RADIUS	10.0

int
testCircle(argc, argv)
	int argc;
	char **argv;
{
	int status = 1;
	int i;
	float rval, r, t,  x, y;
	FILE *fp;

	fp = fopen("circle_points.txt", "w");

	for (i = 0; i < NPOINTS; i++)
	{
		rval = floatNormalizedRandom();
		r = sqrt(rval) * RADIUS;

		t = floatNormalizedRandom() * M_PI * 2.0;

		x = CARTESIAN_X_FROM_POLAR(r, t);
		y = CARTESIAN_Y_FROM_POLAR(r, t);

		fprintf(fp, "%f %f %f %f\n", x, y, r, t);
	}

	fclose(fp);

	return(status);
}

