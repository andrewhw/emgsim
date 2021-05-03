/**
 * $Id: testNoChannels.cpp 65 2008-11-23 15:46:07Z andrew $
 */

//#include "DQEmgDataOSdefs.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "DQEmgData.h"


static int stringMatches(const char *s1, const char *s2)
{
    if (s1 == NULL)
	{
		printf("<FAIL> matchstring 1 is NULL\n");
		return 0;
    }
    if (s2 == NULL)
	{
		printf("<FAIL> matchstring 2 is NULL\n");
		return 0;
    }

    if (strcmp(s1, s2) == 0)
		return 1;

    return 0;
}

int
testNoChannels()
{
    const char *FILENAME	= "nochannels.dat";


    const char *VENDOR_ID	= "Viking";
    const char *PATIENT		= "EMG Lund 001";
    const char *MUSCLE		= "1st dorsal interosseous";

    int GENDER				= 0;
    const DQEmgData::Side SIDE = DQEmgData::LEFT;

    const unsigned long AGE	= 0;


    printf("<TEST> testing Judex no-channel data\n");


    {
		DQEmgData *loadData;
		unsigned long age;

		loadData = new DQEmgData();
		if ( loadData->load(FILENAME) <= 0)
		{
		    printf("<FAIL> Cannot load data file '%s'\n", FILENAME);
			return 0;
		}

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID))
		{
		    printf("<PASS> vendor id match\n");
		} else
		{
		    printf("<FAIL> vendor id match: got [%s], want [%s]\n",
						loadData->getVendorIdentifier(), VENDOR_ID);
		}

		if (loadData->getOperatorDescription() == NULL)
		{
		    printf("<PASS> operator match\n");
		} else
		{
		    printf("<FAIL> operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT))
		{
		    printf("<PASS> patient match\n");
		} else
		{
		    printf("<FAIL> patient match got[%s], want [%s]\n",
						loadData->getSubjectDescription(), PATIENT);
		}

		if (loadData->getSubjectID() == NULL)
		{
		    printf("<PASS> patient ID match\n");
		} else
		{
		    printf("<FAIL> patient ID match\n");
		}

		age = loadData->getSubjectAge();
		if (age == AGE)
		{
		    printf("<PASS> age match\n");
		} else
		{
		    printf("<FAIL> age match got %lu, want %lu\n", age, AGE);
		}


		if (loadData->getSubjectGender() == GENDER)
		{
		    printf("<PASS> gender match\n");
		} else
		{
		    printf("<FAIL> gender match: got %u, want %d\n",
		    				loadData->getSubjectGender(), GENDER);
		}

		if (loadData->getMuscleSide() == SIDE)
		{
		    printf("<PASS> muscle side match\n");
		} else
		{
		    printf("<FAIL> muscle side match: got %u, want %u\n",
		    				loadData->getMuscleSide(), SIDE);
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE))
		{
		    printf("<PASS> muscle description match\n");
		} else
		{
		    printf("<FAIL> muscle description match got[%s], want [%s]\n",
						loadData->getMuscleDescription(), MUSCLE);
		}

		if (loadData->getGeneralDescription() == NULL)
		{
		    printf("<PASS> description match\n");
		} else
		{
		    printf("<FAIL> description match got[%s], want [%s]\n",
						loadData->getGeneralDescription(), "NULL");
		}

		/** channel data tests */
		if (loadData->getNumChannels() == 0)
		{
		    printf("<PASS> num channels\n");
		} else
		{
		    printf("<FAIL> channels are reported in this data!\n");
		}

		delete loadData;
    }

    return 1;
}

