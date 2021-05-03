#include <stdio.h>

#include "tclCkalloc.h"
#include "plottools.h"

int
testSimpleBins()
{
    GSimplePlot *plot;
    int *counts;
    double mean, denominator;
    int nBins;
    int i;

    /*
    for (nBins = 4; nBins < 128; nBins++)
    */
   
    nBins = 9;
    {
	counts = (int *) ckalloc(nBins * sizeof(int));
	memset(counts, 0, nBins * sizeof(int));

	mean = 0;
	denominator = 0;
	for (i = 1; i < nBins; i+=2) {
	    counts[i] = i * 10;
	}

	plot = gHistPltCreate(
		"plotHist",
		"plot.output",
		"Test Histogram",
		"Index", "Count");

	gHistPltAddSeries(plot, 1, NULL, "data.txt");

	for (i = 0; i < nBins; i++) {
	    printf("<DEBUG> counts[%3d] = %d\n", i, counts[i]);
	    denominator += i;
	    mean += i * 10;
	}
	mean = mean / denominator;
	printf("<DEBUG> mean = %g\n", mean);

	for (i = 0; i < nBins; i++) {
	    gHistPltAddPointToSeries(
		plot, 1, 
		(i * 10), 
		counts[i]
	    );
	}


	gHistPltSetMeanInSeries(plot, 1, mean);

	gHistPltClose(plot);

	ckfree(counts);
	counts = NULL;
    }

    printf("<PASS> -- got to end of test\n");

    return 1;
}

