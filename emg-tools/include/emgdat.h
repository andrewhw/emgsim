/**
 ** Utilities to convert a file of floats to a file of 16-bit
 ** values.
 **
 ** $Id: emgdat.h 8 2008-04-24 22:22:33Z andrew $
 **/

#ifndef __EMG_DAT_FILE_HEADER__
#define __EMG_DAT_FILE_HEADER__

#include "io_utils.h"
#include "os_types.h"

typedef struct EmgHeader
{
    osInt16	emg_channel;
    osInt16	emg_hpCutoff;
    osInt16	emg_lpCutoff;
    osInt16	emg_scale;
    osInt32	emg_samplingRate;
    osInt32	emg_numberOfSamples;
    osInt32	emg_elapsedTime;
    osInt16	emg_compressionThreshold;

    struct {
	float	effectiveScale_;
}       work_;
} EmgHeader;



#define wEmgDat(fp, value)	w2byteInt((fp), (value))
#define rEmgDat(fp, value)	r2byteInt((fp), (value))

extern int writeEmgDatHeader(FP *outputFile, EmgHeader *header);
extern int readEmgDatHeader(FP *inputFile, EmgHeader *header);

extern int dumpEmgDatHeader(FILE *fp, int indent, EmgHeader *header);

#endif /* __EMG_DAT_FILE_HEADER__ */


