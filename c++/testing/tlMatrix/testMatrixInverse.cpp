#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>


static int
testStrangExample1()
{
	tlMatrix *A, *R, *T;
	tlSrString s;

	A = new tlMatrix(3, 3);
	A->ref();
	s = "[ 2 -1 0 ; -1 2 -1 ; 0 -1 2 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	R = A->inverse();
	R->ref();

	T = new tlMatrix(3, 3);
	T->ref();
	s = "[ 0.75 0.5 0.25 ; 0.5 1 0.5 ; 0.25 0.5 0.75 ]";
	if (T->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	if (T->equals(R))
	{
		PASS(__FILE__, __LINE__, "Inverse values match\n");
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n", R->getValueString().getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n", T->getValueString().getValue());
	}

	R->unref();
	T->unref();
	A->unref();

	return 1;
}

static int
testFLPaperExample1()
{
	tlMatrix *A, *R, *T;
	tlSrString s;

	A = new tlMatrix(4, 4);
	A->ref();
	s = "[ 1000000 1000 0 625 ; 1000 4 0.15 0 ; 0 0.15 0.1 0.125 ; 625 0 0.125 25 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	R = A->inverse();
	R->ref();

	T = new tlMatrix(4, 4);
	T->ref();
	s = "[     1.36e-06   -0.000371    0.000605   -3.79e-05 ; -0.000341    0.364   -0.561    0.0121 ; 0   -0.5610   10.9   -0.0698 ; -0    0.0121   -0.0698    0.0413 ]";

	if (T->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	if (T->equals(R))
	{
		PASS(__FILE__, __LINE__, "Inverse values match\n");
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n", R->getValueString().getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n", T->getValueString().getValue());
	}

	R->unref();
	T->unref();
	A->unref();

	return 1;
}

static int
testMatlabExample1()
{
	tlMatrix *A, *R, *T;
	tlSrString s;

	A = new tlMatrix(4, 4);
	A->ref();
	s = "[ 1 2 3 4 ; 0 4 2 1 ; 8 9 0.5 2 ; 14 14 14 14]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	R = A->inverse();
	R->ref();

	T = new tlMatrix(4, 4);
	T->ref();
	s = "[  -0.2277   -0.1980    0.0198    0.0764 ; 0.0891    0.2079    0.0792   -0.0516 ; -0.4950    0.1782   -0.2178    0.1598 ; 0.6337   -0.1881    0.1188   -0.1132]";




	if (T->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	if (T->equals(R))
	{
		PASS(__FILE__, __LINE__, "Inverse values match\n");
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n", R->getValueString().getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n", T->getValueString().getValue());
	}

	R->unref();
	T->unref();
	A->unref();

	return 1;
}

int
testMatrixInverse()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> Strang example\n");
	s = testStrangExample1();
	status = status && s;

	printf("<SUBTESTCASE> FL example\n");
	s = testFLPaperExample1();
	status = status && s;

	printf("<SUBTESTCASE> Matlab example\n");
	s = testMatlabExample1();
	status = status && s;

	return status;
}
