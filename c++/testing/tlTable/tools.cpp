#include <stdio.h>

#include <tlTable.h>
#include <tlBinTable.h>
#include <tlSTable.h>
#include <tlTuple.h>

#include <tlErrorManager.h>

#include "tools.h"


static void 
writeDataFile(const char *lineList[])
{
	FILE *tfp;
	int i;

	tfp = fopen(FILENAME, "w");
	MSG_ASSERT(tfp != NULL, "Cannot open file");

	for (i = 0; lineList[i] != NULL; i++)
	{
		fputs(lineList[i], tfp);
		fputc('\n', tfp);
	}
	fclose(tfp);
}



static const char *threeValueData[] = {
	"IntZero,IntOne,IntTwo,StrZero,StrOne,StrTwo,FltZero,FltOne,FltTwo",
	"0,0,0,ZERO,ZERO,ZERO,0.0,0.0,0.0",
	"0,1,1,ZERO,ONE,ONE,0.0,1.0,1.0",
	"0,0,2,ZERO,ZERO,TWO,0.0,0.0,2.0",
	"0,1,0,ZERO,ONE,ZERO,0.0,1.0,0.0",
	"0,0,1,ZERO,ZERO,ONE,0.0,0.0,1.0",
	"0,1,2,ZERO,ONE,TWO,0.0,1.0,2.0",
	NULL
};

int
testDiscretize1to16(tlBinTable::BinStrategy strategy)
{
		tlErrorManager *err;
		tlBinTable *table;
		int nBins;


	if (strategy == tlBinTable::MME_GROUP_UNIQUE)
	{
			printf("<SUBTESTCASE> 1-16(MME - group unique)\n");
	} else if (strategy == tlBinTable::MME_IGNORE_UNIQUE)
	{
			printf("<SUBTESTCASE> 1-16(MME - ignore unique)\n");
	} else if (strategy == tlBinTable::EQUAL_BIN_RANGE)
	{
			printf("<SUBTESTCASE> 1-16(equal bin range)\n");
	} else
	{
			printf("<SUBTESTCASE> 1-16(unknown)\n");
	}

	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();


	/** create the data file */
	writeDataFile(threeValueData);


	for (nBins = 1; nBins <= 16; nBins = nBins * 2)
	{
		printf("<TEST> nBins = %d\n", nBins);


		table = new tlBinTable();
		MSG_ASSERT(table != NULL, "allocation error");
		table->ref();

		if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
		{
			printf("<FAIL> file load -- '%s' \n", FILENAME);
			return 0;
		}
		if (!table->createBins(strategy, nBins, err))
		{
			printf("<FAIL> createBins\n");
			return 0;
		}
		//table->dump(stdout);

		table->unref();
		table = NULL;
	}

	err->unref();

	return 1;
}

static const char *floatSeries[] = {
	"x,y",
	"0.0, 0.9",
	"0.1, 0.8",
	"0.2, 0.7",
	"0.3, 0.6",
	"0.4, 0.5",
	"0.5, 0.4",
	"0.6, 0.3",
	"0.7, 0.2",
	"0.8, 0.1",
	"0.9, 0.0",
	NULL
};

int
testDiscretizeFloatSeries(tlBinTable::BinStrategy strategy)
{
	tlErrorManager *err;
	tlBinTable *table;
	int nBins = 2;
tlBin::tlBinId id;
	int i, expectId;


	printf("<SUBTESTCASE> FloatSeries(%s)\n",
strategy == tlBinTable::MME_GROUP_UNIQUE ? "MME" : "EQUAL");

	err = new tlErrorManager();
	MSG_ASSERT(err != NULL, "allocation error");
	err->ref();


	/** create the data file */
	writeDataFile(floatSeries);

	printf("<TEST> nBins = %d\n", nBins);


	table = new tlBinTable();
	MSG_ASSERT(table != NULL, "allocation error");
	table->ref();


	if (!tlSTable::loadTable(table, FILENAME, DELIM_SET, err))
	{
		printf("<FAIL> file load -- '%s' \n", FILENAME);
		return 0;
	}
	if (!table->createBins(strategy, nBins, err))
	{
		printf("<FAIL> createBins\n");
		return 0;
	}
	//table->dump(stdout);

	for (i = 0; i < table->getNumRows(); i++)
	{

		/** get ascending ID */
		id = table->getColumnBin(0)->getBinId(i);
		expectId = (int) ((i * 2) / table->getNumRows());
		if ((int) id != expectId)
		{
			printf("<FAIL> %s id at %d was %d for value %g, expected %d\n",
			       "ascending",
			       i, id,
			       table->getRow(i)->getValue(0).getRealValue(),
			       expectId);
		} else
		{
			printf("<PASS> %s id at %d was %d for value %g\n",
			       "ascending",
			       i, id,
			       table->getRow(i)->getValue(0).getRealValue());
		}

		/** get descending ID */
		id = table->getColumnBin(1)->getBinId(i);
		expectId = (int)
			(((table->getNumRows() - (i + 1)) * 2) / table->getNumRows());
		if ((int) id != expectId)
		{
			printf("<FAIL> %s id at %d was %d for value %g, expected %d\n",
			       "descending",
			       i, id,
			       table->getRow(i)->getValue(1).getRealValue(),
			       expectId);
		} else
		{
			printf("<PASS> %s id at %d was %d for value %g\n",
			       "descending",
			       i, id,
			       table->getRow(i)->getValue(1).getRealValue());
		}
	}

	table->unref();
	table = NULL;

	err->unref();

	return 1;
}


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
testDiscretizeXorFloatSeries(tlBinTable::BinStrategy strategy)
{
	static int trainingValueList[] = {4, 8, 16, 32, 64, 128, (-1)};
	static int nBinList[] = {1, 2, 4, (-1)};

	tlErrorManager *err;
	tlBinTable *table;
	tlBin::tlBinId id;
	int expectId;
	int modValue;
	int colId;
	int i, j, k;


	if (strategy == tlBinTable::MME_GROUP_UNIQUE)
	{
		printf("<SUBTESTCASE> XOR-FloatSeries(MME - group unique)\n");
	} else if (strategy == tlBinTable::MME_IGNORE_UNIQUE)
	{
		printf("<SUBTESTCASE> XOR-FloatSeries(MME - ignore unique)\n");
	} else if (strategy == tlBinTable::EQUAL_BIN_RANGE)
	{
		printf("<SUBTESTCASE> XOR-FloatSeries(equal bin range)\n");
	} else
	{
		printf("<SUBTESTCASE> XOR-FloatSeries(unknown)\n");
	}

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
			if (!table->createBins(strategy, nBinList[j], err))
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
					expectId = (int) (k % 2);
				}
				if ((int) id != expectId)
				{
					printf("<FAIL> \"X\" index %d, value %g: id %d, want %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id, expectId);
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
					printf("<FAIL> \"Y\" index %d, value %g: id %d, want %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id, expectId);
				} else
				{
					printf("<PASS> \"Y\" index %d, value %g: id %d\n",
					       k,
					       table->getRow(k)->getValue(colId).getRealValue(),
					       id);
				}
			}

			table->unref();
			table = NULL;
		}
	}

	err->unref();

	return 1;
}


/**
 * $Id: tools.cpp 15 2008-04-26 13:34:27Z andrew $
 */
