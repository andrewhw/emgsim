#include <stdio.h>

#include <tlGslMatrix.h>

#include <testutils.h>
#include "stringtools.h"


static int
testStrangExample1()
{
	tlGslMatrix *A, *R, *T;
	tlSrString s;
	float delta;

	A = new tlGslMatrix(3, 3);
	A->ref();
	s = "[ 2 -1 0 ; -1 2 -1 ; 0 -1 2 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	R = A->inverse();
	if (R == NULL)
	{
		FAIL(__FILE__, __LINE__, "inverse failed");
		return 0;
	}
	R->ref();

	T = new tlGslMatrix(3, 3);
	T->ref();


	s = "[ 0.75 0.5 0.25 ; 0.5 1 0.5 ; 0.25 0.5 0.75 ]";
	if (T->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	delta = T->minDifference(R);
	if (delta < 0.001)
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
	tlGslMatrix *A, *R, *T;
	tlSrString s;
	float delta;

	A = new tlGslMatrix(4, 4);
	A->ref();


	s = "[ 1000000 1000 0 625 ; 1000 4 0.15 0 ; 0 0.15 0.1 0.125 ; 625 0 0.125 25 ]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	R = A->inverse();
	if (R == NULL)
	{
		FAIL(__FILE__, __LINE__, "inverse failed");
		return 0;
	}
	R->ref();

	T = new tlGslMatrix(4, 4);
	T->ref();
	s = "[ 1.3951460543525649e-06 -0.00037145763697137046 0.00060456329021944474 -3.7901467809911355e-05 ;"
		" -0.00037145763697137046 0.36390059584362738 -0.5609649760209271 0.012091265804388901 ; "
		" 0.00060456329021944452 -0.5609649760209271 10.928644092428424 -0.069757302717628339 ; "
		" -3.7901467809911348e-05 0.012091265804388896 -0.069757302717628242 0.041296323208835924 ]";

	if (T->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	delta = T->minDifference(R);
	if (delta < 0.001)
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
	tlGslMatrix *A, *R, *T;
	tlSrString s;
	float delta;

	A = new tlGslMatrix(4, 4);
	A->ref();
	s = "[ 1 2 3 4 ; 0 4 2 1 ; 8 9 0.5 2 ; 14 14 14 14]";
	if (A->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	R = A->inverse();
	R->ref();

	T = new tlGslMatrix(4, 4);
	T->ref();
	s = "[ -0.22772277227722773 -0.19801980198019803 0.019801980198019806 0.076379066478076379; "
		" 0.089108910891089105 0.20792079207920794 0.079207920792079209 -0.051626591230551619; "
		" -0.49504950495049505 0.17821782178217821 -0.21782178217821782 0.1598302687411598; "
		" 0.63366336633663367 -0.18811881188118812 0.11881188118811881 -0.11315417256011313]";



	if (T->setValues(s) < 0)
	{
		FAIL(__FILE__, __LINE__, "Failure on load\n");
		return 0;
	}
	delta = T->minDifference(R);
	if (delta < 0.001)
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
testGslMatrixInverse()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> Strang example\n");
	s = testStrangExample1();
	status = status && s;

	printf("<SUBTESTCASE> Strang example\n");
	s = testFLPaperExample1();
	status = status && s;

	printf("<SUBTESTCASE> Matlab example\n");
	s = testMatlabExample1();
	status = status && s;

	return status;
}
