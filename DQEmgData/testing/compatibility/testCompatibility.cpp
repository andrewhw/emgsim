/**
 * $Id: testCompatibility.cpp 65 2008-11-23 15:46:07Z andrew $
 */

//#include "DQEmgDataOSdefs.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "DQEmgData.h"

#include "testutils.h"


static int stringMatches(const char *s1, const char *s2)
{
	if (s1 == NULL)
	{
		FAIL(MK, "matchstring 1 is NULL\n");
		return 0;
	}
	if (s2 == NULL)
	{
		FAIL(MK, "matchstring 2 is NULL\n");
		return 0;
	}

	if (strcmp(s1, s2) == 0)
		return 1;

	return 0;
}

static int
testNoData()
{
	const char *TESTFILENAME		= "tmpfile.dat";


	const char *VENDOR_ID		= "uw-test";
	const char *OPERATOR		= "Testsuite Operator : Fred \"Testy\"";
	const char *PATIENT				= "Testsuite Patient : Betty \'The Patient\' Subject";
	const char *PATIENT_ID		= "abc123";
	const char *MUSCLE				= "Bicep : A Great Big Muscle";
	const char *GENERALDESC		= "Characterization: A=\"b\";c=0.2;d=12;";

	const DQEmgData::Gender GENDER		= DQEmgData::FEMALE;
	const DQEmgData::Side SIDE				= DQEmgData::LEFT;

	const unsigned long DOB_YEAR		= 1756;
	const unsigned long DOB_MONTH		= 7;
	const unsigned long DOB_DAY				= 30;
	const unsigned long AGE				= 248;

	const char *channelDescriptions[] = {
		"Needle", "Surface", NULL
	};


	DQEmgData *data;
	DQEmgChannelData *channel0;
	DQEmgChannelData *channel1;
	time_t now;


	TEST(MK, "no data\n");

	now = time(NULL);
	data = new DQEmgData(VENDOR_ID,
					now,
				OPERATOR,
				PATIENT,
				PATIENT_ID,
				DOB_YEAR, DOB_MONTH, DOB_DAY,
				AGE,
				GENDER,
				MUSCLE,
				SIDE,
				GENERALDESC
			);

	channel0 = new DQEmgChannelData(
					0, 100000, 100,
				channelDescriptions[0]);

	channel1 = new DQEmgChannelData(
					1, 200000, 200,
				channelDescriptions[1]);

	data->addChannel(channel0);
	data->addChannel(channel1);


	data->store(TESTFILENAME);
	{
		DQEmgData *loadData;
		unsigned long day, month, year;

		loadData = new DQEmgData();
		loadData->load(TESTFILENAME);

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID))
		{
			PASS(MK, "vendor id match\n");
		} else
		{
			FAIL(MK, "vendor id match\n");
		}

		if (stringMatches(loadData->getOperatorDescription(), OPERATOR))
		{
			PASS(MK, "operator match\n");
		} else
		{
			FAIL(MK, "operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT))
		{
			PASS(MK, "patient match\n");
		} else
		{
			FAIL(MK, "patient match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		loadData->getSubjectDateOfBirth(&year, &month, &day);


		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY)
		{
			PASS(MK, "DOB match\n");
		} else
		{
			FAIL(MK, "DOB match\n");
		}


		if (loadData->getSubjectGender() == GENDER)
		{
			PASS(MK, "gender match\n");
		} else
		{
			FAIL(MK, "gender match\n");
		}

		if (loadData->getMuscleSide() == SIDE)
		{
			PASS(MK, "muscle side match\n");
		} else
		{
			FAIL(MK, "muscle side match\n");
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE))
		{
			PASS(MK, "muscle description match\n");
		} else
		{
			FAIL(MK, "muscle description match\n");
		}

		if (stringMatches(loadData->getGeneralDescription(), GENERALDESC))
		{
			PASS(MK, "description match\n");
		} else
		{
			FAIL(MK, "description match\n");
		}

		delete loadData;
	}

	delete data;
	unlink(TESTFILENAME);

	return 1;
}


static int
testSimpleReadWrite()
{
	const char *TESTFILENAME		= "tmpfile.dat";


	const char *VENDOR_ID		= "uw-test";
	const char *OPERATOR		= "Testsuite Operator : Fred \"Testy\"";
	const char *PATIENT				= "Testsuite Patient : Betty \'The Patient\' Subject";
	const char *PATIENT_ID		= "abc123";
	const char *MUSCLE				= "Bicep : A Great Big Muscle";
	const char *GENERALDESC		= "Characterization: A=\"b\";c=0.2;d=12;";

	const DQEmgData::Gender GENDER		= DQEmgData::FEMALE;
	const DQEmgData::Side SIDE				= DQEmgData::LEFT;

	const unsigned long DOB_YEAR		= 1756;
	const unsigned long DOB_MONTH		= 7;
	const unsigned long DOB_DAY				= 30;
	const unsigned long AGE				= 72;

	const float SAMPLING_RATE		= 31250;

	const float SCALE						= 0.5;

	const int N_DATA_ELEMENTS		= 16;

	const char *channelDescriptions[] = {
		"Needle", "Surface", NULL
	};

	short channel0Data[N_DATA_ELEMENTS];
	short channel1Data[N_DATA_ELEMENTS];


	DQEmgData *data;
	DQEmgChannelData *channel0;
	DQEmgChannelData *channel1;
	time_t now;
	int i;


	TEST(MK, "simple read/write\n");

	for (i = 0; i < N_DATA_ELEMENTS; i++)
	{
		channel0Data[i] = i * 10;
		channel1Data[i] = i * (-10);
	}


	now = time(NULL);
	data = new DQEmgData(VENDOR_ID,
					now,
				OPERATOR,
				PATIENT,
				PATIENT_ID,
				DOB_YEAR, DOB_MONTH, DOB_DAY,
				AGE,
				GENDER,
				MUSCLE,
				SIDE,
				GENERALDESC
			);

	channel0 = new DQEmgChannelData(
					0, 100000, 100,
				channelDescriptions[0]);
	channel0->setData(N_DATA_ELEMENTS, channel0Data,
							SAMPLING_RATE, "µV", SCALE);

	channel1 = new DQEmgChannelData(
					1, 200000, 200,
				channelDescriptions[1]);
	channel1->setData(N_DATA_ELEMENTS, channel1Data,
							SAMPLING_RATE, "µV", SCALE);


	data->addChannel(channel0);
	data->addChannel(channel1);


	data->store(TESTFILENAME);
	{
		DQEmgData *loadData;
		unsigned long day, month, year;

		loadData = new DQEmgData();
		loadData->load(TESTFILENAME);

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID))
		{
			PASS(MK, "vendor id match\n");
		} else
		{
			FAIL(MK, "vendor id match\n");
		}

		if (stringMatches(loadData->getOperatorDescription(), OPERATOR))
		{
			PASS(MK, "operator match\n");
		} else
		{
			FAIL(MK, "operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT))
		{
			PASS(MK, "patient match\n");
		} else
		{
			FAIL(MK, "patient match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		loadData->getSubjectDateOfBirth(&year, &month, &day);


		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY)
		{
			PASS(MK, "DOB match\n");
		} else
		{
			FAIL(MK, "DOB match\n");
		}


		if (loadData->getSubjectGender() == GENDER)
		{
			PASS(MK, "gender match\n");
		} else
		{
			FAIL(MK, "gender match\n");
		}

		if (loadData->getMuscleSide() == SIDE)
		{
			PASS(MK, "muscle side match\n");
		} else
		{
			FAIL(MK, "muscle side match\n");
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE))
		{
			PASS(MK, "muscle description match\n");
		} else
		{
			FAIL(MK, "muscle description match\n");
		}

		if (stringMatches(loadData->getGeneralDescription(), GENERALDESC))
		{
			PASS(MK, "description match\n");
		} else
		{
			FAIL(MK, "description match\n");
		}


		{
			DQEmgChannelData *loadChannel;
			short *loadChannelData;
			unsigned long loadNumSamples;
			float loadSamplingRate;
			float loadScale;
			int dataOk;

			loadChannel = loadData->getChannel(0);
			if (loadChannel == NULL)
			{
				FAIL(MK, "channel 0 is null\n");
			} else
			{

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescriptions[0]))
				{
					PASS(MK, "channel description match\n");
				} else
				{
					FAIL(MK, "channel 0 description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (i = 0; i < N_DATA_ELEMENTS; i++)
				{
					if (loadChannelData[i] != i * 10)
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel 0 data ok\n");
				} else
				{
					FAIL(MK, "channel 0 data ok\n");
				}
			}

		   
			loadChannel = loadData->getChannel(1);
			if (loadChannel == NULL)
			{
				FAIL(MK, "channel 1 is null\n");
			} else
			{

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescriptions[1]))
				{
					PASS(MK, "channel 1 description match\n");
				} else
				{
					FAIL(MK, "channel 1 description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (i = 0; i < N_DATA_ELEMENTS; i++)
				{
					if (loadChannelData[i] != i * (-10))
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel 0 data ok\n");
				} else
				{
					FAIL(MK, "channel 0 data ok\n");
				}
			}
		}
		delete loadData;
	}

	delete data;
	unlink(TESTFILENAME);

	return 1;
}

static int
testSimpleRead()
{
	const char *TESTFILENAME		= "simpleread.dat";


	const char *VENDOR_ID		= "uw-test";
	const char *OPERATOR		= "Testsuite Operator : Fred \"Testy\"";
	const char *PATIENT				= "Testsuite Patient : Betty \'The Patient\' Subject";
	const char *PATIENT_ID		= "abc123";
	const char *MUSCLE				= "Bicep : A Great Big Muscle";
	const char *GENERALDESC		= "Characterization: A=\"b\";c=0.2;d=12;";

	const DQEmgData::Gender GENDER		= DQEmgData::FEMALE;
	const DQEmgData::Side SIDE				= DQEmgData::LEFT;

	const unsigned long DOB_YEAR		= 1756;
	const unsigned long DOB_MONTH		= 7;
	const unsigned long DOB_DAY				= 30;

	const float SAMPLING_RATE		= 31250;

	const float SCALE						= 0.5;

	const int N_DATA_ELEMENTS		= 16;

	const char *channelDescriptions[] = {
		"Needle", "Surface", NULL
	};

	short channel0Data[N_DATA_ELEMENTS];
	short channel1Data[N_DATA_ELEMENTS];

	int i;


	TEST(MK, "simple read/write\n");

	for (i = 0; i < N_DATA_ELEMENTS; i++)
	{
		channel0Data[i] = i * 10;
		channel1Data[i] = i * (-10);
	}

	{
		DQEmgData *loadData;
		unsigned long day, month, year;

		loadData = new DQEmgData();
		loadData->load(TESTFILENAME);

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID))
		{
			PASS(MK, "vendor id match\n");
		} else
		{
			FAIL(MK, "vendor id match\n");
		}

		if (stringMatches(loadData->getOperatorDescription(), OPERATOR))
		{
			PASS(MK, "operator match\n");
		} else
		{
			FAIL(MK, "operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT))
		{
			PASS(MK, "patient match\n");
		} else
		{
			FAIL(MK, "patient match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		loadData->getSubjectDateOfBirth(&year, &month, &day);


		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY)
		{
			PASS(MK, "DOB match\n");
		} else
		{
			FAIL(MK, "DOB match\n");
		}


		if (loadData->getSubjectGender() == GENDER)
		{
			PASS(MK, "gender match\n");
		} else
		{
			FAIL(MK, "gender match\n");
		}

		if (loadData->getMuscleSide() == SIDE)
		{
			PASS(MK, "muscle side match\n");
		} else
		{
			FAIL(MK, "muscle side match\n");
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE))
		{
			PASS(MK, "muscle description match\n");
		} else
		{
			FAIL(MK, "muscle description match\n");
		}

		if (stringMatches(loadData->getGeneralDescription(), GENERALDESC))
		{
			PASS(MK, "description match\n");
		} else
		{
			FAIL(MK, "description match\n");
		}


		{
			DQEmgChannelData *loadChannel;
			short *loadChannelData;
			unsigned long loadNumSamples;
			float loadSamplingRate;
			float loadScale;
			int dataOk;

			loadChannel = loadData->getChannel(0);
			if (loadChannel == NULL)
			{
				FAIL(MK, "channel 0 is null\n");
			} else
			{

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescriptions[0]))
				{
					PASS(MK, "channel description match\n");
				} else
				{
					FAIL(MK, "channel 0 description match\n");
				}

				if (loadChannel->getScaleFactor() == SCALE)
				{
					PASS(MK, "scale match\n");
				} else
				{
					FAIL(MK, "scale match\n");
				}
				if (loadChannel->getSamplingRate() == SAMPLING_RATE)
				{
					PASS(MK, "sampling rate match\n");
				} else
				{
					FAIL(MK, "sampling rate match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (i = 0; i < N_DATA_ELEMENTS; i++)
				{
					if (loadChannelData[i] != i * 10)
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel 0 data ok\n");
				} else
				{
					FAIL(MK, "channel 0 data ok\n");
				}
			}

		   
			loadChannel = loadData->getChannel(1);
			if (loadChannel == NULL)
			{
				FAIL(MK, "channel 1 is null\n");
			} else
			{

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescriptions[1]))
				{
					PASS(MK, "channel 1 description match\n");
				} else
				{
					FAIL(MK, "channel 1 description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (i = 0; i < N_DATA_ELEMENTS; i++)
				{
					if (loadChannelData[i] != i * (-10))
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel 0 data ok\n");
				} else
				{
					FAIL(MK, "channel 0 data ok\n");
				}
			}
		}
		delete loadData;
	}

	return 1;
}


static int
test128Channels()
{
	const char *TESTFILENAME = "tmpfile.dat";


	const char *VENDOR_ID	= "uw-test";
	const char *OPERATOR	= "Testsuite Operator : Fred \"Testy\"";
	const char *PATIENT		= "Testsuite Patient : Betty \'The Patient\' Subject";
	const char *PATIENT_ID	= "abc123";
	const char *MUSCLE		= "Bicep : A Great Big Muscle";
	const char *GENERALDESC	= "Characterization: A=\"b\";c=0.2;d=12;";

	const DQEmgData::Gender GENDER	= DQEmgData::FEMALE;
	const DQEmgData::Side SIDE		= DQEmgData::LEFT;

	const unsigned long DOB_YEAR	= 1756;
	const unsigned long DOB_MONTH	= 7;
	const unsigned long DOB_DAY		= 30;
	const unsigned long AGE			= 30;

	const float SAMPLING_RATE		= 31250;

	const float SCALE				= 0.5;

	const int N_DATA_ELEMENTS		= 128;
	const int N_CHANNELS			= 128;

	short channelData[N_DATA_ELEMENTS];


	DQEmgData *data;
	DQEmgChannelData *channel;
	char channelDescription[128];
	time_t now;
	int i, j;


	TEST(MK, "128 channels\n");


	now = time(NULL);
	data = new DQEmgData(VENDOR_ID,
					now,
				OPERATOR,
				PATIENT,
				PATIENT_ID,
				DOB_YEAR, DOB_MONTH, DOB_DAY,
				AGE,
				GENDER,
				MUSCLE,
				SIDE,
				GENERALDESC
			);

	for (i = 0; i < N_CHANNELS; i++)
	{
		sprintf(channelDescription, "channel_%03d", i);
		for (j = 0; j < N_DATA_ELEMENTS; j++)
		{
			channelData[j] = i * 10;
		}
		channel = new DQEmgChannelData(
					i, 10000 * i, 100 * i,
				channelDescription);
		channel->setData(N_DATA_ELEMENTS, channelData,
							SAMPLING_RATE, "µV", SCALE);

		data->addChannel(channel);
	}


	data->store(TESTFILENAME);
	{
		DQEmgData *loadData;
		unsigned long day, month, year;

		loadData = new DQEmgData();
		loadData->load(TESTFILENAME);

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID))
		{
			PASS(MK, "vendor id match\n");
		} else
		{
			FAIL(MK, "vendor id match\n");
		}

		if (stringMatches(loadData->getOperatorDescription(), OPERATOR))
		{
			PASS(MK, "operator match\n");
		} else
		{
			FAIL(MK, "operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT))
		{
			PASS(MK, "patient match\n");
		} else
		{
			FAIL(MK, "patient match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID))
		{
			PASS(MK, "patient ID match\n");
		} else
		{
			FAIL(MK, "patient ID match\n");
		}

		loadData->getSubjectDateOfBirth(&year, &month, &day);


		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY)
		{
			PASS(MK, "DOB match\n");
		} else
		{
			FAIL(MK, "DOB match\n");
		}


		if (loadData->getSubjectGender() == GENDER)
		{
			PASS(MK, "gender match\n");
		} else
		{
			FAIL(MK, "gender match\n");
		}

		if (loadData->getMuscleSide() == SIDE)
		{
			PASS(MK, "muscle side match\n");
		} else
		{
			FAIL(MK, "muscle side match\n");
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE))
		{
			PASS(MK, "muscle description match\n");
		} else
		{
			FAIL(MK, "muscle description match\n");
		}

		if (stringMatches(loadData->getGeneralDescription(), GENERALDESC))
		{
			PASS(MK, "description match\n");
		} else
		{
			FAIL(MK, "description match\n");
		}

		/** channel data tests */
		{
			DQEmgChannelData *loadChannel;
			short *loadChannelData;
			unsigned long loadNumSamples;
			float loadSamplingRate;
			float loadScale;
			int dataOk;

			for (i = 0; i < N_CHANNELS; i++)
			{
				loadChannel = loadData->getChannel(i);
				if (loadChannel == NULL)
				{
					FAIL(MK, "NULL channel data\n");
					break;
				}

				sprintf(channelDescription, "channel_%03d", i);

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescription))
				{
					PASS(MK, "channel description match\n");
				} else
				{
					FAIL(MK, "channel description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (j = 0; j < N_DATA_ELEMENTS; j++)
				{
					if (loadChannelData[j] != i * 10)
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel description match\n");
				} else
				{
					FAIL(MK, "channel description match\n");
				}
			}
		}

		delete loadData;
	}

	delete data;
	unlink(TESTFILENAME);

	return 1;
}


static int
testBaseData()
{
	const char *TESTFILENAME = "basedata.dat";

	const char *VENDOR_ID = "uw-test";
	const char *OPERATOR = "Testsuite Operator : Fred \"Testy\"";
	const char *PATIENT = "Testsuite Patient : Betty \'The Patient\' Subject";
	const char *MUSCLE = "Bicep : A Great Big Muscle";
	const char *GENERALDESC = "Characterization: A=\"b\";c=0.2;d=12;";

	const DQEmgData::Gender GENDER = DQEmgData::FEMALE;
	const DQEmgData::Side SIDE = DQEmgData::LEFT;

	const unsigned long DOB_YEAR = 1756;
	const unsigned long DOB_MONTH = 7;
	const unsigned long DOB_DAY = 30;

	const int N_DATA_ELEMENTS = 16;

	const char *channelDescriptions[] = {
		"Needle", "Surface", NULL
	};

	TEST(MK, "base data\n");

	DQEmgData *loadData;
	unsigned long day, month, year;

	loadData = new DQEmgData();
	if (loadData->load(TESTFILENAME) <= 0) {
		FAIL(MK, "Loading of file returned failure\n");
	}

	if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID))
	{
		PASS(MK, "vendor id match\n");
	} else
	{
		FAIL(MK, "vendor id match: got [%s], expected [%s]\n",
				loadData->getVendorIdentifier(),
				VENDOR_ID);
	}

	if (stringMatches(loadData->getOperatorDescription(), OPERATOR))
	{
		PASS(MK, "operator match\n");
	} else
	{
		FAIL(MK, "operator match: got [%s], expected [%s]\n",
				loadData->getOperatorDescription(),
				OPERATOR);
	}

	if (stringMatches(loadData->getSubjectDescription(), PATIENT))
	{
		PASS(MK, "patient match\n");
	} else
	{
		FAIL(MK, "patient description: got [%s], expected [%s]\n",
				loadData->getSubjectDescription(),
				PATIENT);
	}

	loadData->getSubjectDateOfBirth(&year, &month, &day);


	if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY)
	{
		PASS(MK, "DOB match\n");
	} else
	{
		FAIL(MK, "DOB match: got [%d/%d/%d], expected [%d/%d/%d]\n",
				year, month, day,
				DOB_YEAR, DOB_MONTH, DOB_DAY);
	}


	if (loadData->getSubjectGender() == GENDER)
	{
		PASS(MK, "gender match\n");
	} else
	{
		FAIL(MK, "gender match: got [%d], expected [%d]\n",
				loadData->getSubjectGender(),
				GENDER);
	}

	if (loadData->getMuscleSide() == SIDE)
	{
		PASS(MK, "muscle side match\n");
	} else
	{
		FAIL(MK, "muscle side match: got [%d], expected [%d]\n",
				loadData->getMuscleSide(),
				SIDE);
	}

	if (stringMatches(loadData->getMuscleDescription(), MUSCLE))
	{
		PASS(MK, "muscle description match\n");
	} else
	{
		FAIL(MK, "muscle description: got [%s], expected [%s]\n",
				loadData->getMuscleDescription(),
				MUSCLE);
	}

	if (stringMatches(loadData->getGeneralDescription(), GENERALDESC))
	{
		PASS(MK, "description match\n");
	} else
	{
		FAIL(MK, "description: got [%s], expected [%s]\n",
				loadData->getGeneralDescription(),
				GENERALDESC);
	}


	{
		DQEmgChannelData *loadChannel;
		short *loadChannelData;
		unsigned long loadNumSamples;
		float loadSamplingRate;
		float loadScale;
		int dataOk;
		int i;

		{
			loadChannel = loadData->getChannel(0);

			if (loadChannel == NULL)
			{
				FAIL(MK, "loadChannel 0 is NULL\n");
			} else
			{
				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescriptions[0]))
									{
					PASS(MK, "channel description match\n");
				} else
				{
					FAIL(MK, "channel 0 description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (i = 0; i < N_DATA_ELEMENTS; i++)
				{
					if (loadChannelData[i] != i * 10)
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel 0 data ok\n");
				} else
				{
					FAIL(MK, "channel 0 data ok\n");
				}
			}
		}

		{
			loadChannel = loadData->getChannel(1);
			if (loadChannel == NULL)
			{
				FAIL(MK, "loadChannel 0 is NULL\n");
			} else
			{

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescriptions[1]))
				{
					PASS(MK, "channel 1 description match\n");
				} else {
					FAIL(MK, "channel 1 description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (i = 0; i < N_DATA_ELEMENTS; i++)
				{
					if (loadChannelData[i] != i * (-10))
						dataOk = 0;
				}

				if (dataOk)
				{
					PASS(MK, "channel 0 data ok\n");
				} else
				{
					FAIL(MK, "channel 0 data ok\n");
				}
			}
		}
	}
	delete loadData;

	return 1;
}



int
testCompatibility()
{
	int status = 1;

	status = testSimpleReadWrite() && status;
	status = testNoData() && status;
	status = test128Channels() && status;

	status = testSimpleRead() && status;
	status = testBaseData() && status;

	return status;
}

