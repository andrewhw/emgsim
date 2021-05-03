#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>
#include "tclCkalloc.h"


static int
test1()
{
	tlMatrix *A, *B, *C, *D;
	int i, j, k, l;


	for (i = 1; i <= 8; i *= 2)
	{
		for (j = 1; j <= 4; j *= 2)
		{

			A = new tlMatrix(i, j);
			A->ref();
			B = new tlMatrix(i, j);
			B->ref();

			for (k = 0; k < i; k++)
			{
				for (l = 0; l < j; l++)
				{
					(*A)[k][l] = ((double) k * (double) l) / (double) i;
					(*B)[k][l] = k * 100 + l;
				}
			}

			for (k = 2; k <= 8; k *= 2)
			{
				C = A->sum(B);
				C->ref();

				D = C->difference(B);
				D->ref();

				if (D->equals(A))
				{
					PASS(__FILE__, __LINE__,
					     "Mult/divide ok i=%d, j=%d, k=%d\n", i, j, k);
				} else
				{
					FAIL(__FILE__, __LINE__, "Mult/divide failed:\n");
					FAIL(__FILE__, __LINE__, " got '%s'\n",
					     C->getValueString().getValue());
					FAIL(__FILE__, __LINE__, "want '%s'\n",
					     A->getValueString().getValue());
				}
				C->unref();
				D->unref();
			}

			A->unref();
			B->unref();
		}
	}

	return 1;
}

int
testMatrixSum()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> example 1\n");
	s = test1();
	status = status && s;

	return status;
}
