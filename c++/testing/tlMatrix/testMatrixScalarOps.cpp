#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>


static int
test1()
{
	tlMatrix *A, *B, *C;
	int i, j, k, l;


	for (i = 1; i < 8; i *= 2)
	{
		for (j = 1; j < 8; j *= 2)
		{

			A = new tlMatrix(i, j);
			A->ref();

			for (k = 0; k < i; k++)
			{
				for (l = 0; l < j; l++)
				{
					(*A)[k][l] = ((double) k * (double) l) / (double) i;
				}
			}

			DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());

			for (k = 2; k < 8; k *= 2)
			{
				B = A->scalarProduct(k);
				B->ref();

				C = B->matrixDividedByScalar(k);
				C->ref();

				if (C->equals(A))
				{
					PASS(__FILE__, __LINE__, "Mult/divide ok\n");
				} else
				{
					FAIL(__FILE__, __LINE__, "Mult/divide failed:\n");
					FAIL(__FILE__, __LINE__, " got '%s'\n",
					     C->getValueString().getValue());
					FAIL(__FILE__, __LINE__, "want '%s'\n",
					     A->getValueString().getValue());
				}
				B->unref();
				C->unref();
			}

			for (k = 2; k < 64; k *= 2)
			{
				B = A->scalarSum(k);
				B->ref();

				C = B->matrixMinusScalar(k);
				C->ref();

				if (C->equals(A))
				{
					PASS(__FILE__, __LINE__, "Plus/minus ok\n");
				} else
				{
					FAIL(__FILE__, __LINE__, "Plus/minus failed:\n");
					FAIL(__FILE__, __LINE__, " got '%s'\n",
					     C->getValueString().getValue());
					FAIL(__FILE__, __LINE__, "want '%s'\n",
					     A->getValueString().getValue());
				}
				B->unref();
				C->unref();
			}

			A->unref();
		}
	}

	return 1;
}

int
testMatrixScalarOps()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> example 1\n");
	s = test1();
	status = status && s;

	return status;
}
