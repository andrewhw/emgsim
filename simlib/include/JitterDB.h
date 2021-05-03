/**
 ** This class controls a definition of a single JitterDB, and
 ** contains the interface to save/restore it to a file
 ** store.
 **
 ** $Id: JitterDB.h 4 2008-04-24 21:27:41Z andrew $
 **/
#ifndef __JITTER_DB_CLASS_HEADER__
#define __JITTER_DB_CLASS_HEADER__

#include "os_defs.h"

# ifndef        MAKEDEPEND
# include       <stdio.h>
#  ifdef        OS_WINDOWS_NT
#    include    <io.h>
#  else
#    include    <unistd.h>
#  endif
#  include      <sys/types.h>
#  include      <fcntl.h>
# endif

#include "io_utils.h"


/**
 ** JitterDB Data structure.
 **/
class JitterDB {
public:
		        static const int RAW;
		        static const int GAUSS;
		        static const int BUFFER;
		        static const int MAX__;


private:        
		        // pointer to self, initialized when accessed.
		        static JitterDB *sThis_[];
		        static float    sSamplingRate_;

		        // create, destroy self
		         JitterDB();
		        ~JitterDB();

private:
		        double  max_, min_, avg_;
		        long    nElements_;
		        long    outliers_;

		        long    nBuckets_;
		        long    *allocBuckets_;
		        long    *buckets_;
		        char    *pathstem_;
		        FP      *logFP_;

private:
		        // internal functions
		        void clear__(void);

		        void logValue__(double jitterValue, double tag);
public:

		        // get, and possibly allocate, the singleton instance
		        static JitterDB *sGet(int id);

		        static void sSetSamplingRate(float rate);
		        static float sGetSamplingRate();

		        // clean up after ourselves
		        static void sCleanup();
		        static void sCleanup(int id);

		        void addJitter(double jitterValue, double tag);
		        void addJitter(double jitterValue);
		        void setOutputDir(const char *path);

		        // sync to the file store
		        int saveHist(void);

		        // dump for debugging
		        void dump(FILE *fp);
};

#endif

