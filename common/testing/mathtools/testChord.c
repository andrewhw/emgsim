#include <stdio.h>
#include "mathtools.h"

static struct {
    double rB_;
    double area_;
} sData[] = {
	{0, 	0},
	{1, 	0},
	{1.1, 	0.0428434},
	{1.25, 	0.172562},
	{1.5, 	0.497448},
	{1.75, 	0.918666},
	{2.0, 	1.40307},
	{5.0, 	M_PI},
	{-1, 	-1}
    };

int
testChord()
{
    double area, areaA, areaB;
    double xA, yA, xB, yB, rA, rB;
    double difference;
    int status = 1;
    int i;

    yA = yB = 0;
    xA = 0;
    xB = 2;
    rA = 1;

    for (i = 0; sData[i].rB_ >= 0; i++) {
    	rB = sData[i].rB_;

	areaA = M_PI * SQR(rA);
	areaB = M_PI * SQR(rB);

	area = calculateAreaOfIntersectionOfTwoCircles(
	    	xA, yA, rA,
		xB, yB, rB
	    );
	printf("<DEBUG> A (%.4f, %.4f) r = %.4f : area = %.4f\n",
		    	xA, yA, rA, areaA);
	printf("<DEBUG> B (%.4f, %.4f) r = %.4f : area = %.4f\n",
		    	xB, yB, rB, areaB);
	printf("<DEBUG>             area = %.4f\n", area);

	difference = fabs(area - sData[i].area_);
	if (difference < 0.001) {
	    printf("<PASS> area for radius %g ok\n", rB);
	} else {
	    printf("<FAIL> bad area for radius %g -- want %g, got %g\n",
			rB, sData[i].area_, area);
	    status = 0;
	}
    }

    return status;
}
