#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"
#include "testutils.h"

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

			TEST(__FILE__, __LINE__,
					"nBins = %d, repeats = %d, nData = %d\n",
			       sConfigData[ci].nBins,
			       nRepeats,
			       sConfigData[ci].nDataValues);


			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();


			if (!tlSTable::loadTable(table, FILENAME,
				  DELIM_SET, err))
			{
				FAIL(__FILE__, __LINE__,
						"file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::MME_IGNORE_UNIQUE,
					   sConfigData[ci].nBins, err))
			{
				FAIL(__FILE__, __LINE__, "createBins\n");
				return 0;
			}
			if (table->getNumRows()
			    == (sConfigData[ci].nDataValues * nRepeats))
			{
				PASS(__FILE__, __LINE__,
						"# rows correct (%d)\n",
				       sConfigData[ci].nDataValues);
			} else
			{
				FAIL(__FILE__, __LINE__,
						"%d rows in table -- expected %d\n",
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
				PASS(__FILE__, __LINE__, "# bins correct (%d)\n",
				       nExpectedBins);
			} else
			{
				FAIL(__FILE__, __LINE__,
						"%d bins in table -- expected %d\n",
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
						PASS(__FILE__, __LINE__,
								"bin id correct (%d)\n", id);
					} else
					{
						FAIL(__FILE__, __LINE__,
						"bin id incorrect %s (%d, but expected %d)\n",
						       id, testBinId);
						passedAll = 0;
					}
					k++;
				}
			}

			if (!passedAll)
			{
				DBG(__FILE__, __LINE__,
						"Dumping table due to failure in test:\n");
				table->dump(stdout);
			}
			table->unref();
			table = NULL;
		}
	}

	err->unref();

	return 1;
}

//static void
//write100x5file(int nUniqueValues, int nRepeats)
//{
//	FILE *tfp;
//	int i, j;
//
//	tfp = fopen(FILENAME, "w");
//	MSG_ASSERT(tfp != NULL, "Cannot open file");
//
//	fprintf(tfp, "X\n");
//	for (i = 0; i < nUniqueValues; i++)
//	{
//		for (j = 0; j < nRepeats; j++)
//		{
//			fprintf(tfp, "%f\n", (double) i);
//		}
//	}
//	fclose(tfp);
//}

//static int
//test_100by5()
//{
//	tlErrorManager *err;
//	tlBinTable *table;
//	int nRepeats, nUniqueValues, nTotalDataValues;
//	int nActualBins;
//	int nExpectedBins;
//	int nBins;
//	int testBinId, id;
//	int passedAll;
//	int i, j, k;
//
//
//	err = new tlErrorManager();
//	MSG_ASSERT(err != NULL, "allocation error");
//	err->ref();
//
//	nRepeats = 100;
//
//	//for (nUniqueValues = 1; nUniqueValues < 128;
//	       //nUniqueValues > 1 ? nUniqueValues *= 4 : nUniqueValues++)
//		nUniqueValues = 2;
//	{
//		/** create the data file */
//		write100x5file(nUniqueValues, nRepeats);
//		nTotalDataValues = nUniqueValues * nRepeats;
//
//		//for (nBins = 1; nBins < 32; nBins *= 2)
//			nBins = 4;
//		{
//			passedAll = 1;
//
//			TEST(__FILE__, __LINE__,
//				"nBins = %d, nRepeats = %d, nUniqueValues = %d\n",
//			    nBins,
//			    nRepeats,
//			    nUniqueValues);
//
//
//			table = new tlBinTable();
//			MSG_ASSERT(table != NULL, "allocation error");
//			table->ref();
//
//
//			if (!tlSTable::loadTable(table, FILENAME,
//				  DELIM_SET, err))
//			{
//				FAIL(__FILE__, __LINE__,
//						"file load -- '%s' \n", FILENAME);
//				return 0;
//			}
//			if (!table->createBins(tlBinTable::MME_IGNORE_UNIQUE,
//					       nBins, err))
//			{
//				FAIL(__FILE__, __LINE__, "createBins\n");
//				return 0;
//			}
//			if (table->getNumRows() == (nUniqueValues * nRepeats))
//			{
//				PASS(__FILE__, __LINE__, "# rows correct (%d)\n",
//				       nUniqueValues);
//			} else
//			{
//				FAIL(__FILE__, __LINE__,
//						"%d rows in table -- expected %d\n",
//				       table->getNumRows(),
//				       (nUniqueValues * nRepeats));
//				passedAll = 0;
//			}
//
//			nExpectedBins = nBins;
//
//
//			nActualBins = table->getColumnBin(0)->getNumBins();
//			if (nActualBins == nExpectedBins)
//			{
//				PASS(__FILE__, __LINE__,
//						"# bins correct (%d)\n", nExpectedBins);
//			} else
//			{
//				FAIL(__FILE__, __LINE__,
//						" %d bins in table -- expected %d\n",
//						nActualBins, nExpectedBins);
//				passedAll = 0;
//			}
//
//
//
//			k = 0;
//			for (i = 0; i < nUniqueValues; i++)
//			{
//				for (j = 0; j < nRepeats; j++)
//				{
//
//					if (nExpectedBins == 1)
//					{
//						testBinId = 0;
//					} else
//					{
//						testBinId = (int)
//							(((double) k / (double) nTotalDataValues)
//							 * nExpectedBins);
//					}
//
//					id = table->getColumnBin(0)->getBinId(k);
//					if (id == testBinId)
//					{
//						PASS(__FILE__, __LINE__,
//							"bin id correct (%d) - row %s\n",
//							id,
//							table->getRow(k)->getStringValue().getValue());
//					} else
//					{
//						FAIL(__FILE__, __LINE__,
//								"bin id incorrect (%d, but expected %d) - row %s\n",
//								id, testBinId,
//								table->getRow(k)->getStringValue().getValue());
//						passedAll = 0;
//					}
//					k++;
//				}
//			}
//
//			if (!passedAll)
//			{
//				DBG(__FILE__, __LINE__,
//						"Dumping table due to failure in test:\n");
//				table->dump(stdout);
//			}
//			table->unref();
//			table = NULL;
//		}
//	}
//
//	err->unref();
//
//	return 1;
//}

int
testDiscretizeMMEIgnoreUnique_floatRepeats()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> repeats -- up to 8 repeats\n");
	s = test_UpTo8Repeats();
	status = status && s;

//	printf("<SUBTESTCASE> 100 * 5 elements\n");
//	s = test_100by5();
//	status = status && s;

	return status;
}
