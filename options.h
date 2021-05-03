/**
 ** Define structures used throughout the simulator
 **
 ** $Id: options.h 5 2008-04-24 22:06:09Z andrew $
 **/

#ifndef __COMMAND_LINE_OPTIONS_HEADER__
#define __COMMAND_LINE_OPTIONS_HEADER__

struct optionflags {
        char *progname;
        int skipValidateGlobals;

        int runLoop;

        int textOutput;

        int readMuscle;
        int waitForKeyToExit;
        int useLastMuscle;
        int useOldFiringTimes;

        int runSurface;
        int DQEmgDataFormat;

        char driveLetter[3];
		char *configFilePath;
		char *destinationRoot;
};


int  parseOptions(struct optionflags *flags,
                int argc, char **argv);
int  updateGlobalsWithOptions(struct globals *g,
                struct optionflags *opts);

void cleanOptions(struct optionflags *flags);

#endif /* __COMMAND_LINE_OPTIONS_HEADER__ */

