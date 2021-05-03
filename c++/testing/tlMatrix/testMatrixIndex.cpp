#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>


const int ROWS = 3;
const int COLS = 4;

int
testMatrixIndex()
{
	tlMatrix *m;
	tlSrString t;
	double v;
	int i, j;

	m = new tlMatrix(ROWS, COLS);
	m->ref();
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			v = ((float) j / 10.0) + (float) i;
			(*m)[i][j] = v;
		}
	}

	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			v = ((float) j / 10.0) + (float) i;
			if ((*m)[i][j] == v)
			{
				printf("<PASS> -- Compare ok\n");
			} else
			{
				printf("<FAIL> -- got '%f', expected '%f' at index [%d][%d]\n",
				       (double)((*m)[i][j]),
					   (double)((i * 10) + j),
				       i, j);
			}
		}
	}

	t = m->getValueString();
	DBG(__FILE__, __LINE__, "t = %s\n", t.getValue());

	m->unref();

	return 1;
}
