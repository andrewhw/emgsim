#include <stdio.h>

#include <tlGslMatrix.h>

#include <testutils.h>


static int
testStrangExample1()
{
	tlGslMatrix *A, *B, *R, *T;
	tlSrString s;

	A = new tlGslMatrix(2, 2);
	A->ref();
	s = "[ 1 1 ; 2 -1 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());

	B = new tlGslMatrix(2, 2);
	B->ref();
	s = "[ 2 2 ; 3 4 ]";
	if (B->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "B = %s\n", B->getValueString().getValue());


	R = A->product(B);
	R->ref();


	s = "[ 5 6 ; 1 0]";
	T = new tlGslMatrix(2, 2);
	T->ref();
	T->setValues(s);


	if (R->equals(T))
	{
		PASS(__FILE__, __LINE__, "Compare ok\n");
		DBG(__FILE__, __LINE__, "R = %s\n", R->getValueString(0).getValue());
		DBG(__FILE__, __LINE__, "T = %s\n", T->getValueString(0).getValue());
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n",
		     R->getValueString(0).getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n",
		     T->getValueString(0).getValue());
	}

	A->unref();
	B->unref();
	R->unref();
	T->unref();

	return 1;
}


static int
testStrangExample2()
{
	tlGslMatrix *A, *B, *R, *T;
	tlSrString s;

	A = new tlGslMatrix(3, 1);
	A->ref();
	s = "[ 0 ; 1 ; 2 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());

	B = new tlGslMatrix(1, 3);
	B->ref();
	s = "[ 1 2 3 ]";
	if (B->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "B = %s\n", B->getValueString().getValue());


	R = A->product(B);
	R->ref();


	s = "[ 0 0 0 ; 1 2 3 ; 2 4 6 ]";
	T = new tlGslMatrix(3, 3);
	T->ref();
	T->setValues(s);


	if (R->equals(T))
	{
		PASS(__FILE__, __LINE__, "Compare ok\n");
		DBG(__FILE__, __LINE__, "R = %s\n", R->getValueString(0).getValue());
		DBG(__FILE__, __LINE__, "T = %s\n", T->getValueString(0).getValue());
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n",
		     R->getValueString(0).getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n",
		     T->getValueString(0).getValue());
	}

	A->unref();
	B->unref();
	R->unref();
	T->unref();

	return 1;
}


int
testGslMatrixProduct()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> example 1\n");
	s = testStrangExample1();
	status = status && s;

	printf("<SUBTESTCASE> example 2\n");
	s = testStrangExample2();
	status = status && s;

	return status;
}
