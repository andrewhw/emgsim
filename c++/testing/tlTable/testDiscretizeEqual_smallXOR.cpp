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
writeXorDataFile(int nDataValues)
{
	FILE *tfp;
	int i;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	fprintf(tfp, "%6s, %6s,  %s\n", "X", "Y", "Output");
	while (nDataValues > 0)
	{
		for (i = 0; xorFloatValues[i].label != NULL; i++)
		{
			fprintf(tfp, "%6g, %6g, \"%s\"\n",
				xorFloatValues[i].x,
				xorFloatValues[i].y,
				xorFloatValues[i].label);
			if (--nDataValues <= 0)
			{
				break;
			}
		}
	}

	fclose(tfp);
}

int
testDiscretizeEqual_smallXOR()
{
	static int trainingValueList[] = {12, (-1)};
	static int nBinList[] = {2, (-1)};

	tlErrorManager *err;
	tlBinTable *table;
	tlBin::tlBinId id;
	int expectId;
	int modValue;
	int colId;
	int i, j, k;


	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();

	for (i = 0; trainingValueList[i] > 0; i++)
	{

		/** create the data file */
		writeXorDataFile(trainingValueList[i]);

		for (j = 0; nBinList[j] > 0; j++)
		{

			printf("<TEST> %d training values, %d bins\n",
			       trainingValueList[i], nBinList[j]);
			table = new tlBinTable();
			MSG_ASSERT(table != NULL, "allocation error");
			table->ref();


			if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
			{
				printf("<FAIL> file load -- '%s' \n", FILENAME);
				return 0;
			}
			if (!table->createBins(tlBinTable::EQUAL_BIN_RANGE,
					       nBinList[j], err))
			{
				printf("<FAIL> createBins\n");
				return 0;
			}
			//table->dump(stdout);

			/** try out "x" column */
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
						printf("%s %s id at %d was %d for value %g, expect %d\n",
						       "<FAIL>",
						       "\"X\"",
						       k, id,
						       table->getRow(k)->getValue(colId).getRealValue(),
						       expectId);
					} else
					{
						printf("<PASS>\n");
					}
				}

				if ((int) id != expectId)
				{
					printf("%s %s id at %d was %d for value %g, expect %d\n",
					       "<FAIL>",
					       "\"Y\"",
					       k, id,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       expectId);
				} else
				{
					printf("<PASS>\n");
				}
			}

			/** try out "y" column */
			colId = 2;
			for (k = 0; k < table->getNumRows(); k++)
			{
				id = table->getColumnBin(colId)->getBinId(k);

				if (nBinList[j] == 1)
				{
					expectId = 0;
				} else
				{
					modValue = k % 4;
					if (modValue == 0 || modValue == 3)
					{
						expectId = 1;
					} else
					{
						expectId = 0;
					}
				}

				if ((int) id != expectId)
				{
					printf("%s %s id at %d was %d for value %s, expect %d\n",
					       "<FAIL>",
					       "\"Label\"",
					       k, id,
					       table->getRow(k)->getValue(
								       colId
					      ).getStringValue().getValue(),
					       expectId);
				} else
				{
					printf("<PASS>\n");
				}
			}


			table->unref();
			table = NULL;
		}
	}

	err->unref();

	return 1;
}
