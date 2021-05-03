#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include <mathtools.h>

#include "tools.h"

/**
 * write out sqrt(x)
 */
static void 
writeExponentialFile(int nValues)
{
	FILE *tfp;
	double value;
	int i;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "X\n");
	for (i = 0; i < nValues; i++)
	{
		value = (double) i / (double) (nValues - 1);
		fprintf(tfp, "%f\n", sqrt(value));
	}
	fclose(tfp);
}

int
testDiscretizeEqual_expDistrib()
{
	tlErrorManager *err;
	tlBinTable *table;
	double fraction;
	int nBins, nDataValues;
	int testBinId, id;
	int max;
	int passedAll;
	int i;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (nDataValues = 2; nDataValues <= 128; nDataValues *= 2)
	{
		passedAll = 1;

		/** create the data file */
		writeExponentialFile(nDataValues);


		max = nDataValues < 16 ? nDataValues : 16;
		for (nBins = 1; nBins <= max; nBins = nBins * 2)
		{
			printf("<TEST>     nBins = %d\n", nBins);


			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();

			if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
			{
				printf("<FAIL> file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::EQUAL_BIN_RANGE, nBins, err))
			{
				printf("<FAIL> createBins\n");
				return 0;
			}
			if (table->getNumRows() == nDataValues)
			{
				printf("<PASS> -- # rows correct (%d)\n", nDataValues);
			} else
			{
				printf("<FAIL> %d rows in table -- expected %d\n",
				       table->getNumRows(), nDataValues);
				passedAll = 0;
			}


			for (i = 0; i < nDataValues; i++)
			{
				if (i == nDataValues - 1)
				{
					testBinId = nBins - 1;
				} else
				{
					fraction = (double) i / (double) (nDataValues - 1);
					fraction = sqrt(fraction);
					testBinId = (int) (fraction * (double) nBins);
				}

				id = table->getColumnBin(0)->getBinId(i);
				if (id == testBinId)
				{
					printf("<PASS> -- bin id correct (%d)\n", id);
				} else
				{
					printf("<FAIL> -- bin id incorrect (%d, but expected %d)\n",
					       id, testBinId);
					passedAll = 0;
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
