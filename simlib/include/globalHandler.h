/**
 ** Wrap up the stuff we will use to read data from the globals
 **
 ** $Id: globalHandler.h 4 2008-04-24 21:27:41Z andrew $
 **/

#ifndef __GLOBAL_HANDLER_HEADER__
#define __GLOBAL_HANDLER_HEADER__


class DQEmgData;

int make16bit(struct globals *g,
				DQEmgData *dqemgData,
		        int fileId,
		        int newMuscle = 0);
int makeDco(struct globals *g, int fileId);

#endif /* __GLOBAL_HANDLER_HEADER__ */


