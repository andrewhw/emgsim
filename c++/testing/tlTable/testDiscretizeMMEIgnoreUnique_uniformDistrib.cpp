#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"

static void 
writeUniformFile(int nValues)
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
		fprintf(tfp, "%f\n", value);
	}
	fclose(tfp);
}

int
testDiscretizeMMEIgnoreUnique_uniformDistrib()
{
	tlErrorManager *err;
	tlBinTable *table;
	double fraction;
	int nBins, nDataValues;
	int testBinId, id;
	int passedAll;
	int i;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (nDataValues = 32; nDataValues <= 128; nDataValues *= 3)
	{
		/** create the data file */
		writeUniformFile(nDataValues);


		for (nBins = 1; nBins <= (nDataValues / 5); nBins = nBins * 4)
		{
			passedAll = 1;

			printf("<TEST>     nBins = %d, nDataValues = %d\n",
			       nBins, nDataValues);


			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();

			if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
			{
				printf("<FAIL> file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::MME_IGNORE_UNIQUE,
					       nBins, err))
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
