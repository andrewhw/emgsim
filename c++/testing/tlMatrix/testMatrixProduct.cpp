#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>


static int
testStrangExample1()
{
	tlMatrix *A, *B, *P;
	tlSrString s, t, c;

	A = new tlMatrix(2, 2);
	A->ref();
	s = "[ 1 1 ; 2 -1 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());

	B = new tlMatrix(2, 2);
	B->ref();
	s = "[ 2 2 ; 3 4 ]";
	if (B->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "B = %s\n", B->getValueString().getValue());

	P = A->product(B);
	P->ref();

	c.sprintf("[ %-9.3g %-9.3g ; %-9.3g %-9.3g ]",
		  5.0, 6.0, 1.0, 0.0);

	t = P->getValueString();

	DBG(__FILE__, __LINE__, "P = %s\n", t.getValue());
	if (t == c)
	{
		PASS(__FILE__, __LINE__, "Load ok\n");
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n", c.getValue());
	}

	A->unref();
	B->unref();
	P->unref();

	return 1;
}


static int
testStrangExample2()
{
	tlMatrix *A, *B, *P;
	tlSrString s, t, c;

	A = new tlMatrix(3, 1);
	A->ref();
	s = "[ 0 ; 1 ; 2 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());

	B = new tlMatrix(1, 3);
	B->ref();
	s = "[ 1 2 3 ]";
	if (B->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	DBG(__FILE__, __LINE__, "B = %s\n", B->getValueString().getValue());

	P = A->product(B);
	P->ref();

	c.sprintf("[ %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g ]",
		  0.0, 0.0, 0.0,
		  1.0, 2.0, 3.0,
		  2.0, 4.0, 6.0);

	t = P->getValueString();

	DBG(__FILE__, __LINE__, "P = %s\n", t.getValue());
	if (t == c)
	{
		PASS(__FILE__, __LINE__, "Load ok\n");
	} else
	{
		FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
		FAIL(__FILE__, __LINE__, "want '%s'\n", c.getValue());
	}

	A->unref();
	B->unref();
	P->unref();

	return 1;
}

//
//static int
//testInverseExample1()
//{
//	tlMatrix *A, *B, *P, *C;
//	tlSrString s, t, c;
//
//
//
//	A = new tlMatrix(4, 4);
//	A->ref();
//	s = "[ 1000000 1000 0 625 ; 1000 4 0.15 0 ; 0 0.15 0.1 0.125 ; 625 0 0.125 25 ]";
//	if (A->setValues(s) < 0)
//	{
//		FAIL(__FILE__, __LINE__, "Failure on load\n");
//		return 0;
//	}
//	DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());
//
//	B = A->inverse();
//	B->ref();
//	DBG(__FILE__, __LINE__, "B = %s\n", B->getValueString().getValue());
//
//
//	c.sprintf("[ %-9.3g %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g %-9.3g ]",
//		  1.0, 0.0, 0.0, 0.0,
//		  0.0, 1.0, 0.0, 0.0,
//		  0.0, 0.0, 1.0, 0.0,
//		  0.0, 0.0, 0.0, 1.0);
//	C = new tlMatrix(4, 4);
//	C->ref();
//	if (C->setValues(c) < 0)
//	{
//		FAIL(__FILE__, __LINE__, "Failure on load\n");
//		return 0;
//	}
//	/** A * A^-1 */
//	P = A->product(B);
//	P->ref();
//	t = P->getValueString();
//	DBG(__FILE__, __LINE__, "P = %s\n", t.getValue());
//	if (t == c)
//	{
//		PASS(__FILE__, __LINE__, "Load ok\n");
//	} else
//	{
//		FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
//		FAIL(__FILE__, __LINE__, "want '%s'\n", c.getValue());
//	}
//	P->unref();
//
//
//
//	/** A^-1 * A */
//	P = B->product(A);
//	P->ref();
//	t = P->getValueString();
//	DBG(__FILE__, __LINE__, "P = %s\n", t.getValue());
//	if (C->equals(P))
//	{
//		PASS(__FILE__, __LINE__, "Load ok\n");
//	} else
//	{
//		FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
//		FAIL(__FILE__, __LINE__, "want '%s'\n", c.getValue());
//	}
//	P->unref();
//
//
//
//	A->unref();
//	B->unref();
//	C->unref();
//
//	return 1;
//}
//
//
//static int
//testInverseExample2()
//{
//	tlMatrix *A, *B, *P;
//	tlSrString s, t, c;
//
//
//	A = new tlMatrix(4, 4);
//	A->ref();
//	s = "[ 10000 100 0 62.5 ; 100 4 1.5 0 ; 0 1.5 1 1.25 ; 62.5 0 1.25 25 ]";
//	if (A->setValues(s) < 0)
//	{
//		FAIL(__FILE__, __LINE__, "Failure on load\n");
//		return 0;
//	}
//	DBG(__FILE__, __LINE__, "A = %s\n", A->getValueString().getValue());
//
//	B = A->inverse();
//	B->ref();
//	DBG(__FILE__, __LINE__, "B = %s\n", B->getValueString().getValue());
//
//
//	c.sprintf("[ %-9.3g %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g %-9.3g ; %-9.3g %-9.3g %-9.3g %-9.3g ;",
//		  1.0, 0.0, 0.0, 0.0,
//		  0.0, 1.0, 0.0, 0.0,
//		  0.0, 0.0, 1.0, 0.0,
//		  0.0, 0.0, 0.0, 1.0);
//
//
//	/** A * A^-1 */
//	P = A->product(B);
//	P->ref();
//	t = P->getValueString();
//	DBG(__FILE__, __LINE__, "P = %s\n", t.getValue());
//	if (t == c)
//	{
//		PASS(__FILE__, __LINE__, "Load ok\n");
//	} else
//	{
//		FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
//		FAIL(__FILE__, __LINE__, "want '%s'\n", c.getValue());
//	}
//	P->unref();
//
//
//
//	/** A^-1 * A */
//	P = B->product(A);
//	P->ref();
//	t = P->getValueString();
//	DBG(__FILE__, __LINE__, "P = %s\n", t.getValue());
//	if (t == c)
//	{
//		PASS(__FILE__, __LINE__, "Load ok\n");
//	} else
//	{
//		FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
//		FAIL(__FILE__, __LINE__, "want '%s'\n", c.getValue());
//	}
//	P->unref();
//
//
//
//	A->unref();
//	B->unref();
//
//	return 1;
//}

int
testMatrixProduct()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> example 1\n");
	s = testStrangExample1();
	status = status && s;

	printf("<SUBTESTCASE> example 2\n");
	s = testStrangExample2();
	status = status && s;

	//printf("<SUBTESTCASE> inverse example 1\n");
	//s = testInverseExample1();
	//status = status && s;

	//printf("<SUBTESTCASE> inverse example 2\n");
	//s = testInverseExample2();
	//status = status && s;

	return status;
}
