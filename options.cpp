/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: options.cpp 11 2011-03-23 11:29:15Z andrew $
 **/
#include <os_defs.h>

#ifndef MAKEDEPEND
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

#  ifdef				OS_WINDOWS_NT
#   include <io.h>
#  endif

# include <ctype.h>
# include <sys/stat.h>
#endif

#include "SimulatorControl.h"
#include "dco.h"

#include "pathtools.h"
#include "stringtools.h"
#include "random.h"
#include "userinput.h"
#include "error.h"
#include "tclCkalloc.h"
#include "log.h"

#include "options.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


static void printVersion(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	LogInfo("\n");
	LogInfo("%s - Muscle Simulator Version %s\n",
			opts->progname, __PROG_VERSION__);
#ifdef _DEBUG
	LogInfo("DEBUG version\n");
#endif
	LogInfo("	Compiled : %s %s\n", __DATE__, __TIME__);
	LogInfo("\n");
	exit (1);
}

static void doWaitForKey(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->waitForKeyToExit = 1;
}
static void doNoWaitForKey(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->waitForKeyToExit = 0;
}

//static void doTextOutput(
//		struct optionflags *opts,
//		const char *arg,
//		const char *tag
//	)
//{
//	opts->textOutput = 1;
//}
//static void doNoTextOutput(
//		struct optionflags *opts,
//		const char *arg,
//		const char *tag
//	)
//{
//	opts->textOutput = 0;
//}

static void doRunSurface(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->runSurface = 1;
}
static void doNoRunSurface(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->runSurface = 0;
}

static void doRunOnce(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->runLoop = 0;
}
static void doRunLoop(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->runLoop = 1;
}

static void doSetConfigFilePath(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	int len = strlen(tag) + 1;
	opts->configFilePath = ckstrdup(&arg[len]);
}
static void doSetDestinationRoot(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	int len = strlen(tag) + 1;
	opts->destinationRoot = ckstrdup(&arg[len]);
}

static void doSkipConfirm(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->skipValidateGlobals = 1;
	opts->runLoop = 0;
	opts->waitForKeyToExit = 0;
}
static void doReadMuscle(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->readMuscle = 1;
}
static void doWriteMuscle(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->readMuscle = 0;
}
static void doUseLastMuscle(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->useLastMuscle = 1;
}
static void doUseOldFiringTimes(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->useOldFiringTimes = 1;
}
static void doUseNewFiringTimes(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->useOldFiringTimes = 0;
}

static void doUseDQEmgDataFormat(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	opts->DQEmgDataFormat = 1;
}

#ifdef  OS_WINDOWS_NT
/** only allow drive setting on NT */
static void doSetDrive(
		struct optionflags *opts,
		const char *arg,
		const char *tag
	)
{
	const char *newDrive = &arg[strlen("-use-drive-")];
	int lowerDrive;

	lowerDrive = tolower(*newDrive);

	if (lowerDrive <= 'a' || lowerDrive >= 'z') {
		Error("Invalid drive in '-use-drive'\n");
		exit (1);
	}

	opts->driveLetter[0] = lowerDrive;
}
#endif

struct option_set {
	const char  *tag;
	const char  *ext;
	const char  *help;
	void (*action)(struct optionflags *, const char *, const char *);
} sOptionSet[] = {
		{"skip-confirm",		NULL,
			"use default values - skip user value confirmation",
			doSkipConfirm	   },
		{"write-muscle",		NULL,
			"write the muscle file, don't read it",
			doWriteMuscle	   },
		{"read-muscle",		 NULL,
			"read the muscle file, don't write it",
			doReadMuscle		},

		{"nowait-for-key",	  NULL,
			"do not wait for ENTER to be pressed before exitting",
			doNoWaitForKey	  },
		{"wait-for-key",		NULL,
			"wait for ENTER to be pressed before exitting",
			doWaitForKey		},

//		{"text",	  NULL,
//			"produce an emg text output file",
//			doTextOutput	  },
//		{"notext",		NULL,
//			"do not produce an emg text output file",
//			doNoTextOutput		},

		{"norunSurface",	  NULL,
			"Do not run the surface generation routines",
			doNoRunSurface	  },
		{"runSurface",		NULL,
			"Run the surface generation routines (default)",
			doRunSurface		},

		{"runLoop",	  NULL,
			"Loop around for multiple studies",
			doRunLoop	  },
		{"runOnce",		NULL,
			"Only run one study",
			doRunOnce		},

		{"configuration-dir=",	  "<DIR>",
			"Specify the directory containing the simulator.cfg file",
			doSetConfigFilePath	  },
		{"data-root=",		"<DIR>",
			"Specify the directory in which to build the output run<N> directories",
			doSetDestinationRoot		},

#ifdef  OS_WINDOWS_NT
		{"use-drive=",		  "<X>",
			"set to new drive to write data on",
			doSetDrive  },
#endif
		{"useLastMuscle",	  NULL,
			"run again using information from last muscle",
			doUseLastMuscle	  },
		{"useOldFiringTimes",	  NULL,
			"if using last muscle, use old times too",
			doUseOldFiringTimes	  },
		{"useNewFiringTimes",	  NULL,
			"if using last muscle, generate new firing times (default)",
			doUseNewFiringTimes	  },
		{"DQEmgData",   NULL,
			"Generate files in DQEmgData format",
			doUseDQEmgDataFormat		},
		{"version",					 NULL,
			"print version info",
			printVersion		},
		{ NULL, NULL, NULL, NULL }
	};

/** forward declarations */
void printHelp(
		FILE *fp,
		const char *progname,
		struct option_set *options);


int parseOptions(struct optionflags *flags, int argc, char **argv)
{
	int i, j, k;
	int foundOption;

	memset(flags, 0, sizeof(struct optionflags));
	flags->progname = strrchr(argv[0], OS_PATH_DELIM);
	if (flags->progname == NULL)
		flags->progname = argv[0];
	else
		flags->progname++;

	//flags->runSurface = 1;
//	flags->useOldFiringTimes = 1;


	flags->textOutput = 0;

#ifdef  OS_WINDOWS_NT
	flags->waitForKeyToExit = 1;
	strcpy(flags->driveLetter, DEFAULT_DRIVE);
	flags->runLoop = 1;
#else
	flags->runLoop = 0;
#endif

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			k = 1;
			if (argv[i][k] == '-')
				k++;

			if (strcmp(&argv[i][k], "help") == 0) {
				flags->waitForKeyToExit = 0;
				flags->runLoop = 0;
				printHelp(stdout, argv[0], sOptionSet);
				return 0;
			}
			foundOption = 0;
			for (j = 0; sOptionSet[j].tag != NULL; j++) {
				if (strncmp(&argv[i][k],
						sOptionSet[j].tag,
						strlen(sOptionSet[j].tag)) == 0) {
					(*sOptionSet[j].action)(flags,
							argv[i], sOptionSet[j].tag);
					foundOption = 1;
					break;
				}
			}

			if ( ! foundOption) {
				fprintf(stderr, "Unknown option '%s' -- aborting\n",
						argv[i]);
				printHelp(stdout, argv[0], sOptionSet);
				return 0;
			}
		} else {
			fprintf(stderr,
					"Unknown argument '%s' -- ignoring\n", argv[i]);
		}
	}

	return 1;
}

void
cleanOptions(struct optionflags *flags)
{
	if (flags->configFilePath != NULL)
		ckfree(flags->configFilePath);

	if (flags->destinationRoot != NULL)
		ckfree(flags->destinationRoot);
}

void printHelp(
		FILE *fp,
		const char *progname,
		struct option_set *options
	)
{
	char opt[BUFSIZ];
	int maxLen, len;
	int i;

	maxLen = 0;
	for (i = 0; options[i].tag != NULL; i++) {
		if (options[i].ext != NULL)
			len = strlen(options[i].tag) + strlen(options[i].ext);
		else
			len = strlen(options[i].tag);
		if (maxLen < len)
			maxLen = len;
	}

	fprintf(fp, "%s <options>\n", progname);
	fprintf(fp, "\n");
	fprintf(fp, "Options:\n");

	for (i = 0; options[i].tag != NULL; i++) {
		if (options[i].ext != NULL)
			slnprintf(opt, BUFSIZ, "%s%s",
					options[i].tag, options[i].ext);
		else
			slnprintf(opt, BUFSIZ, "%s", options[i].tag);
		fprintf(fp, "  -%-*s : %s\n", maxLen, opt, options[i].help);
	}
	fprintf(fp, "\n");
}

