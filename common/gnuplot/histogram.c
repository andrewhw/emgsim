/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: histogram.c 17 2008-07-03 17:24:49Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <stdarg.h>
#include        <string.h>
#include        <ctype.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#ifndef OS_WINDOWS_NT
#include        <sys/wait.h>
#include        <unistd.h>
#include        <stdlib.h>
#else
#include        <io.h>
#endif
#include        <stdio.h>
#include        <time.h>
#include        <errno.h>

#endif

#include        "plottools.h"

#include        "tclCkalloc.h"
#include        "mathtools.h"
#include        "listalloc.h"
#include        "massert.h"


/**
 * Create a hist structure with a plot, but no data
 */
OS_EXPORT gHistogram *
gHistToolCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *title,
		const char *labelX,
		const char *labelY,
		int nBins,
		double min,
		double max
	)
{
	gHistogram     *plot;

	plot = (gHistogram *) ckalloc(sizeof(gHistogram));
	memset(plot, 0, sizeof(gHistogram));

	plot->plot_ = gHistPltCreate(
								 plotScriptFileName,
								 outputDataFileName,
								 title,
								 labelX,
								 labelY
		);

	plot->nBins_ = nBins;
	plot->max_ = max;
	plot->min_ = min;
	plot->range_ = max - min;

	return plot;
}

/**
 * Add a series to the plot
 */
OS_EXPORT int
gHistToolAddSeries(
		gHistogram * plot,
		int series,
		const char *seriesName,
		const char *seriesDataFileName
	)
{
	MSG_ASSERT(series < GPLT_MAX_SERIES, "series out of range");
	plot->nSeries_ = MAX(plot->nSeries_, series + 1);

	if (plot->series_[series].count_ == NULL)
	{
		plot->series_[series].count_ =
			(int *) ckalloc(plot->nBins_ * sizeof(int));
		memset(plot->series_[series].count_, 0, plot->nBins_ * sizeof(int));
	}
	return gHistPltAddSeries(plot->plot_,
							 series, seriesName, seriesDataFileName);
}

/**
 * Add a point to a series
 */
OS_EXPORT int
gHistToolAddValueToSeries(
		gHistogram * plot,
		int series,
		double x
	)
{
	double          normalizedValue;
	int             binMapping;


	if ((x < plot->min_) || (x > plot->max_))
	{
		printf("\n\nx data value seen (%f) not in defined range [%f:%f]\n\n",
			   x, plot->min_, plot->max_);
		return 0;
	}
	MSG_ASSERT(x <= plot->max_, "x value greater than max");
	MSG_ASSERT(x >= plot->min_, "x value less than min");

	plot->series_[series].mean_ += x;
	normalizedValue = ((x - plot->min_) / plot->range_);

	binMapping = (int) (normalizedValue * plot->nBins_);
	if (binMapping == plot->nBins_)
	{
		binMapping = plot->nBins_ - 1;
	}
	MSG_ASSERT(binMapping < plot->nBins_, "Number of bins exceeded");
	MSG_ASSERT(binMapping >= 0, "bin id is negative");

	/** add this count to a bin */
	plot->series_[series].count_[binMapping]++;

	/* record the number of points we have seen, for mean */
	plot->series_[series].n_++;

	return 1;
}

/**
 * Close the plot -- we are done getting data, so now do everything
 * required to turn this into a plot
 */
OS_EXPORT void
gHistToolClose(gHistogram * plot)
{
	double          binXValue;
	int             i, j;

	for (i = 0; i < plot->nSeries_; i++)
	{

		/** only add data if present */
		if (plot->series_[i].n_ > 0)
		{
			/** calculate the mean from the sum */
			plot->series_[i].mean_ =
				plot->series_[i].mean_
				/ (double) plot->series_[i].n_;

			for (j = 0; j < plot->nBins_; j++)
			{
				binXValue = (((double) j / (double) plot->nBins_)
							 * plot->range_)
					+ plot->min_;
				gHistPltAddPointToSeries(
										 plot->plot_, i,
										 binXValue,
										 plot->series_[i].count_[j]
					);
			}
			gHistPltAddPointToSeries(
									 plot->plot_, i,
									 plot->range_ + plot->min_,
									 0);

			gHistPltSetMeanInSeries(plot->plot_, i, plot->series_[i].mean_);
		}
	}
	gHistPltClose(plot->plot_);

	for (i = 0; i < plot->nSeries_; i++)
	{
		if (plot->series_[i].count_ != NULL)
		{
			ckfree(plot->series_[i].count_);
		}
	}
	ckfree(plot);
}

