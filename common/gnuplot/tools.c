/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: tools.c 107 2013-06-22 20:22:05Z andrew $
 **/

#include		"os_defs.h"

#ifndef MAKEDEPEND
#include		<stdio.h>
#include		<stdarg.h>
#include		<string.h>
#include		<ctype.h>
#include		<sys/types.h>
#include		<sys/stat.h>
#ifndef OS_WINDOWS_NT
#include		<sys/wait.h>
#include		<unistd.h>
#include		<stdlib.h>
#else
#include		<io.h>
#endif
#include		<stdio.h>
#include		<time.h>
#include		<errno.h>

#endif

#include "os_defs.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#include		"plottools.h"

#include		"tclCkalloc.h"
#include		"stringtools.h"
#include		"pathtools.h"
#include		"filetools.h"
#include		"listalloc.h"
#include		"log.h"


static char	*sGnuplotHeader[] =
{
	"",
	"TAG=\"\"",
	"COLOUR=\"monochrome\"",
	"",
	"OUTPUT_TYPE=\"X\"",
	"FONT=\"Helvetica\"",
	"FONTSIZE=\"\"",
	"",
	"SUBTITLE=\"\"",
	"",
	"# plot to a local file ",
	"TMPDIR=\"tmp\"",
	"TMP_PLOT=\"${TMPDIR}/gplot$$\"",
	"",
	"VIEW_PIT=\"60\"",
	"VIEW_YAW=\"30\"",
	"VIEW_ZOOM=\"1\"",
	"",
	"XMIN=\"*\"",
	"XMAX=\"*\"",
	"YMIN=\"*\"",
	"YMAX=\"*\"",
	"ZMIN=\"*\"",
	"ZMAX=\"*\"",
	"",
	"GAMMA=\"2.2\"",
	"CONTOUR=\"base\"",
	"MAP=\"NO\"",
	"LIST=\"NO\"",
	"",
	"## mark flags for output type",
	"for opt in \"$@\"",
	"do",
	"	case \"${opt}\" in",
	"		-ps*)	OUTPUT_TYPE=\"PS\"",
	"			;;",
	"		-eps*)	OUTPUT_TYPE=\"EPS\"",
	"			;;",
	"		-pdf*)	OUTPUT_TYPE=\"PDF\"",
	"			;;",
	"		-gif*)	OUTPUT_TYPE=\"GIF\"",
	"			;;",
	"		-png*)	OUTPUT_TYPE=\"PNG\"",
	"			;;",
	"		-pbm*)	OUTPUT_TYPE=\"PBM\"",
	"			;;",
	"		-latex*)	OUTPUT_TYPE=\"LATEX\"",
	"			;;",
	"		-jpg*)		OUTPUT_TYPE=\"JPG\"",
	"			;;",
	"		-jpeg*)		OUTPUT_TYPE=\"JPG\"",
	"			;;",
	"		-colour*)	COLOUR=\"color\"",
	"			;;",
	"		-mono*)		COLOUR=\"monochrome\"",
	"			;;",
	"		-map*)		MAP=\"YES\"",
	"			;;",
	"		-gamma=*)		GAMMA=`echo ${opt}| sed -e 's/^-gamma=//'`",
	"			;;",
	"		-contour=*)		CONTOUR=`echo ${opt}| sed -e 's/^-contour=//'`",
	"			;;",
	"		-subtitle=*)		VALUE=`echo ${opt}| sed -e 's/^-title=//'`",
	"			SUBTITLE=\"\\n(${VALUE})\"",
	"			;;",
	"		-font=*)		FONT=`echo ${opt}| sed -e 's/^-font=//'`",
	"			;;",
	"		-fsize=*)		FONTSIZE=`echo ${opt}| sed -e 's/^-fsize=//'`",
	"			;;",
	"		-subtitle=*)  VALUE=`echo ${opt}| sed -e 's/^-subtitle=//'`",
	"			SUBTITLE=\"\\n(${VALUE})\"",
	"			;;",
	"		-tag=*)  TAG=`echo ${opt}| sed -e 's/^-tag=//'`",
	"			;;",
	"		-v[0-9]*) ",
	"			VIEW_PIT=`echo ${opt} | sed -e's/-v//' -e's/,.*//'`",
	"			VIEW_YAW=`echo ${opt} | sed -e's/-v//' -e's/[^,]*,//' -e's/,[^,]*//'`",
	"			VIEW_ZOOM=`echo ${opt}| sed -e's/-v//' -e's/.*,//'`",
	"			;;",
	"",
	"		-x[0-9]*) ",
	"			XMIN=`echo ${opt} | sed -e 's/-x//' -e 's/:.*//'`",
	"			XMAX=`echo ${opt} | sed -e 's/-x//' -e 's/.*://'`",
	"			case X\"${XMIN}\" in",
	"				X[Oo][Pp][Ee][Nn]*)",
	"				XMIN=\"*\"",
	"				;;",
	"			esac",
	"			case X\"${XMAX}\" in",
	"				X[Oo][Pp][Ee][Nn]*)",
	"				XMAX=\"*\"",
	"				;;",
	"			esac",
	"			;;",
	"		-y*) ",
	"			YMIN=`echo ${opt} | sed -e 's/-y//' -e 's/:.*//'`",
	"			YMAX=`echo ${opt} | sed -e 's/-y//' -e 's/.*://'`",
	"			case X\"${YMIN}\" in",
	"				X[Oo][Pp][Ee][Nn]*)",
	"				YMIN=\"*\"",
	"				;;",
	"			esac",
	"			case X\"${YMAX}\" in",
	"				X[Oo][Pp][Ee][Nn]*)",
	"				YMAX=\"*\"",
	"				;;",
	"			esac",
	"			;;",
	"		-z*) ",
	"			ZMIN=`echo ${opt} | sed -e 's/-z//' -e 's/:.*//'`",
	"			ZMAX=`echo ${opt} | sed -e 's/-z//' -e 's/.*://'`",
	"			case X\"${ZMIN}\" in",
	"				X[Oo][Pp][Ee][Nn]*)",
	"				ZMIN=\"*\"",
	"				;;",
	"			esac",
	"			case X\"${ZMAX}\" in",
	"				X[Oo][Pp][Ee][Nn]*)",
	"				ZMAX=\"*\"",
	"				;;",
	"			esac",
	"			;;",
	"		-list*)  LIST=\"YES\"",
	"			;;",
	"		*)	echo \"Unknown option ${opt} - ignoring\"",
	"			;;",
	"	esac",
	"done",
	"",
	"",
	"# ensure sane defaults",
	"if [ X\"${XMIN}\" = X"" ] ; then XMIN=\"*\" ; fi",
	"if [ X\"${XMAX}\" = X"" ] ; then XMAX=\"*\" ; fi",
	"if [ X\"${YMIN}\" = X"" ] ; then YMIN=\"0\" ; fi",
	"if [ X\"${YMAX}\" = X"" ] ; then YMAX=\"*\" ; fi",
	"if [ X\"${ZMIN}\" = X"" ] ; then ZMIN=\"0\" ; fi",
	"if [ X\"${ZMAX}\" = X"" ] ; then ZMAX=\"*\" ; fi",
	"",
	"",
	"## Change to the script directory",
	"SCRIPTDIR=`dirname $0`",
	"cd ${SCRIPTDIR}",
	"if [ ! -d \"${TMPDIR}\" ]",
	"then",
	"	mkdir \"${TMPDIR}\"",
	"fi",
	"trap \"rm -rf ${TMP_PLOT}\" 0 2 3 15",
	"",
	"",
	"",
	"case \"${OUTPUT_TYPE}\" in",
	"PS*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal postscript landscape enhanced ${COLOUR} ${FONTSIZE}",
	"	set output \"%s${ID}.ps\"",
	"__EOF__",
	"	output=\"%s${ID}.ps\"",
	"	;;",
	"PDF*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal postscript landscape enhanced ${COLOUR} ${FONTSIZE}",
	"	set output \"%s${ID}.ps\"",
	"__EOF__",
	"	output=\"%s${ID}.ps\"",
	"	;;",
	"EPS*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal postscript eps enhanced ${COLOUR} ${FONTSIZE}",
	"	set output \"%s${ID}.eps\"",
	"__EOF__",
	"	output=\"%s${ID}.eps\"",
	"	;;",
	"GIF*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal gif small size 1024,768",
	"	set output \"%s${ID}.gif\"",
	"__EOF__",
	"	output=\"%s${ID}.gif\"",
	"	;;",
	"PNG*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal png small ${COLOUR}",
	"	set output \"%s${ID}.png\"",
	"__EOF__",
	"	output=\"%s${ID}.png\"",
	"	;;",
	"PBM*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal pbm small ${COLOUR}",
	"	set output \"%s${ID}.pbm\"",
	"__EOF__",
	"	output=\"%s${ID}.pbm\"",
	"	;;",
	"JPG*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal jpeg small",
	"	set output \"%s${ID}.jpg\"",
	"__EOF__",
	"	output=\"%s${ID}.jpg\"",
	"	;;",
	"LATEX*)",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	set terminal latex",
	"	set output \"%s${ID}.tex\"",
	"__EOF__",
	"	output=\"%s${ID}.tex\"",
	"	;;",
	"esac",
	"",
	"## -- end of standard header --",
	"",
	NULL
};

static char	*sGnuplotFooter[] =
{
	"",
	"if [ X\"${OUTPUT_TYPE}\" = X\"X\" ]",
	"then",
	"	cat >> ${TMP_PLOT} << __EOF__",
	"	pause -1",
	"__EOF__",
	"else",
	"	echo \" . Plotting data to ${output}\"",
	"fi",
	"",
	"",
	"gnuplot ${TMP_PLOT}",
	"",
	"",
	"## convert ps file to pdf if indicated",
	"case \"${OUTPUT_TYPE}\" in",
	"PDF*)",
	"	ps2pdf \"${output}\"",
	"	;;",
	"*)",
	"	;;",
	"esac",
	"",
	"if [ X\"${LIST}\" = X\"YES\" ]",
	"then",
	"	cat ${TMP_PLOT}",
	"fi",
	"",
	NULL
};


/**
 * write out the gnuplot standard header to the given file pointer
 */
static int
writeHeader(
		GFP * gfp,
		const char *outputFilename,
		const char *outputId,
		const char *tag
	)
{
	off_t		offset;
	time_t		curtime;
	const char	*filename;
	int			i;

	if (outputFilename != NULL)
	{
		filename = outputFilename;
	} else
	{
		filename = "plot-output";
	}

	curtime = time(NULL);
	offset = ftell(gfp->headerFp_);
	if (offset == 0)
	{
		fprintf(gfp->headerFp_, "#!/bin/sh\n");
	}
	fprintf(gfp->headerFp_, "#\n");

	if (tag != NULL)
	{
		fprintf(gfp->headerFp_,
				"# Gnuplot standard file header created by the\n");
		fprintf(gfp->headerFp_, "#	%s\n", tag);
		fprintf(gfp->headerFp_, "# program on\n");
		fprintf(gfp->headerFp_, "#	%s", ctime(&curtime));
	} else
	{
		fprintf(gfp->headerFp_,
				"# Gnuplot standard file header created on:\n");
		fprintf(gfp->headerFp_, "#	%s", ctime(&curtime));
	}
	fprintf(gfp->headerFp_, "#\n");
	fprintf(gfp->headerFp_, "#\n");


	if (outputId != NULL)
	{
		fprintf(gfp->headerFp_, "ID='-%s'\n", outputId);
	} else
	{
		fprintf(gfp->headerFp_, "ID=''\n");
	}

	if (gfp->colour_ != 0)
	{
		fprintf(gfp->headerFp_, "COLOUR=\"color\"\n");
	} else
	{
		fprintf(gfp->headerFp_, "COLOUR=\"monochrome\"\n");
	}

	for (i = 0; sGnuplotHeader[i] != NULL; i++)
	{
		if (strstr(sGnuplotHeader[i], "%s") != NULL)
		{
			fprintf(gfp->headerFp_, sGnuplotHeader[i], filename);
			fputc('\n', gfp->headerFp_);
		} else
		{
			fputs(sGnuplotHeader[i], gfp->headerFp_);
			fputc('\n', gfp->headerFp_);
		}
	}

	gfp->pendingFooter_ = 1;

	return !ferror(gfp->headerFp_);
}

static int
openBodyHereDocument(GFP * gfp)
{
	fprintf(gfp->bodyFp_, "cat >> ${TMP_PLOT} << __EOF__\n");
	gfp->inBodyHereDocument_ = 1;
	return !ferror(gfp->bodyFp_);
}

static int
closeBodyHereDocument(GFP * gfp)
{
	fprintf(gfp->headerFp_, "__EOF__\n# end of body\n\n");
	gfp->inBodyHereDocument_ = 0;
	fflush(gfp->bodyFp_);
	return !ferror(gfp->bodyFp_);
}


static int
openHeaderHereDocument(GFP * gfp)
{
	fprintf(gfp->headerFp_, "cat >> ${TMP_PLOT} << __EOF__\n");
	gfp->inHeaderHereDocument_ = 1;
	return !ferror(gfp->headerFp_);
}

static int
closeHeaderHereDocument(GFP * gfp)
{
	fprintf(gfp->headerFp_, "__EOF__\n# end of header\n\n");
	fflush(gfp->headerFp_);
	gfp->inHeaderHereDocument_ = 0;
	return !ferror(gfp->headerFp_);
}

/**
 * Create a new gnuplot workfile by the given name
 */
OS_EXPORT GFP  *
gPltCreate(
		const char *plotScriptFileName,
		const char *outputDataFileName,
		const char *outputId,
		const char *tag
	)
{
	GFP			*gfp;

	gfp = (GFP *) ckalloc(sizeof(GFP));
	memset(gfp, 0, sizeof(GFP));

	if ((gfp->headerFp_ = fopenpath(plotScriptFileName, "w")) == NULL)
	{
		goto FAIL;
	}
	gfp->bodyTmpFilename_ = allocTempFileName("gplot");
	/*
	fprintf(stderr, "ALLOCATED bodyTmpFilename_ : ox%08x\n", (int) gfp->bodyTmpFilename_);
	*/

	if ((gfp->bodyFp_ = fopen(gfp->bodyTmpFilename_, "w+")) == NULL)
	{
		goto FAIL;
	}
#ifndef OS_WINDOWS_NT
	/** set permissions to run on UNIX */
	fchmod(fileno(gfp->headerFp_), 0777);
	fchmod(fileno(gfp->bodyFp_), 0777);
#endif

	if (!writeHeader(gfp, outputDataFileName, outputId, tag))
	{
		goto FAIL;
	}
	return gfp;


FAIL:
	if (gfp != NULL)
	{
		if (gfp->bodyFp_ != NULL)
			fclose(gfp->bodyFp_);
		if (gfp->headerFp_ != NULL)
			fclose(gfp->headerFp_);

		if (gfp->bodyTmpFilename_ != NULL)
		{
#ifndef	OS_WINDOWS_NT
			unlink(gfp->bodyTmpFilename_);
#else
			_unlink(gfp->bodyTmpFilename_);
#endif
			ckfree(gfp->bodyTmpFilename_);
		}
		ckfree(gfp);
	}
	return NULL;
}


/**
 * Open an existing gnuplot workfile by the given name
 */
OS_EXPORT GFP  *
gPltOpen(const char *filename, const char *tag)
{
	GFP			*gfp;

	gfp = (GFP *) ckalloc(sizeof(GFP));
	memset(gfp, 0, sizeof(GFP));

	if ((gfp->headerFp_ = fopen(filename, "r")) == NULL)
	{
		ckfree(gfp);
		return NULL;
	}
	return gfp;
}


/**
 * Close a gnuplot workfile
 */
OS_EXPORT int
gPltClose(GFP * gfp)
{
	int			status = 1;
	if (gfp->pendingFooter_ == 1)
	{
		gPltWriteFooter(gfp);
	}
	fclose(gfp->headerFp_);

	if (gfp->bodyTmpFilename_ != NULL)
	{
#ifndef	OS_WINDOWS_NT
		unlink(gfp->bodyTmpFilename_);
#else
		_unlink(gfp->bodyTmpFilename_);
#endif
		ckfree(gfp->bodyTmpFilename_);
	}

	/** clean up memory */
	ckfree(gfp);

	return status;
}


OS_EXPORT void
gPltSetColour(
		GFP * gfp,
		int useColour
	)
{
	gfp->colour_ = useColour;
}

OS_EXPORT int
gPltSetLabels(
		GFP * gfp,
		const char *title,
		const char *xLabel,
		const char *yLabel
	)
{
	return gPltSet2dLabels(gfp, title, xLabel, yLabel);
}

OS_EXPORT int
gPltSet2dLabels(
		GFP * gfp,
		const char *title,
		const char *xLabel,
		const char *yLabel
	)
{
	int			status = 1;

	if (title != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set title \"%s${SUBTITLE}\" font \"${FONT}\"\n", title)
			&& status);
	}
	if (xLabel != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set xlabel \"%s\" font \"${FONT}\"\n", xLabel)
			&& status);
	}
	if (yLabel != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set ylabel \"%s\" font \"${FONT}\"\n", yLabel)
			&& status);
	}
	return status;
}

OS_EXPORT int
gPltSet3dLabels(
		GFP * gfp,
		const char *title,
		const char *xLabel,
		const char *yLabel,
		const char *zLabel
	)
{
	int			status = 1;

	if (title != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set title \"%s\" font \"${FONT}\"\n", title)
			&& status);
	}
	if (xLabel != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set xlabel \"%s\" font \"${FONT}\"\n", xLabel)
			&& status);
	}
	if (yLabel != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set ylabel \"%s\" font \"${FONT}\"\n", yLabel)
			&& status);
	}
	if (zLabel != NULL)
	{
		status =
			(gPltWriteSetupLine(gfp,
						"	set zlabel \"%s\" font \"${FONT}\"\n", zLabel)
			&& status);
	}
	return status;
}


OS_EXPORT int
gPltSetSquare(GFP * gfp)
{
	return gPltWriteSetupLine(gfp, "	set size ratio -1\n");
}


OS_EXPORT int
gPltWriteBodyLine(GFP * gfp, const char *message,...)
{
	int			status;
	va_list		messageArgs;

	if (gfp->inBodyHereDocument_ == 0)
	{
		if (!openBodyHereDocument(gfp))
			return 0;
	}
	va_start(messageArgs, message);
	status = vfprintf(gfp->bodyFp_, message, messageArgs);
	va_end(messageArgs);

	fflush(gfp->bodyFp_);

	return status;
}


OS_EXPORT int
gPltWriteSetupLine(GFP * gfp, const char *message,...)
{
	int			status;
	va_list		messageArgs;

	if (gfp->inHeaderHereDocument_ == 0)
	{
		if (!openHeaderHereDocument(gfp))
			return 0;
	}
	va_start(messageArgs, message);
	status = vfprintf(gfp->headerFp_, message, messageArgs);
	va_end(messageArgs);

	fflush(gfp->headerFp_);

	return status;
}

OS_EXPORT int
gPltWriteFooter(GFP * gfp)
{
	int			i, status = 1;

	if (gfp->inHeaderHereDocument_ == 1)
	{
		if (!closeHeaderHereDocument(gfp))
			return 0;
	}
	if (gfp->bodyFp_ != NULL)
	{

		if (gfp->inBodyHereDocument_ == 1)
		{
			if (!closeBodyHereDocument(gfp))
				status = 0;
		}
		rewind(gfp->bodyFp_);
		if (!copyFileContents(
#ifndef OS_WINDOWS_NT
							fileno(gfp->headerFp_),
							fileno(gfp->bodyFp_))
#else
							_fileno(gfp->headerFp_),
							_fileno(gfp->bodyFp_))
#endif
						)
		{
			LogError("Failure copying gnuplot body contents\n");
			status = 0;
		}
		fclose(gfp->bodyFp_);
		gfp->bodyFp_ = NULL;

	}
	gfp->pendingFooter_ = 0;

	for (i = 0; sGnuplotFooter[i] != NULL; i++)
	{
		fprintf(gfp->headerFp_, "%s\n", sGnuplotFooter[i]);
	}

	return !ferror(gfp->headerFp_) && status;
}

OS_EXPORT gWaitList *
gPltCreateWaitList()
{
	gWaitList	*result;

	result = ckalloc(sizeof(gWaitList));
	memset(result, 0, sizeof(gWaitList));

	return result;
}


#ifndef OS_WINDOWS_NT
OS_EXPORT int
gPltWaitAll(gWaitList * list)
{
	int			childIndex;
	int			exitPid;
	int			status;
	int			i;


	while (list->nChildren_ > 0)
	{
		exitPid = waitpid(0, &status, 0);

		childIndex = (-1);
		for (i = 0; i < list->nChildren_; i++)
		{
			if (list->pidList_[i] == exitPid)
			{
				childIndex = i;
				break;
			}
		}

		if (childIndex < 0)
		{
			LogError("Caught exit of unmanaged child %d\n", exitPid);
			continue;
		}
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0)
			{
				LogError("Child %d '%s' exitted with status %d\n",
						exitPid,
						list->nameList_[childIndex],
						WEXITSTATUS(status));
			}
			/** everything is ok */
		} else
		{
			LogError("Child %d '%s' crashed\n",
					exitPid,
					list->nameList_[childIndex]);
		}


		ckfree(list->nameList_[childIndex]);
		for (i = childIndex + 1; i < list->nChildren_; i++)
		{
			list->nameList_[i - 1] = list->nameList_[i];
			list->pidList_[i - 1] = list->pidList_[i];
		}
		list->nChildren_--;
	}

	if (list->nameList_ != NULL)
		ckfree(list->nameList_);
	if (list->pidList_ != NULL)
		ckfree(list->pidList_);
	ckfree(list);

	return 1;
}
#else
OS_EXPORT int
gPltWaitAll(gWaitList * list)
{
	int			i;

	LogError("Wait for children not implemented on Windows\n");
	for (i = 0; i < list->nChildren_; i++)
	{
		ckfree(list->nameList_[i]);
	}

	ckfree(list->nameList_);
	ckfree(list->pidList_);
	ckfree(list);

	return 0;
}
#endif

OS_EXPORT int
gPltRunV(
		gWaitList * waitList,
		const char *filename,
		char *execArgs[]
	)
{
	pid_t		newPid;
	char		*dummyPtr[2];

	(void) dummyPtr;
	(void) newPid;


	listMkCheckSize(waitList->nChildren_ + 1,
					(void **) &waitList->pidList_,
					&waitList->nPidBlocks_,
					GPLT_BLOCKSIZE,
					sizeof(pid_t),
					__FILE__, __LINE__);

	listMkCheckSize(waitList->nChildren_ + 1,
					(void **) &waitList->nameList_,
					&waitList->nNameBlocks_,
					GPLT_BLOCKSIZE,
					sizeof(char *), __FILE__, __LINE__);


	waitList->nameList_[waitList->nChildren_] = ckstrdup(filename);

#ifndef OS_WINDOWS_NT
	newPid = fork();
	if (newPid < 0)
	{
		LogError("fork() failed\n");
		return 0;
	}
	if (newPid == 0)
	{
		/** we are the child */
		char		**arglist;
		int			nArgs;
		int			i;

		if (execArgs != NULL)
		{
			nArgs = 0;
			while (execArgs[nArgs] != NULL)
			{
				nArgs++;
			}

			arglist = ckalloc(sizeof(char *) * nArgs + 2);
			arglist[0] = waitList->nameList_[waitList->nChildren_];
			for (i = 0; i <= nArgs; i++)
			{
				arglist[i + 1] = execArgs[i];
			}

		} else
		{
			dummyPtr[0] = waitList->nameList_[waitList->nChildren_];
			dummyPtr[1] = NULL;
			arglist = dummyPtr;
		}

		for (i = 0; arglist[i] != NULL; i++)
		{
			LogInfo("	%3d '%s'\n", i, arglist[i]);
		}

		execvp(waitList->nameList_[waitList->nChildren_], arglist);
		LogCrit("exec failed!\n");
		exit(1);
	}
	waitList->pidList_[waitList->nChildren_] = newPid;

	LogInfo("Started '%s' with pid %d\n",
			waitList->nameList_[waitList->nChildren_],
			waitList->pidList_[waitList->nChildren_]);
#else
	LogError("Cannot run child on NT\n");
#endif

	waitList->nChildren_++;

	return 1;
}


OS_EXPORT int
gPltRun(gWaitList * waitList, const char *filename,...)
{
	va_list		args;
	char		*curArg, **arglist;
	int			nArgsPlusOne;
	int			status;
	int			i;

	/** after this loop, we will have counted the args, plus NULL */
	nArgsPlusOne = 0;
	va_start(args, filename);
	do
	{
		curArg = va_arg(args, char *);
		nArgsPlusOne++;
	} while (curArg != NULL);
	va_end(args);


	/** now allocate a vector for the args */
	arglist = ckalloc(sizeof(char *) * nArgsPlusOne);
	va_start(args, filename);
	for (i = 0; i < nArgsPlusOne; i++)
	{
		arglist[i] = va_arg(args, char *);
	}
	va_end(args);

	status = gPltRunV(waitList, filename, arglist);
	ckfree(arglist);

	return status;
}

