/**
 ** Define the EMG file when loaded into memory
 **
 ** $Id: emg.h 8 2008-04-24 22:22:33Z andrew $
 **/

#ifndef __EMG_DEFINITION_HEADER__
#define __EMG_DEFINITION_HEADER__

#include "emgdat.h"
#include "os_types.h"


typedef osInt16   emgValue;

typedef struct EmgData {
	EmgHeader	definition_;
	emgValue	*data_;
	float		*scaledData_;
} EmgData;



extern EmgData *createEmgData();
extern void deleteEmgData(EmgData *data);

extern void dumpEmg(
		FILE *dumpfp,
		int indent,
		EmgData *data
	    );
extern void dumpEmgVerbose(
		FILE *dumpfp,
		int indent,
		EmgData *data
	    );


extern EmgData *readEmgFile(const char *inputFile);

extern int writeEmgFile(const char *outputFile, EmgData *data);

extern void scaleEmgData(EmgData *data);


#endif /* __EMG_DEFINITION_HEADER__ */

