#include <stdio.h>
#include <stdlib.h>

/* include for fabs() */
#include <math.h>

#include "arrayAllocator.h"
#include "tclCkalloc.h"
#include "testutils.h"


/* define our comparison epsilon as 1e-3 = 0.001 */
#define		EPSILON		(1e-3)


static void
printArray3d_TYPE(TYPE ***a, int nDepths, int nRows, int nCols)
{
	int i, j, k;

	DBG(__FILE__, __LINE__,
			"Array of %d x %d x %d:\n", nDepths, nRows, nCols);

	for (i = 0; i < nDepths; i++)
	{
		DBG(__FILE__, __LINE__, "Level %d:\n", i);
		for (j = 0; j < nRows; j++)
		{
			DBG(__FILE__, __LINE__, "%4d %4d :", i, j);
			for (k = 0; k < nCols; k++)
			{
				printf(" %5g", (float) a[i][j][k]);
			}
			printf("\n");
		}
		DBG(__FILE__, __LINE__, "\n");
	}
}


static int
runTestArray3d_TYPE(int nDepths, int nRows, int nCols)
{
	TYPE ***a, v;
	int i, j, k;
	int status = 1;

	a = (TYPE ***) alloc3darray(nDepths, nRows, nCols, sizeof(TYPE));

	for (i = 0; i < nDepths; i++)
	{
		for (j = 0; j < nRows; j++)
		{
			for (k = 0; k < nCols; k++)
			{
				a[i][j][k] = ((i+1) * 10.0) + (j + 1.0) + ((k + 1) / 10.0);
			}
		}
	}

	printArray3d_TYPE(a, nDepths,nRows,nCols);

	for (k = 0; k < nCols; k++)
	{
		for (j = 0; j < nRows; j++)
		{
			for (i = 0; i < nDepths; i++)
			{
				v = ((i+1) * 10.0) + (j + 1.0) + ((k + 1) / 10.0);
				if (a[i][j][k] != v)
				{
					status = 0;
					FAIL(__FILE__, __LINE__,
							"Mismatch at (%d,%d,%d) : got %g, expected %g)\n",
							i, j, k,
							(float) a[i][j][k],
							(float) v);
				}
				else
				{
					PASS(__FILE__, __LINE__,
							"Found expected %g at (%d, %d, %d))\n",
							(float) a[i][j][k], i, j, k);
				}
			}
		}
	}

	ckfree(a);

	return status;
}

static int
runTestBlock3d_TYPE(int x, int y, int z)
{
	int i, j, k, ll;
	TYPE ***a;
	int status = 1;

	a = (TYPE ***) alloc3darray(z, y, x, sizeof(TYPE));

	for (i = 0; i < x * y * z; i++)
	{
		a[0][0][i] = (TYPE) i;
	}

	printArray3d_TYPE(a, z,y,x);

	ll = 0;
	for (i = 0; i < z; i++)
	{
		for (j = 0; j < y; j++)
		{
			for (k = 0; k < x; k++)
			{
				if (fabs(a[i][j][k] - ll) > EPSILON)
				{
					status = 0;
					FAIL(__FILE__, __LINE__,
							"Mismatch at (%d,%d,%d) : got %g, expected %d)\n",
							i, j, k,
							(double) a[i][j][k],
							ll);
				}
				else
				{
					PASS(__FILE__, __LINE__,
							"Found expected %g at (%d, %d, %d))\n",
							(float) a[i][j][k], i, j, k);
				}
				ll++;
			}
		}
	}

	ckfree(a);

	return status;
}

static int
runTest3d_TYPE(int nDepths, int nRows, int nCols)
{
	int status;

	TEST(__FILE__, __LINE__,
			"3d: allocation of %d x %d x %d\n",
			nDepths, nCols, nRows);

	status = runTestArray3d_TYPE(nDepths,nRows,nCols);
	status = runTestBlock3d_TYPE(nCols,nRows,nDepths);

	return status;
}

struct testindex {
	int nCols;
	int nRows;
	int nDepths;
};

static struct testindex sTests[] = {
		{ 2, 3, 4 },
		{ 3, 4, 2 },
		{ 4, 2, 3 },
		{ 5, 5, 5 },
		{ 1, 9, 1 },
		{ 9, 1, 1 },
		{ 1, 1, 9 },
		{ -1, -1, -1 }
	};

int
test_3d_TYPE(argc, argv)
	int argc;
	char **argv;
{
	int i;
	int status = 1;

	for (i = 0; sTests[i].nCols >= 0; i++)
	{
		status =
				runTest3d_TYPE(
					sTests[i].nDepths,
					sTests[i].nRows,
					sTests[i].nCols
				) && status;
	}

	return(status);
}

