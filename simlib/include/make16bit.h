/**
 ** Utilities to convert a file of floats to a file of 16-bit
 ** values.
 **
 ** $Id: make16bit.h 5 2008-04-24 22:05:30Z andrew $
 **/

#ifndef __MAKE_16_BIT_HEADER__
#define __MAKE_16_BIT_HEADER__

#include        "emgdat.h"

typedef struct EmgVoltageDesc {
		float   minVoltage_;
		float   maxVoltage_;
		float   maxAbsVoltage_;
		int             nFloats_;
} EmgVoltageDesc;

class DQEmgData;

int make16bitFile(
		DQEmgData *dqemgData,
		const char *outputfile,
		const char *inputfile,
		short maxShortVoltage,
		const char *textfile,
		int doTextOutput
	);

#endif /* __MAKE_16_BIT_HEADER__ */


