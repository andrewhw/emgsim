#include <stdio.h>
#include <stdlib.h>

#include "arrayAllocator.h"
#include "tclCkalloc.h"
#include "testutils.h"

static void
printArray_TYPE(TYPE **a, int nRows, int nCols)
{
	int i, j;

	for (i = 0; i < nRows; i++)
	{
		DBG(__FILE__, __LINE__, "%4d :", i);
		for (j = 0; j < nCols; j++)
		{
			printf(" %5g", (float) a[i][j]);
		}
		printf("\n");
	}
}


static int
runTestArray_TYPE(int nCols, int nRows)
{
	TYPE **a, v;
	int i, j;
	int status = 1;


	a = (TYPE **) alloc2darray(nRows, nCols, sizeof(TYPE));

	for (i = 0; i < nRows; i++)
	{
		for (j = 0; j < nCols; j++)
		{
			a[i][j] = (i+1) + ((j + 1) / 10.0);
		}
	}

	printArray_TYPE(a, nRows,nCols);

	for (j = 0; j < nCols; j++)
	{
		for (i = 0; i < nRows; i++)
		{
			v = (i+1) + ((j + 1) / 10.0);
			if (a[i][j] != v)
			{
				status = 0;
				FAIL(__FILE__, __LINE__,
						"Mismatch at (%d, %d) : got %g, expected %g)\n",
						i, j,
						(float) a[i][j],
						(float) v);
			}
			else
			{
				PASS(__FILE__, __LINE__,
						"Found expected %g at (%d, %d))\n",
						(float) a[i][j], i, j);
			}
		}
	}

	ckfree(a);

	return status;
}

static int
runTestBlock_TYPE(int x, int y)
{
	int i, j, k;
	TYPE **a;
	int status = 1;

	a = (TYPE **) alloc2darray(y, x, sizeof(TYPE));

	for (i = 0; i < x * y; i++)
	{
		a[0][i] = (TYPE) i;
	}

	printArray_TYPE(a, y,x);


	k = 0;
	for (i = 0; i < y; i++)
	{
		for (j = 0; j < x; j++)
		{
			if (a[i][j] != k)
			{
				status = 0;
				FAIL(__FILE__, __LINE__,
						"Mismatch at (%d, %d) : got %g, expected %d)\n",
						i, j,
						(float) a[i][j],
						k);
			}
			else
			{
				PASS(__FILE__, __LINE__,
						"Found expected %g at (%d, %d))\n",
						(float) a[i][j], i, j);
			}
			k++;
		}
	}

	ckfree(a);

	return status;
}

static int
runTest_TYPE(int nCols, int nRows)
{
	int status;

	TEST(__FILE__, __LINE__,
			"2d: allocation of %d x %d\n",
			nCols, nRows);

	status = runTestArray_TYPE(nCols, nRows);
	status = runTestBlock_TYPE(nCols, nRows) && status;

	return status;
}

struct testpair {
	int nCols;
	int nRows;
};

static struct testpair sTests[] = {
		{ 4, 1 },
		{ 2, 3 },
		{ 9, 9 },
		{ 1, 12 },
		{ 12, 1 },
		{ -1, -1 }
	};
int
test_2d_TYPE(argc, argv)
	int argc;
	char **argv;
{
	int i;
	int status = 1;

	for (i = 0; sTests[i].nCols >= 0; i++)
	{
		status = runTest_TYPE(
					sTests[i].nCols, sTests[i].nRows
				) && status;
	}

	return(status);
}

