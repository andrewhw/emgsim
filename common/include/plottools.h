/** ------------------------------------------------------------
 ** Tools for plotting various data elements to gnuplot
 ** ------------------------------------------------------------
 ** $Id: plottools.h 68 2010-01-30 17:06:46Z andrew $
 **/

#ifndef PLOT_TOOLS_HEADER__
#define PLOT_TOOLS_HEADER__

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
# ifndef OS_WINDOWS_NT
# include <sys/types.h>
# include <sys/wait.h>
# endif
#endif

#include "os_defs.h"

typedef struct GFP {
	FILE *headerFp_;
	FILE *bodyFp_;
	char *bodyTmpFilename_;

	int inBodyHereDocument_;
	int inHeaderHereDocument_;

	int pendingFooter_;

	int colour_;

} GFP;

#define		GPLT_MAX_SERIES		16

typedef struct GHistPlotExtension {
	double minY;
	double maxY;
	int nPoints;
} GHistPlotExtension;

typedef struct GHist3dPlotExtension {
	double minZ;
	double maxZ;
	int nPoints;
} GHist3dPlotExtension;

typedef struct GSimplePlot
{
	GFP *gfp;

	int sawFirstLine;
	int numSeries;
	const char *seriesName[GPLT_MAX_SERIES];
	const char *seriesDataFullFileName[GPLT_MAX_SERIES];
	const char *seriesDataLocalFileName[GPLT_MAX_SERIES];
	const char *dataDirectory;
	const char *localDataPrefix;
	FILE *seriesFP[GPLT_MAX_SERIES];
	void *extraData;
} GSimplePlot;

#define		GPLT_BLOCKSIZE		4

typedef struct gWaitList {
	int  nChildren_;

	pid_t *pidList_;
	int  nPidBlocks_;

	char **nameList_;
	int  nNameBlocks_;
} gWaitList;


#define		GPLT_HISTTOOL_BLOCKSIZE 	1024
struct gSimplePlotSeries {
	int n_;
	int *count_;
	double mean_;
};
typedef struct gHistogram {
	GSimplePlot *plot_;

	int nSeries_;
	int nBins_;

	struct gSimplePlotSeries series_[GPLT_MAX_SERIES];
	double min_;
	double max_;
	double range_;
} gHistogram;

#ifndef	lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif


OS_EXPORT GFP *gPltCreate(const char *filename,
			const char *outputFilename,
			const char *outputId,
			const char *tag);
OS_EXPORT GFP *gPltOpen(const char *filename, const char *tag);
OS_EXPORT int  gPltClose(GFP *gfp);


OS_EXPORT void gPltSetColour(
			GFP *plot,
			int useColour
		);

OS_EXPORT int gPltSetLabels(GFP *gfp,
			const char *title,
			const char *xLabel,
			const char *yLabel
		);

OS_EXPORT int gPltSet2dLabels(GFP *gfp,
			const char *title,
			const char *xLabel,
			const char *yLabel
		);

OS_EXPORT int gPltSet3dLabels(GFP *gfp,
			const char *title,
			const char *xLabel,
			const char *yLabel,
			const char *zLabel
		);

OS_EXPORT int gPltWriteFooter(GFP *gfp);
OS_EXPORT int gPltWriteBodyLine(GFP *gfp, const char *line, ...);

OS_EXPORT int gPltWriteSetupLine(GFP *gfp, const char *line, ...);
OS_EXPORT int gPltSetSquare(GFP *gfp);

OS_EXPORT gWaitList * gPltCreateWaitList();
OS_EXPORT int gPltRun(gWaitList *list, const char *executable, ...);
OS_EXPORT int gPltRunV(gWaitList *list,
			const char *executable,
			char *arguments[]
		);

OS_EXPORT int gPltWaitAll(gWaitList *list);

OS_EXPORT GSimplePlot *gPointPltCreate(
			const char *plotScriptFileName,
			const char *outputDataFileName,
			const char *title,
			const char *labelX,
			const char *labelY
		);

OS_EXPORT int gPointPltAddSeries(
			GSimplePlot *plot,
			int series,
			const char *seriesName,
			const char *seriesDataFileName
		);

OS_EXPORT void gPointPltAddPointToSeries(
			GSimplePlot *plot,
			int series,
			double x, double y
		);

OS_EXPORT void gPointPltClose(GSimplePlot *plot);


OS_EXPORT GSimplePlot *gHistPltCreate(
			const char *plotScriptFileName,
			const char *outputDataFileName,
			const char *title,
			const char *labelX,
			const char *labelY
		);

OS_EXPORT int gHistPltAddSeries(
			GSimplePlot *plot,
			int series,
			const char *seriesName,
			const char *seriesDataFileName
		);

OS_EXPORT void gHistPltAddPointToSeries(
			GSimplePlot *plot,
			int series,
			double x, double y
		);

OS_EXPORT int gHistPltSetMeanInSeries(
			GSimplePlot *plot,
			int series,
			double mean
		);

OS_EXPORT int gHistPltSetMeanAndSDInSeries(
			GSimplePlot *plot,
			int series,
			double mean,
			double sd
		);

OS_EXPORT void gHistPltClose(GSimplePlot *plot);


OS_EXPORT GSimplePlot *gHist3dPltCreate(
			const char *plotScriptFileName,
			const char *outputDataFileName,
			const char *title,
			const char *labelX,
			const char *labelY,
			const char *labelZ
		);

OS_EXPORT int gHist3dPltAddSeries(
			GSimplePlot *plot,
			int series,
			const char *seriesName,
			const char *seriesDataFileName,
			int mapmode,
			float paletteGamma
		);

OS_EXPORT void gHist3dPltAddPointToSeries(
			GSimplePlot *plot,
			int series,
			double x, double y, double z
		);

OS_EXPORT void gHist3dPltNewRowInSeries(
			GSimplePlot *plot,
			int series
		);

OS_EXPORT int gHist3dPltSetMeanInSeries(
			GSimplePlot *plot,
			int series,
			double mean
		);

OS_EXPORT int gHist3dPltSetMeanAndSDInSeries(
			GSimplePlot *plot,
			int series,
			double mean,
			double sd
		);

OS_EXPORT void gHist3dPltClose(GSimplePlot *plot);


OS_EXPORT int gSimplePltAddFunctionToPlot(
			GSimplePlot *plot,
			const char *function
		);


OS_EXPORT gHistogram *gHistToolCreate(
			const char *plotScriptFileName,
			const char *outputDataFileName,
			const char *title,
			const char *labelX,
			const char *labelY,
			int nBins,
			double min, double max
		);

OS_EXPORT int gHistToolAddSeries(
			gHistogram *plot,
			int series,
			const char *seriesName,
			const char *seriesDataFileName
		);

OS_EXPORT int gHistToolAddValueToSeries(
			gHistogram *plot,
			int series,
			double x
		);

OS_EXPORT void gHistToolClose(gHistogram *plot);


# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif

#endif  /* PLOT_TOOLS_HEADER__ */

