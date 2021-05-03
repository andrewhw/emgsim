#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"

static void 
writeStringFile(int nValues, int nRepeats)
{
	FILE *tfp;
	int i, j;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "Values\n");
	for (i = 0; i < nRepeats; i++)
	{
		for (j = 0; j < nValues; j++)
		{
			fprintf(tfp, "str_%03d\n", j);
		}
	}

	fclose(tfp);
}

int
testStringsWithStrategy(tlBinTable::BinStrategy strategy)
{
	tlErrorManager *err;
	tlBinTable *table;
	tlSrString lastValue, loValue, hiValue;
	int nBins, nDataValues, nRepeats;
	int nActualBins;
	int testBinId, id;
	int passedAll;
	int i, j, k;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (nDataValues = 2; nDataValues <= 128; nDataValues *= 8)
	{
		for (nRepeats = 1; nRepeats <= 16; nRepeats *= 2)
		{
			/** create the data file */
			writeStringFile(nDataValues, nRepeats);

			nBins = 1;
			while (nBins <= nDataValues * 4)
			{

				passedAll = 1;

				printf("%s nBins = %d, repeats = %d, nData = %d\n",
				"<TEST>     ", nBins, nRepeats, nDataValues);


				table = new tlBinTable();
				MSG_ASSERT(table != NULL, "allocation error");
				table->ref();

				if (!tlSTable::loadTable(table, FILENAME,
					  DELIM_SET, err))
				{
					printf("<FAIL> file load -- '%s' \n", FILENAME);
					return 0;
				}
				if (!table->createBins(strategy, nBins, err))
				{
					printf("<FAIL> createBins\n");
					return 0;
				}
				if (table->getNumRows() == (nDataValues * nRepeats))
				{
					printf("<PASS> -- # rows correct (%d)\n",
					       nDataValues);
				} else
				{
					printf("<FAIL> %d rows in table -- expected %d\n",
					       table->getNumRows(), (nDataValues * nRepeats));
					passedAll = 0;
				}

				nActualBins = table->getColumnBin(0)->getNumBins();
				if (nActualBins == nDataValues)
				{
					printf("<PASS> -- # bins correct (%d)\n",
					       nDataValues);
				} else
				{
					printf("<FAIL> %d bins in table -- expected %d\n",
					       nActualBins, nDataValues);
					passedAll = 0;
				}



				k = 0;
				for (i = 0; i < nRepeats; i++)
				{
					for (j = 0; j < nDataValues; j++)
					{
						testBinId = j;

						id = table->getColumnBin(0)->getBinId(k);
						if (id == testBinId)
						{
							printf("<PASS> -- bin id correct (%d)\n",
							       id);
						} else
						{
							printf("%s (%d, but expected %d)\n",
							       "<FAIL> -- bin id incorrect",
							     id, testBinId);
							passedAll = 0;
						}
						k++;
					}
				}


				loValue = *table->getColumnBin(0)->getLoBoundValue(0)->str_;
				hiValue = *table->getColumnBin(0)->getHiBoundValue(0)->str_;
				if (loValue != hiValue)
				{
					printf("<FAIL> -- bin[0] bounds lo (%s) != hi(%s)\n",
					       loValue.getValue(), hiValue.getValue());
					passedAll = 0;
				}
				lastValue = loValue;

				for (i = 1; i < table->getColumnBin(0)->getNumBins(); i++)
				{

					loValue = *table->getColumnBin(0)->getLoBoundValue(i)->str_;
					hiValue = *table->getColumnBin(0)->getHiBoundValue(i)->str_;
					if (loValue != hiValue)
					{
						printf("<FAIL> -- bin[%d] bounds lo (%s) != hi(%s)\n",
						       i, loValue.getValue(), hiValue.getValue());
						passedAll = 0;
					}
					if (lastValue == loValue)
					{
						printf("<FAIL> -- bin[%d] '%s' matches last value '%s'!\n",
						       i, loValue.getValue(), lastValue.getValue());
						passedAll = 0;
					}
					lastValue = loValue;
				}


				if (!passedAll)
				{
					printf("Dumping table due to failure in test:\n");
					table->dump(stdout);
				}
				table->unref();
				table = NULL;

				if (nBins == 1)
				{
					nBins = 2;
				} else
				{
					nBins = nBins * nBins;
				}
			}
		}
	}

	err->unref();

	return 1;
}

int
testDiscretizeStrings()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> String test with \"Equal Range\"\n");
	s = testStringsWithStrategy(tlBinTable::EQUAL_BIN_RANGE);
	status = status && s;

	printf("<SUBTESTCASE> String test with \"MME - group unique\"\n");
	s = testStringsWithStrategy(tlBinTable::MME_GROUP_UNIQUE);
	status = status && s;

	printf("<SUBTESTCASE> String test with \"MME - ignore unique\"\n");
	s = testStringsWithStrategy(tlBinTable::MME_IGNORE_UNIQUE);
	status = status && s;

	return status;
}
