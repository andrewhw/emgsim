#include <stdio.h>

#include "tclCkalloc.h"
#include "plottools.h"
#include "random.h"

#define	MAX	4
#define	MIN	-4
int
testHistogramTool()
{
    gHistogram *plot;
    int i;
    double value;

    plot = gHistToolCreate(
    	"plotHistScript",
	"plotHist.output",
	"Test Histogram",
	"Value", "Count",
	25, MIN, MAX);


    gHistToolAddSeries(plot, 0, "data", "data.txt");

    for (i = 0; i < 100000; i++) {
	value = gauss01();
	if (value < MIN)	value = MIN;
	if (value > MAX)	value = MAX;
	gHistToolAddValueToSeries(plot, 0, value);
    }
    gHistToolClose(plot);

    printf("<PASS> -- got to end of test\n");

    return 1;
}

