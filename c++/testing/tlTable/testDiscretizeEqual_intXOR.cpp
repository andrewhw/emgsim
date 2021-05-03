#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"

static void 
writeIntegerXORFile()
{
	FILE *tfp;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "Input_A, Input_B, Output\n");
	fprintf(tfp, "%10d, %10d, %10s\n", 0, 0, "LOW");
	fprintf(tfp, "%10d, %10d, %10s\n", 0, 1, "HIGH");
	fprintf(tfp, "%10d, %10d, %10s\n", 1, 0, "HIGH");
	fprintf(tfp, "%10d, %10d, %10s\n", 1, 1, "LOW");

	fclose(tfp);
}

int
testDiscretizeEqual_intXOR()
{
	tlErrorManager *err;
	tlBinTable *table;
	int nRepeats, nBins;
	int nActualBins;
	int nExpectedBins;
	int nDataValues = 4;
	int testBinId, id;
	int passedAll;
	int i, j, k;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (nRepeats = 1; nRepeats <= 100; nRepeats *= 10)
	{
		for (nBins = 1; nBins <= 4; nBins *= 2)
		{
			/** create the data file */
			writeIntegerXORFile();

			passedAll = 1;
			printf("%s nBins = %d, repeats = %d\n",
			       "<TEST>     ",
			       nBins,
			       nRepeats);

			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();


			for (i = 0; i < nRepeats; i++)
			{
			if (!tlSTable::loadTable(table, FILENAME,
					  DELIM_SET, err))
				{
					printf("<FAIL> file load -- '%s' \n", FILENAME);
					return 0;
				}
			}

			if (!table->createBins(tlBinTable::EQUAL_BIN_RANGE,
					       nBins, err))
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
				       table->getNumRows(),
				       (nDataValues * nRepeats));
				passedAll = 0;
			}


			for (i = 0; i < table->getNumColumns(); i++)
			{

				if (table->getColumn(i)->getType() != tlSrValue::STRING)
				{
					if (nBins < 2)
						nExpectedBins = nBins;
					else
						nExpectedBins = 2;
				} else
				{
					nExpectedBins = 2;
				}

				nActualBins = table->getColumnBin(i)->getNumBins();
				if (nActualBins == nExpectedBins)
				{
					printf("<PASS> -- column %d # bins correct (%d)\n",
					       i, nExpectedBins);
				} else
				{
					printf(
					       "<FAIL> %d bins in column %d -- expected %d\n",
					     nActualBins, i, nExpectedBins);
					passedAll = 0;
				}
			}


			/** A */
			k = 0;
			for (i = 0; i < nRepeats; i++)
			{
				for (j = 0; j < nDataValues; j++)
				{

					if (nBins == 1)
					{
						testBinId = 0;
					} else
					{
						testBinId = (j % 4) / 2;
					}

					id = table->getColumnBin(0)->getBinId(k);
					if (id == testBinId)
					{
						printf("<PASS> -- A bin id correct (%d)\n", id);
					} else
					{
						printf("%s (%d, but expected %d)\n",
						       "<FAIL> -- A bin id incorrect",
						       id, testBinId);
						passedAll = 0;
					}
					k++;
				}
			}


			/** B */
			k = 0;
			for (i = 0; i < nRepeats; i++)
			{
				for (j = 0; j < nDataValues; j++)
				{

					if (nBins == 1)
					{
						testBinId = 0;
					} else
					{
						testBinId = (j % 2);
					}

					id = table->getColumnBin(1)->getBinId(k);
					if (id == testBinId)
					{
						printf("<PASS> -- B bin id correct (%d)\n", id);
					} else
					{
						printf("%s (%d, but expected %d)\n",
						       "<FAIL> -- B bin id incorrect",
						       id, testBinId);
						passedAll = 0;
					}
					k++;
				}
			}


			/** Output */
			k = 0;
			for (i = 0; i < nRepeats; i++)
			{
				for (j = 0; j < nDataValues; j++)
				{

					testBinId = ((j + 3) % 4) / 2;

					id = table->getColumnBin(2)->getBinId(k);
					if (id == testBinId)
					{
						printf("<PASS> -- Output bin id correct (%d)\n", id);
					} else
					{
						printf("%s (%d, but expected %d)\n",
						       "<FAIL> -- Output bin id incorrect",
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
