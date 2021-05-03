#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"

static void 
writeFloatRepeatsFile(int nValues, int nRepeats)
{
	FILE *tfp;
	int i, j;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "X\n");
	for (i = 0; i < nRepeats; i++)
	{
		for (j = 0; j < nValues; j++)
		{
			fprintf(tfp, "%f\n", (double) j);
		}
	}
	fclose(tfp);
}

static struct ConfigPair
{
	int nDataValues;
	int nBins;
} sConfigData[] =
{
	{
		25, 5
	},
	{
		20, 2
	},
	{
		10, 1
	},
	{
		250, 5
	},
	{
		200, 2
	},
	{
		100, 1
	},
	{
		1, 1
	},
	{
		1, 2
	},
	{
		1, 16
	},
	{
		(-1), (-1)
	}
};

static int
test_UpTo8Repeats()
{
	tlErrorManager *err;
	tlBinTable *table;
	int nRepeats;
	int nActualBins;
	int nExpectedBins;
	int expectedNumPerBin;
	int testBinId, id;
	int passedAll;
	int i, j, k, ci;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (ci = 0; sConfigData[ci].nDataValues > 0; ci++)
	{
		for (nRepeats = 1; nRepeats <= 8; nRepeats *= 2)
		{
			/** create the data file */
			writeFloatRepeatsFile(sConfigData[ci].nDataValues, nRepeats);


			passedAll = 1;

			printf("%s nBins = %d, repeats = %d, nData = %d\n",
			       "<TEST>     ",
			       sConfigData[ci].nBins,
			       nRepeats,
			       sConfigData[ci].nDataValues);


			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();


			if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
			{
				printf("<FAIL> file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::MME_GROUP_UNIQUE,
					       sConfigData[ci].nBins, err))
			{
				printf("<FAIL> createBins\n");
				return 0;
			}
			if (table->getNumRows()
			    == (sConfigData[ci].nDataValues * nRepeats))
			{
				printf("<PASS> -- # rows correct (%d)\n",
				       sConfigData[ci].nDataValues);
			} else
			{
				printf("<FAIL> %d rows in table -- expected %d\n",
				       table->getNumRows(),
				  (sConfigData[ci].nDataValues * nRepeats));
				passedAll = 0;
			}

			if (sConfigData[ci].nDataValues == 1)
				nExpectedBins = 1;
			else
				nExpectedBins = sConfigData[ci].nBins;


			nActualBins = table->getColumnBin(0)->getNumBins();
			if (nActualBins == nExpectedBins)
			{
				printf("<PASS> -- # bins correct (%d)\n",
				       nExpectedBins);
			} else
			{
				printf("<FAIL> %d bins in table -- expected %d\n",
				       nActualBins, nExpectedBins);
				passedAll = 0;
			}



			k = 0;
			for (i = 0; i < nRepeats; i++)
			{
				for (j = 0; j < sConfigData[ci].nDataValues; j++)
				{


					expectedNumPerBin = (sConfigData[ci].nDataValues
							   / nExpectedBins);

					if (sConfigData[ci].nDataValues == 1)
					{
						testBinId = 0;
					} else if (expectedNumPerBin == 0)
					{
						testBinId = 0;
					} else
					{
						testBinId = j / expectedNumPerBin;
					}

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

			if (!passedAll)
			{
				printf("Dumping table due to failure in test:\n");
				table->dump(stdout);
			}
			table->unref();
			table = NULL;
		}
	}

	err->unref();

	return 1;
}

static void
write100x5file(int nUniqueValues, int nRepeats)
{
	FILE *tfp;
	int i, j;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "X\n");
	for (i = 0; i < nUniqueValues; i++)
	{
		for (j = 0; j < nRepeats; j++)
		{
			fprintf(tfp, "%f\n", (double) i);
		}
	}
	fclose(tfp);
}

static int
test_100by5()
{
	tlErrorManager *err;
	tlBinTable *table;
	int nRepeats, nUniqueValues;
	int nActualBins;
	int nExpectedBins;
	int nBins;
	int testBinId, id;
	int passedAll;
	int i, j, k;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	nRepeats = 100;

	for (nUniqueValues = 1; nUniqueValues < 128;
	     nUniqueValues > 1 ? nUniqueValues *= 4 : nUniqueValues++)
	{
		/** create the data file */
		write100x5file(nUniqueValues, nRepeats);

		for (nBins = 1; nBins < 32; nBins *= 2)
		{
			passedAll = 1;

			printf("%s nBins = %d, nRepeats = %d, nUniqueValues = %d\n",
			       "<TEST>     ",
			       nBins,
			       nRepeats,
			       nUniqueValues);


			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();


			if (!tlSTable::loadTable(table, FILENAME,
				  DELIM_SET, err))
			{
				printf("<FAIL> file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::MME_GROUP_UNIQUE,
					       nBins, err))
			{
				printf("<FAIL> createBins\n");
				return 0;
			}
			if (table->getNumRows() == (nUniqueValues * nRepeats))
			{
				printf("<PASS> -- # rows correct (%d)\n",
				       nUniqueValues);
			} else
			{
				printf("<FAIL> %d rows in table -- expected %d\n",
				       table->getNumRows(),
				       (nUniqueValues * nRepeats));
				passedAll = 0;
			}

			if (nBins < nUniqueValues)
				nExpectedBins = nBins;
			else
				nExpectedBins = nUniqueValues;


			nActualBins = table->getColumnBin(0)->getNumBins();
			if (nActualBins == nExpectedBins)
			{
				printf("<PASS> -- # bins correct (%d)\n",
				       nExpectedBins);
			} else
			{
				printf("<FAIL> %d bins in table -- expected %d\n",
				       nActualBins, nExpectedBins);
				passedAll = 0;
			}



			k = 0;
			for (i = 0; i < nUniqueValues; i++)
			{
				for (j = 0; j < nRepeats; j++)
				{

					if (nExpectedBins == 1)
					{
						testBinId = 0;
					} else
					{
						if (nUniqueValues <= nExpectedBins)
						{
							testBinId = i;
						} else
						{
							testBinId = (int)
								(((double) i / (double) nUniqueValues)
							   * nExpectedBins);
						}
					}

					id = table->getColumnBin(0)->getBinId(k);
					if (id == testBinId)
					{
						printf("<PASS> -- bin id correct (%d)\n", id);
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

			if (!passedAll)
			{
				printf("Dumping table due to failure in test:\n");
				table->dump(stdout);
			}
			table->unref();
			table = NULL;
		}
	}

	err->unref();

	return 1;
}

int
testDiscretizeMMEGroupUnique_floatRepeats()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> repeats -- up to 8 repeats\n");
	s = test_UpTo8Repeats();
	status = status && s;

	printf("<SUBTESTCASE> 100 * 5 elements\n");
	s = test_100by5();
	status = status && s;

	return status;
}
