#include <stdio.h>
#include <stdlib.h>

#include "mathtools.h"
#include "random.h"

#include "testutils.h"

#define	NPOINTS	10000
#define	RADIUS	10.0

int
testUniform(argc, argv)
	int argc;
	char **argv;
{
	int status = 1;
	int i;
	float val;
	FILE *fp;

	fp = fopen("uniform_points.txt", "w");

	for (i = 0; i < NPOINTS; i++)
	{
		val = floatNormalizedRandom();
		fprintf(fp, "%d %f\n", i, val);
	}

	fclose(fp);

	return(status);
}

