#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"

int
testDiscretizeMMEGroupUnique_floatXORDatafile()
{
	tlErrorManager *err;
	tlBinTable *table;
	int nBins;
	int testBinId, id;
	int passedAll;
	int i;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (nBins = 2; nBins <= 2; nBins++)
	{

		passedAll = 1;
		printf("<TEST>     nBins = %d\n", nBins);

		table = new tlBinTable();
		MSG_ASSERT(table != NULL, "allocation error");
		table->ref();


		if (!tlSTable::loadTable(table, "floatXorData.txt",
			  DELIM_SET, err))
		{
			printf("<FAIL> file load -- '%s' \n", "floatXorData.txt");
			return 0;
		}
		if (!table->createBins(tlBinTable::MME_GROUP_UNIQUE,
				       nBins, err))
		{
			printf("<FAIL> createBins\n");
			return 0;
		}
		/** A */
		for (i = 0; i < 128; i++)
		{

			if (nBins == 1)
			{
				testBinId = 0;
			} else
			{
				testBinId = (i % 4) / 2;
			}

			id = table->getColumnBin(0)->getBinId(i);
			if (id == testBinId)
			{
				printf("<PASS> -- A bin id correct (%d)\n", id);
			} else
			{
				printf("%s %d (%d, but expected %d)\n",
				     "<FAIL> -- A bin id incorrect on line",
				       i,
				       id, testBinId);
				passedAll = 0;
			}
		}


		/** B */
		for (i = 0; i < 128; i++)
		{

			if (nBins == 1)
			{
				testBinId = 0;
			} else
			{
				testBinId = (i % 2);
			}

			id = table->getColumnBin(1)->getBinId(i);
			if (id == testBinId)
			{
				printf("<PASS> -- B bin id correct (%d)\n", id);
			} else
			{
				printf("%s %d (%d, but expected %d)\n",
				     "<FAIL> -- B bin id incorrect on line",
				       i,
				       id, testBinId);
				passedAll = 0;
			}
		}


		/** Output */
		for (i = 0; i < 128; i++)
		{

			testBinId = ((i + 3) % 4) / 2;

			id = table->getColumnBin(2)->getBinId(i);
			if (id == testBinId)
			{
				printf("<PASS> -- Output bin id correct (%d)\n", id);
			} else
			{
				printf("%s %d (%d, but expected %d)\n",
				"<FAIL> -- Output bin id incorrect on line",
				       i,
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

	err->unref();

	return 1;
}
