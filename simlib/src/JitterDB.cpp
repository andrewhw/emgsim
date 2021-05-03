/**
 ** EDF Read/Write tools
 **
 ** $Id: JitterDB.cpp 4 2008-04-24 21:27:41Z andrew $
 **/

#include "os_defs.h"

# ifndef        MAKEDEPEND
#  include      <stdio.h>
#  include      <string.h>
#  include      <math.h>
#  include      <errno.h>
#  ifdef        OS_WINDOWS_NT
#    include    <io.h>
#  else
#    include    <unistd.h>
#    include    <sys/types.h>
#    include    <fcntl.h>
#  endif
# endif

#include "SimulatorControl.h"
#include "JitterDB.h"

#include "stringtools.h"
#include "io_utils.h"
#include "listalloc.h"
#include "random.h"

#include "tclCkalloc.h"
#include "error.h"
#include "massert.h"


#define         HALF_N_BUCKETS  40

const int JitterDB::RAW                 = 0;
const int JitterDB::GAUSS               = 1;
const int JitterDB::BUFFER              = 2;
const int JitterDB::MAX__               = 3;

		/*
		 * pointer to self
		 */
JitterDB *JitterDB::sThis_[MAX__] = { NULL };
float JitterDB::sSamplingRate_ = 1.0;


JitterDB::JitterDB()
{
		memset(this, 0, sizeof(JitterDB));

		nBuckets_ = (HALF_N_BUCKETS * 2) + 1;

		allocBuckets_ = (long *) ckalloc(sizeof(long) * nBuckets_);
		memset(allocBuckets_, 0, sizeof(long) * nBuckets_);

		buckets_ = &allocBuckets_[HALF_N_BUCKETS];
}

JitterDB::~JitterDB()
{
		clear__();
}

void
JitterDB::clear__(void)
{
		if (allocBuckets_ != NULL)
		{
		        ckfree(allocBuckets_);
		        buckets_ = allocBuckets_ = NULL;
		}
		if (pathstem_ != NULL)
		        ckfree(pathstem_);
		if (logFP_ != NULL)
		        closeFP(logFP_);
}

void
JitterDB::logValue__(double jitterValue, double tag)
{
		if (logFP_ == NULL)
		{
		        char *filename;

		        filename = strconcat(pathstem_, ".jlg", NULL);
		        logFP_ = openFP(filename, "a");
		        ckfree(filename);
		        MSG_ASSERT(logFP_ != NULL, "Cannot open Jitter log file");
		}

		fputs(niceDouble(tag), logFP_->fp);
		fputc(' ', logFP_->fp);
		fputs(niceDouble(jitterValue * sSamplingRate_), logFP_->fp);
		/* fputs(niceDouble(jitterValue), logFP_->fp); */
		fputc('\n', logFP_->fp);
}

void
JitterDB::addJitter(double jitterValue, double tag)
{
		addJitter(jitterValue);
		logValue__(jitterValue, tag);
}

void
JitterDB::addJitter(double jitterValue)
{
		long bucketValue;
		double truncValue = jitterValue * 10.0;

		if (truncValue >= 0.0)
		        bucketValue = (long) (truncValue + 0.5);
		else
		        bucketValue = (long) (truncValue - 0.5);

		nElements_++;

		if (nElements_ == 1)
		{
		        min_ = max_ = jitterValue;
		} else
		{
		        if (min_ > jitterValue)         min_ = jitterValue;
		        if (max_ < jitterValue)         max_ = jitterValue;
		}

		avg_ = avg_ + jitterValue;

		if ((bucketValue > HALF_N_BUCKETS) || (bucketValue <= (-HALF_N_BUCKETS)))
		{
		        outliers_++;
		} else
		{
		        buckets_[bucketValue]++;
		}
}

float
JitterDB::sGetSamplingRate()
{
		return sSamplingRate_;
}

void
JitterDB::sSetSamplingRate(float rate)
{
		sSamplingRate_ = rate;
}


JitterDB *
JitterDB::sGet(int id)
{
		if (sThis_[id] == NULL)
		{
		        sThis_[id] = new JitterDB();
		}

		return sThis_[id];
}

void
JitterDB::sCleanup(void)
{
		int i;
		for (i = 0; i < MAX__; i++)
		{
		        sCleanup(i);
		}
}


void
JitterDB::sCleanup(int id)
{
		if (sThis_[id] != NULL)
		{
		        delete sThis_[id];
		}

		sThis_[id] = NULL;
}


void
JitterDB::setOutputDir(const char *path)
{
		if (pathstem_ != NULL)
		{
		        ckfree(pathstem_);
		        if (logFP_ != NULL)
				{
		                closeFP(logFP_);
		                logFP_ = NULL;
		        }
		}
		pathstem_ = ckstrdup(path);
}

int
JitterDB::saveHist(void)
{
		FP *fp;
		char *path;
		int status = 1;

		path = strconcat(pathstem_, ".hist", NULL);

		fp = openFP(path, "wb");

		ckfree(path);

		MSG_ASSERT(fp != NULL, "Cannot open JitterDB data file");
		dump(fp->fp);
		closeFP(fp);

		if (logFP_ != NULL)
		{
		        closeFP(logFP_);
		        logFP_ = NULL;
		}

		return status;
}

void
JitterDB::dump(FILE *fp)
{
		int i;

/*
		fprintf(fp, "Jitter Values:\n");
		fprintf(fp, "\n");

		fprintf(fp, "  %ld elements\n", nElements_);
		fprintf(fp, "  outliers: %ld\n", outliers_);
		fprintf(fp, "       min: %f\n", min_);
		fprintf(fp, "       max: %f\n", max_);
		fprintf(fp, "       avg: %f\n",
		                        (double) (avg_ / (double) nElements_));

		fprintf(fp, "\n");
		fprintf(fp, "\n");
*/

		for (i = ((-HALF_N_BUCKETS) + 1); i < HALF_N_BUCKETS; i++)
		{
		        /* fprintf(fp, "%d %ld\n", i, buckets_[i]); */
		        fprintf(fp, "%f %ld\n", i/100.0, buckets_[i]);
		}

}


