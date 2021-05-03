#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"


struct XorFloat
{
	double x;
	double y;
	const char *label;
};

static struct XorFloat xorFloatValues[] = {
	{-0.5, -0.5, "LOW"},
	{-0.5, 0.5, "HIGH"},
	{0.5, -0.5, "HIGH"},
	{0.5, 0.5, "LOW"},
	{0.0, 0.0, NULL}
};

static void 
sWriteXorDataFile(int nRepeats)
{
	FILE *tfp;
	int i, j;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "%6s, %6s,  %s\n", "X", "Y", "Output");
	for (i = 0; i < nRepeats; i++)
	{
		for (j = 0; xorFloatValues[j].label != NULL; j++)
		{
			fprintf(tfp, "%6g, %6g, \"%s\"\n",
				xorFloatValues[j].x,
				xorFloatValues[j].y,
				xorFloatValues[j].label);
		}
	}

	fclose(tfp);
}

int
testDiscretizeMMEGroupUnique_XOR()
{
	static int nRepeats[] = {8, 32, (-1)};
	static int nBinList[] = {1, 2, 4, (-1)};

	tlErrorManager *err;
	tlBinTable *table;
	tlBin::tlBinId id;
	int expectId;
	int modValue;
	int colId;
	int passedAll;
	int i, j, k;


	printf("<SUBTESTCASE> XOR-FloatSeries(MME)\n");

	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (i = 0; nRepeats[i] > 0; i++)
	{

		/** create the data file */
		sWriteXorDataFile(nRepeats[i]);

		for (j = 0; nBinList[j] > 0; j++)
		{

			passedAll = 1;

			printf("<TEST> %d repeats of training values, %d bins\n",
			       nRepeats[i], nBinList[j]);
			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();


			if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
			{
				printf("<FAIL> file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::MME_GROUP_UNIQUE,
					       nBinList[j], err))
			{
				printf("<FAIL> createBins\n");
				return 0;
			}
			/** try out "x" column -- value changes every two entries */
			colId = 0;
			for (k = 0; k < table->getNumRows(); k++)
			{
				id = table->getColumnBin(colId)->getBinId(k);
				if (nBinList[j] == 1)
				{
					expectId = 0;
				} else
				{
					modValue = k % 4;
					if (modValue < 2)
					{
						expectId = 0;
					} else
					{
						expectId = 1;
					}
				}
				if ((int) id != expectId)
				{
					printf("<FAIL> \"X\" index %d, value %g: id %d, want %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id, expectId);
					passedAll = 0;
				} else
				{
					printf("<PASS> \"X\" index %d, value %g: id %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id);
				}
			}

			/** try out "y" column */
			colId = 1;
			for (k = 0; k < table->getNumRows(); k++)
			{
				id = table->getColumnBin(colId)->getBinId(k);
				if (nBinList[j] == 1)
				{
					expectId = 0;
				} else
				{
					expectId = (int) (k % 2);
				}
				if ((int) id != expectId)
				{
					printf("<FAIL> \"Y\" index %d, value %g: id %d, want %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id, expectId);
					passedAll = 0;
				} else
				{
					printf("<PASS> \"Y\" index %d, value %g: id %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id);
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
