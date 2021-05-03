/**
 ** Handle interface to user-supplied values.
 **
 ** $Id: userinput.h 6 2008-04-24 22:22:40Z andrew $
 **/

#ifndef         __USER_INPUT_HEADER__
#define         __USER_INPUT_HEADER__

extern void dumpGlobals();
extern void fixPathnames(struct globals *g);
extern void loadValues(struct globals *g);
extern char *getUserInput(const char *message);
extern void dumpGlobalSettings(struct globals *g);
extern int validateGlobals(struct globals *g, int *quitFlag);
extern int saveGlobalDirectoryInfo(
		        struct globals *g,
		        const char *userPath
		    );
extern int saveOutputDirConfigFile(struct globals *g, int fileId);
extern int reuseGlobalDirectoryInfo(struct globals *g);
extern int setupGlobalDirectoryInfoForOpen(
		        struct globals *g,
		        const char *path
		);

#endif          /* __USER_INPUT_HEADER__ */


