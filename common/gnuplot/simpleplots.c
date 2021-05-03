/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: simpleplots.c 78 2010-04-24 15:53:40Z andrew $
 **/

#include	"os_defs.h"

#ifndef MAKEDEPEND
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifndef OS_WINDOWS_NT
#include	<sys/wait.h>
#include	<unistd.h>
#include	<stdlib.h>
#else
#include	<io.h>
#endif
#include	<stdio.h>
#include	<time.h>
#include	<errno.h>

#endif

#include	"plottools.h"

#include	"tclCkalloc.h"
#include	"stringtools.h"
#include	"pathtools.h"
#include	"filetools.h"
#include	"listalloc.h"
#include	"log.h"
#include	"massert.h"


/**
 * Create a generic point plot
 */
static GSimplePlot *
gSimplePltCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *title,
		const char *labelX,
		const char *labelY
	)
{
	GSimplePlot	*result;
	char		*delimiterPos, *tmpScriptName;

	result = (GSimplePlot *) ckalloc(sizeof(GSimplePlot));
	memset(result, 0, sizeof(GSimplePlot));

	result->gfp = gPltCreate(
							plotScriptFileName,
							outputDataFileName,
							NULL, NULL);
	if (result->gfp == NULL)
	{
		return NULL; 
	}

	tmpScriptName = ckstrdup(plotScriptFileName);
	delimiterPos = strrchr(tmpScriptName, OS_PATH_DELIM);
	if (delimiterPos != NULL)
	{
		*delimiterPos = 0;
		result->dataDirectory = strconcat(
										tmpScriptName,
										OS_PATH_DELIM_STRING,
										"plotdata",
										OS_PATH_DELIM_STRING,
										NULL
			);
	} else
	{
		result->dataDirectory = strconcat(
										"plotdata",
										OS_PATH_DELIM_STRING,
										NULL
			);
	}
	ckfree(tmpScriptName);

	result->localDataPrefix = strconcat(
										"plotdata",
										OS_PATH_DELIM_STRING,
										NULL
		);

	gPltSetLabels(result->gfp, title, labelX, labelY);
	gPltWriteBodyLine(result->gfp, "	plot \\\n");

	return result;
}


/**
 * Create a generic point plot
 */
static GSimplePlot *
gSimple3dPltCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *title,
		const char *labelX,
		const char *labelY,
		const char *labelZ
	)
{
	GSimplePlot	*result;
	char		*delimiterPos, *tmpScriptName;

	result = (GSimplePlot *) ckalloc(sizeof(GSimplePlot));
	memset(result, 0, sizeof(GSimplePlot));

	result->gfp = gPltCreate(
							plotScriptFileName,
							outputDataFileName,
							NULL, NULL);
	if (result->gfp == NULL)
	{
		return NULL; 
	}

	tmpScriptName = ckstrdup(plotScriptFileName);
	delimiterPos = strrchr(tmpScriptName, OS_PATH_DELIM);
	if (delimiterPos != NULL)
	{
		*delimiterPos = 0;
		result->dataDirectory = strconcat(
										tmpScriptName,
										OS_PATH_DELIM_STRING,
										"plotdata",
										OS_PATH_DELIM_STRING,
										NULL
			);
	} else
	{
		result->dataDirectory = strconcat(
										"plotdata",
										OS_PATH_DELIM_STRING,
										NULL
			);
	}
	ckfree(tmpScriptName);

	result->localDataPrefix = strconcat(
										"plotdata",
										OS_PATH_DELIM_STRING,
										NULL
		);

	gPltSet3dLabels(result->gfp, title, labelX, labelY, labelZ);
	gPltWriteBodyLine(result->gfp, "	splot \\\n");

	return result;
}


/**
 * Create a generic histogram plot
 */
OS_EXPORT GSimplePlot *
gHistPltCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *title,
		const char *labelX,
		const char *labelY
	)
{
	GSimplePlot	*result;

	result = gSimplePltCreate(
							plotScriptFileName,
							outputDataFileName,
							title,
							labelX,
							labelY
		);

	result->extraData =
		(GHistPlotExtension *) ckalloc(sizeof(GHistPlotExtension));
	memset(result->extraData, 0, sizeof(GHistPlotExtension));

	return result;
}


/**
 * Create a generic histogram plot
 */
OS_EXPORT GSimplePlot *
gHist3dPltCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *title,
		const char *labelX,
		const char *labelY,
		const char *labelZ
	)
{
	GSimplePlot	*result;

	result = gSimple3dPltCreate(
							plotScriptFileName,
							outputDataFileName,
							title,
							labelX,
							labelY,
							labelZ
		);

	result->extraData =
		(GHist3dPlotExtension *) ckalloc(sizeof(GHist3dPlotExtension));
	memset(result->extraData, 0, sizeof(GHist3dPlotExtension));

	return result;
}

/**
 * Create a generic point plot
 */
OS_EXPORT GSimplePlot *
gPointPltCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *title,
		const char *labelX,
		const char *labelY
	)
{
	return gSimplePltCreate(
				plotScriptFileName,
				outputDataFileName,
				title,
				labelX,
				labelY
	);
}


int
gSimplePltAddSeries(
		GSimplePlot * plot,
		int series,
		const char *seriesName,
		const char *seriesDataFileName,
		const char *shape
	)
{
	if (plot->seriesDataFullFileName[series] != NULL)
		return 0;

	/** ensure that we have a blank series location */
	if (series >= plot->numSeries)
	{
		plot->numSeries = series + 1;
	}
	MSG_ASSERT(plot->numSeries < GPLT_MAX_SERIES, "Too many series in plot");

	plot->seriesDataFullFileName[series] =
		strconcat(plot->dataDirectory,
				seriesDataFileName,
				NULL
		);

	if (seriesName != NULL)
	{
		plot->seriesName[series] = ckstrdup(seriesName);
	} else
	{
		plot->seriesName[series] = NULL;
		gPltWriteSetupLine(plot->gfp, "set key below\n");
	}

	plot->seriesFP[series] =
		fopenpath(plot->seriesDataFullFileName[series], "w");

	if (plot->sawFirstLine == 0)
	{
		plot->sawFirstLine = 1;
	} else
	{
		gPltWriteBodyLine(plot->gfp, ", \\\n");
	}

	plot->seriesDataLocalFileName[series] =
		strconcat(plot->localDataPrefix,
				seriesDataFileName,
				NULL
		);

	if (plot->seriesName[series] != NULL)
	{
		gPltWriteBodyLine(plot->gfp, "	\"%s\" t \"%s\" with %s",
						plot->seriesDataLocalFileName[series],
						plot->seriesName[series],
						shape
			);
	} else
	{
		gPltWriteBodyLine(plot->gfp, "	\"%s\" with %s",
						plot->seriesDataLocalFileName[series],
						shape
			);
	}

	return 1;
}

void
gSimplePltAddPointToSeries(
		GSimplePlot * plot,
		int series,
		double x,
		double y
	)
{
	fprintf(plot->seriesFP[series], "%g %g\n", x, y);
}

void
gSimplePlt3dAddPointToSeries(
		GSimplePlot * plot,
		int series,
		double x,
		double y,
		double z
	)
{
	fprintf(plot->seriesFP[series], "%g %g %g\n", x, y, z);
}

OS_EXPORT int
gHistPltAddSeries(
		GSimplePlot * plot,
		int series,
		const char *seriesName,
		const char *seriesDataFullFileName
	)
{
	return gSimplePltAddSeries(plot,
				series, seriesName, seriesDataFullFileName,
				"boxes");
}

OS_EXPORT int
gHist3dPltAddSeries(
		GSimplePlot * plot,
		int series,
		const char *seriesName,
		const char *seriesDataFullFileName,
		int mapmode,
		float paletteGamma
	)
{
	char *linestart[2] = { "# ", "" };
	int gammastart;

	/** ensure that mapmode is either one or zero */
	mapmode = mapmode ? 1 : 0;


	gPltWriteSetupLine(plot->gfp, "set key below\n");
	gPltWriteSetupLine(plot->gfp, "set noborder\n");
	gPltWriteSetupLine(plot->gfp, "set pm3d at s hidden3d 100\n");
	gPltWriteSetupLine(plot->gfp, "set style line 100 lt 5 lw 0.25 lc rgb \"black\"\n");
	gPltWriteSetupLine(plot->gfp, "set contour ${CONTOUR}\n");
	gPltWriteSetupLine(plot->gfp, "unset hidden3d\n");
	gPltWriteSetupLine(plot->gfp, "unset surf\n");


	gPltWriteSetupLine(plot->gfp,
			"%sset view map\n",
			linestart[mapmode]);


	gPltWriteSetupLine(plot->gfp,
			"%sset contour both\n",
			linestart[1 - mapmode]);

	if (paletteGamma > 1)
	{
		gammastart = 1;
	}
	else
	{
		gammastart = 1;
		paletteGamma = 2.2f;
	}



	gPltWriteSetupLine(plot->gfp, "\n");
	gPltWriteSetupLine(plot->gfp,
			"%s    gamma = %g\n",
			linestart[gammastart], paletteGamma);
	gPltWriteSetupLine(plot->gfp,
			"%s    color(gray) = gray**(1./gamma)\n",
			linestart[gammastart]);
	gPltWriteSetupLine(plot->gfp,
			"%s    #set palette model RGB functions %s\n",
			linestart[gammastart],
			"color(gray), color(gray), color(gray)");
	gPltWriteSetupLine(plot->gfp,
			"%s    set palette model XYZ functions %s\n",
			linestart[gammastart],
			"color(gray)**0.35, color(gray)**0.5, color(gray)**0.8");

	gPltWriteSetupLine(plot->gfp, "\n");



	return gSimplePltAddSeries(plot,
				series, seriesName, seriesDataFullFileName,
				"lines");
}

OS_EXPORT void
gHistPltAddPointToSeries(
		GSimplePlot * plot,
		int series,
		double x,
		double y
	)
{
	GHistPlotExtension *histData;

	gSimplePltAddPointToSeries(plot, series, x, y);
	if (plot->extraData != NULL)
	{
		histData = (GHistPlotExtension *) plot->extraData;
		if (histData->nPoints == 0)
		{
			histData->maxY = histData->minY = y;
		} else
		{
			if (histData->maxY < y)
				histData->maxY = y;
			if (histData->minY > y)
				histData->minY = y;
		}
		histData->nPoints++;
	}
}


OS_EXPORT void
gHist3dPltNewRowInSeries(
		GSimplePlot * plot,
		int series
	)
{
	fprintf(plot->seriesFP[series], "\n");
}

OS_EXPORT void
gHist3dPltAddPointToSeries(
		GSimplePlot * plot,
		int series,
		double x,
		double y,
		double z
	)
{
	GHist3dPlotExtension *histData;

	gSimplePlt3dAddPointToSeries(plot, series, x, y, z);
	if (plot->extraData != NULL)
	{
		histData = (GHist3dPlotExtension *) plot->extraData;
		if (histData->nPoints == 0)
		{
			histData->maxZ = histData->minZ = z;
		} else
		{
			if (histData->maxZ < y) histData->maxZ = y;
			if (histData->minZ > z) histData->minZ = z;
		}
		histData->nPoints++;
	}
}

OS_EXPORT int
gHistPltSetMeanInSeries(
		GSimplePlot * plot,
		int series,
		double mean
	)
{
	GHistPlotExtension *histData;
	FILE		*fp;
	char		*dataFileName;

	if (plot->extraData == NULL)
	{
		return 0;
	}
	histData = (GHistPlotExtension *) plot->extraData;
	if (histData->nPoints == 0)
	{
		return 0;
	}
	/**
	* write out the data
	*/
	dataFileName = strconcat(
							plot->seriesDataFullFileName[series],
							".mean",
							NULL);
	fp = fopenpath(dataFileName, "w");
	fprintf(fp, "%f %f\n", mean, histData->minY);
	fprintf(fp, "%f %f\n", mean, histData->maxY);
	fclose(fp);
	ckfree(dataFileName);


	/**
	* Record the datafile to be plotted
	*/
	if (plot->sawFirstLine == 0)
	{
		plot->sawFirstLine = 1;
	} else
	{
		gPltWriteBodyLine(plot->gfp, ", \\\n");
	}

	if (plot->seriesName[series] != NULL)
	{
		gPltWriteBodyLine(plot->gfp, "	\"%s.mean\" t \"%s Mean\" with lines lw 4",
						plot->seriesDataLocalFileName[series],
						plot->seriesName[series]
			);
	} else
	{
		gPltWriteBodyLine(plot->gfp, "	\"%s.mean\" with lines lw 4",
						plot->seriesDataLocalFileName[series]
			);
	}

	return 1;
}

OS_EXPORT int
gHistPltSetMeanAndSDInSeries(
		GSimplePlot * plot,
		int series,
		double mean,
		double sd
	)
{
	GHistPlotExtension *histData;
	FILE		*fp;
	char		*dataFileName;

	if (plot->extraData == NULL)
	{
		return 0;
	}
	histData = (GHistPlotExtension *) plot->extraData;
	if (histData->nPoints == 0)
	{
		return 0;
	}
	/**
	* write out the data
	*/
	dataFileName = strconcat(
							plot->seriesDataFullFileName[series],
							".sd",
							NULL);
	fp = fopenpath(dataFileName, "w");
	fprintf(fp, "%f %f %f %f %f\n",
				mean, histData->minY - 1,
				mean-sd, mean+sd, sd);
	fclose(fp);
	ckfree(dataFileName);


	/**
	* Record the datafile to be plotted
	*/
	if (plot->sawFirstLine == 0)
	{
		plot->sawFirstLine = 1;
	} else
	{
		gPltWriteBodyLine(plot->gfp, ", \\\n");
	}

	if (plot->seriesName[series] != NULL)
	{
		gPltWriteBodyLine(plot->gfp,
				"	\"%s.sd\" t \"%s SD\" with xerrorbars lw 4",
				plot->seriesDataLocalFileName[series],
				plot->seriesName[series]
			);
	} else
	{
		gPltWriteBodyLine(plot->gfp,
				"	\"%s.sd\" with xerrorbars lw 4",
				plot->seriesDataLocalFileName[series]
			);
	}

	return 1;
}

OS_EXPORT int
gPointPltAddSeries(
		GSimplePlot * plot,
		int series,
		const char *seriesName,
		const char *seriesDataFullFileName
	)
{
	return gSimplePltAddSeries(plot,
				series, seriesName, seriesDataFullFileName,
				"points");
}

OS_EXPORT void
gPointPltAddPointToSeries(
		GSimplePlot * plot,
		int series,
		double x,
		double y
	)
{
	gSimplePltAddPointToSeries(plot, series, x, y);
}


OS_EXPORT int
gSimplePltAddFunctionToPlot(
		GSimplePlot * plot,
		const char *function
	)
{
	/**
	* Add in another plot line
	*/
	if (plot->sawFirstLine == 0)
	{
		plot->sawFirstLine = 1;
	} else
	{
		gPltWriteBodyLine(plot->gfp, ", \\\n");
	}

	gPltWriteBodyLine(plot->gfp, "	%s\n", function);

	return 1;
}

/**
 * Clean up the generic plot
 */
void
gSimplePltClose(GSimplePlot * plot)
{
	int i;

	gPltWriteBodyLine(plot->gfp, "\n");
	for (i = 0; i < plot->numSeries; i++)
	{
		if (plot->seriesName[i] != NULL)
		{
			ckfree((void *) plot->seriesName[i]);
		}
		if (plot->seriesDataFullFileName[i] != NULL)
		{
			ckfree((void *) plot->seriesDataFullFileName[i]);
		}
		if (plot->seriesDataLocalFileName[i] != NULL)
		{
			ckfree((void *) plot->seriesDataLocalFileName[i]);
		}
		if (plot->seriesFP[i] != NULL)
		{
			fclose(plot->seriesFP[i]);
		}
	}

	if (plot->dataDirectory != NULL)
	{
		ckfree((void *) plot->dataDirectory);
	}
	if (plot->localDataPrefix != NULL)
	{
		ckfree((void *) plot->localDataPrefix);
	}
	gPltClose(plot->gfp);
	ckfree(plot);
}


/**
 * Clean up the generic histogram plot
 */
OS_EXPORT void
gHistPltClose(GSimplePlot * plot)
{
	/**
	* have to do local cleanup before general cleanup or the
	* plot structure will be invalid already!
	*/
	if (plot->extraData != NULL)
		ckfree(plot->extraData);

	gSimplePltClose(plot);
}

/**
 * Clean up the generic histogram plot
 */
OS_EXPORT void
gHist3dPltClose(GSimplePlot * plot)
{
	gHistPltClose(plot);
}

/**
 * Clean up the generic histogram plot
 */
OS_EXPORT void
gPointPltClose(GSimplePlot * plot)
{
	gSimplePltClose(plot);
}

