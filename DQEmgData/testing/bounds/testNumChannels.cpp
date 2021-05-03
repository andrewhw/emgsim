/**
 * $Id: testNumChannels.cpp 36 2008-09-19 18:46:54Z andrew $
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "DQEmgData.h"


static int stringMatches(const char *s1, const char *s2)
{
	if (s1 == NULL) {
		printf("<FAIL> matchstring 1 is NULL\n");
		return 0;
	}
	if (s2 == NULL) {
		printf("<FAIL> matchstring 2 is NULL\n");
		return 0;
	}

	if (strcmp(s1, s2) == 0)
		return 1;

	return 0;
}

static int
testNChannels(int nChannels)
{
	const char *TESTFILENAME		= "tmpfile.dat";


	const char *VENDOR_ID		= "uw-test";
	const char *OPERATOR		= "Operator";
	const char *PATIENT				= "Patient";
	const char *PATIENT_ID		= "id";
	const char *MUSCLE				= "bicep";

	const DQEmgData::Gender GENDER		= DQEmgData::MALE;
	const DQEmgData::Side SIDE				= DQEmgData::RIGHT;

	const unsigned long DOB_YEAR		= 2008;
	const unsigned long DOB_MONTH		= 1;
	const unsigned long DOB_DAY				= 1;

	const unsigned long AGE				= 25;

	const unsigned long SAMPLING_RATE		= 42000;

	const float SCALE						= 0.25;

	const int N_DATA_ELEMENTS				= 128;

	short channelData[N_DATA_ELEMENTS];


	DQEmgData *data;
	DQEmgChannelData *channel;
	char channelDescription[128];
	time_t now;
	int i, j;


	printf("<TEST> %d channels\n", nChannels);


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
				SIDE
			);

	for (i = 0; i < nChannels; i++) {
		sprintf(channelDescription, "channel_%03d", i);
		for (j = 0; j < N_DATA_ELEMENTS; j++) {
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
		unsigned long day, month, year, age;

		loadData = new DQEmgData();
		loadData->load(TESTFILENAME);

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID)) {
			printf("<PASS> vendor id match\n");
		} else {
			printf("<FAIL> vendor id match\n");
		}

		if (stringMatches(loadData->getOperatorDescription(), OPERATOR)) {
			printf("<PASS> operator match\n");
		} else {
			printf("<FAIL> operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT)) {
			printf("<PASS> patient match\n");
		} else {
			printf("<FAIL> patient match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID)) {
			printf("<PASS> patient ID match\n");
		} else {
			printf("<FAIL> patient ID match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID)) {
			printf("<PASS> patient ID match\n");
		} else {
			printf("<FAIL> patient ID match\n");
		}


		age = loadData->getSubjectAge();
		if (age == AGE) {
			printf("<PASS> age match\n");
		} else {
			printf("<FAIL> age match\n");
		}

		loadData->getSubjectDateOfBirth(&year, &month, &day);
		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY) {
			printf("<PASS> DOB match\n");
		} else {
			printf("<FAIL> DOB match\n");
		}


		if (loadData->getSubjectGender() == GENDER) {
			printf("<PASS> gender match\n");
		} else {
			printf("<FAIL> gender match\n");
		}

		if (loadData->getMuscleSide() == SIDE) {
			printf("<PASS> muscle side match\n");
		} else {
			printf("<FAIL> muscle side match\n");
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE)) {
			printf("<PASS> muscle description match\n");
		} else {
			printf("<FAIL> muscle description match\n");
		}

		if (loadData->getGeneralDescription() == NULL) {
			printf("<PASS> description match\n");
		} else {
			printf("<FAIL> description match\n");
		}

		/** channel data tests */
		{
			DQEmgChannelData *loadChannel;
			short *loadChannelData;
			unsigned long loadNumSamples;
			float loadSamplingRate;
			float loadScale;
			int dataOk;

			for (i = 0; i < nChannels; i++) {
				loadChannel = loadData->getChannel(i);

				sprintf(channelDescription, "channel_%03d", i);

				if (stringMatches(loadChannel->getChannelDescription(),
									channelDescription)) {
					printf("<PASS> channel description match\n");
				} else {
					printf("<FAIL> channel description match\n");
				}

				/** get the data */
				loadChannelData = loadChannel->getDataAsShort(
								&loadNumSamples,
								&loadSamplingRate,
								&loadScale
							);


				/** check that it is ok */
				dataOk = 1;
				for (j = 0; j < N_DATA_ELEMENTS; j++) {
					if (loadChannelData[j] != i * 10)
						dataOk = 0;
				}

				if (dataOk) {
					printf("<PASS> channel description match\n");
				} else {
					printf("<FAIL> channel description match\n");
				}
			}
		}

		delete loadData;
	}

	unlink(TESTFILENAME);
	delete data;

	return 1;
}


static int
testTooManyChannels()
{
	const char *TESTFILENAME		= "tmpfile.dat";


	const char *VENDOR_ID		= "uw-test";
	const char *OPERATOR		= "Operator";
	const char *PATIENT				= "Patient";
	const char *PATIENT_ID		= "id";
	const char *MUSCLE				= "bicep";

	const DQEmgData::Gender GENDER		= DQEmgData::MALE;
	const DQEmgData::Side SIDE				= DQEmgData::RIGHT;

	const unsigned long DOB_YEAR		= 2008;
	const unsigned long DOB_MONTH		= 1;
	const unsigned long DOB_DAY				= 1;
	const unsigned long AGE				= 30;

	const unsigned long SAMPLING_RATE		= 42000;

	const float SCALE						= 0.25;

	const int N_DATA_ELEMENTS				= 128;
	const int N_CHANNELS				= 256;

	short channelData[N_DATA_ELEMENTS];


	DQEmgData *data;
	DQEmgChannelData *channel;
	char channelDescription[128];
	time_t now;
	int status;
	int i, j;


	printf("<TEST> invalie %d channels\n", N_CHANNELS);


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
				SIDE
			);

	for (i = 0; i < N_CHANNELS; i++) {
		sprintf(channelDescription, "channel_%03d", i);
		for (j = 0; j < N_DATA_ELEMENTS; j++) {
			channelData[j] = i * 10;
		}
		channel = new DQEmgChannelData(
					i, 10000 * i, 100 * i,
				channelDescription);
		channel->setData(N_DATA_ELEMENTS, channelData,
							SAMPLING_RATE, "µV", SCALE);

		status = data->addChannel(channel);
		if (i < 256) {
			if (status) {
				printf("<PASS> addChannnel status ok < 255 channels\n");
			} else {
				printf("<PASS> addChannnel status 0 < 255 channels\n");
			}
		} else {
			if (!status) {
				printf("<PASS> addChannnel status 0 < 255 channels\n");
			} else {
				printf("<PASS> addChannnel status == 0 < 255 channels\n");
			}
		}

	}


	data->store(TESTFILENAME);
	{
		DQEmgData *loadData;
		unsigned long day, month, year, age;

		loadData = new DQEmgData();
		loadData->load(TESTFILENAME);

		if (stringMatches(loadData->getVendorIdentifier(), VENDOR_ID)) {
			printf("<PASS> vendor id match\n");
		} else {
			printf("<FAIL> vendor id match\n");
		}

		if (stringMatches(loadData->getOperatorDescription(), OPERATOR)) {
			printf("<PASS> operator match\n");
		} else {
			printf("<FAIL> operator match\n");
		}

		if (stringMatches(loadData->getSubjectDescription(), PATIENT)) {
			printf("<PASS> patient match\n");
		} else {
			printf("<FAIL> patient match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID)) {
			printf("<PASS> patient ID match\n");
		} else {
			printf("<FAIL> patient ID match\n");
		}

		if (stringMatches(loadData->getSubjectID(), PATIENT_ID)) {
			printf("<PASS> patient ID match\n");
		} else {
			printf("<FAIL> patient ID match\n");
		}


		loadData->getSubjectDateOfBirth(&year, &month, &day);
		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY) {
			printf("<PASS> DOB match\n");
		} else {
			printf("<FAIL> DOB match\n");
		}

		age = loadData->getSubjectAge();
		if (age == AGE) {
			printf("<PASS> age match\n");
		} else {
			printf("<FAIL> age match\n");
		}


		loadData->getSubjectDateOfBirth(&year, &month, &day);
		if (year == DOB_YEAR && month == DOB_MONTH && day == DOB_DAY) {
			printf("<PASS> DOB match\n");
		} else {
			printf("<FAIL> DOB match\n");
		}


		if (loadData->getSubjectGender() == GENDER) {
			printf("<PASS> gender match\n");
		} else {
			printf("<FAIL> gender match\n");
		}

		if (loadData->getMuscleSide() == SIDE) {
			printf("<PASS> muscle side match\n");
		} else {
			printf("<FAIL> muscle side match\n");
		}

		if (stringMatches(loadData->getMuscleDescription(), MUSCLE)) {
			printf("<PASS> muscle description match\n");
		} else {
			printf("<FAIL> muscle description match\n");
		}

		if (loadData->getGeneralDescription() == NULL) {
			printf("<PASS> description match\n");
		} else {
			printf("<FAIL> description match\n");
		}

		/** channel data tests */
		{
			DQEmgChannelData *loadChannel;
			short *loadChannelData;
			unsigned long loadNumSamples;
			float loadSamplingRate;
			float loadScale;
			int dataOk;

			for (i = 0; i < N_CHANNELS; i++) {
				loadChannel = loadData->getChannel(i);

				if (i < 255) {

					sprintf(channelDescription, "channel_%03d", i);

					if (stringMatches(loadChannel->getChannelDescription(),
										channelDescription)) {
						printf("<PASS> channel description match\n");
					} else {
						printf("<FAIL> channel description match\n");
					}

					/** get the data */
					loadChannelData = loadChannel->getDataAsShort(
									&loadNumSamples,
									&loadSamplingRate,
									&loadScale
								);


					/** check that it is ok */
					dataOk = 1;
					for (j = 0; j < N_DATA_ELEMENTS; j++) {
						if (loadChannelData[j] != i * 10)
							dataOk = 0;
					}

					if (dataOk) {
						printf("<PASS> channel description match\n");
					} else {
						printf("<FAIL> channel description match\n");
					}
				} else {
					if (loadChannel == NULL) {
						printf("<PASS> channel null as expected\n");
					} else {
						printf("<FAIL> channel %d (> 255) not null\n", i);
					}
				}
			}
		}

		delete loadData;
	}

	unlink(TESTFILENAME);
	delete data;

	return 1;
}


int
testNumChannels()
{
	int status = 1;
	int channelTests[] = {
			0, 1, 2, 3, 4, 5, 8, 16, 32, 64, 127, 128, 255, (-1)
		};
	int i;
		
	for (i = 0; channelTests[i] >= 0; i++) {
		status = testNChannels(channelTests[i]) && status;
	}

	status = testTooManyChannels() && status;

	return status;
}

