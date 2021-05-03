#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>


static int
testShieldsExample2x2()
{
	tlMatrix *m;
	tlSrString s, t;
	double r, expected = (-14);

	{
		m = new tlMatrix(2, 2);
		m->ref();
		s = "[ 3 1 ; 2 -4 ]";
		if (m->setValues(s) < 0)
		{
			FAIL(__FILE__, __LINE__, "Failure on load\n");
			return 0;
		}
		DBG(__FILE__, __LINE__, "m = %s\n", m->getValueString().getValue());

		r = m->determinant();

		if (r == expected)
		{
			PASS(MK, "Got expected determinant of %f\n", r);
		} else
		{
			FAIL(MK, "Expected determinant of %f, got %f\n",
			     expected, r);
		}
		m->unref();
	}

	return 1;
}


static int
testShieldsExample3x3()
{
	tlMatrix *m;
	tlSrString s, t;
	double r, expected = (-9);

	{
		m = new tlMatrix(3, 3);
		m->ref();
		s = "[ 3 1 2 ; 4 1 -6 ; 1 0 1 ]";
		if (m->setValues(s) < 0)
		{
			FAIL(__FILE__, __LINE__, "Failure on load\n");
			return 0;
		}
		DBG(__FILE__, __LINE__, "m = %s\n", m->getValueString().getValue());

		r = m->determinant();

		if (r == expected)
		{
			PASS(MK, "Got expected determinant of %f\n", r);
		} else
		{
			FAIL(MK, "Expected determinant of %f, got %f\n",
			     expected, r);
		}
		m->unref();
	}

	return 1;
}


static int
testShieldsExample4x4()
{
	tlMatrix *m;
	tlSrString s, t;
	double r, expected = (-20);

	{
		m = new tlMatrix(4, 4);
		m->ref();
		s = "[ 4 1 2 -1 ; 3 -1 2 1 ; 6 -1 0 -1 ; 2 -1 1 0 ]";
		if (m->setValues(s) < 0)
		{
			FAIL(__FILE__, __LINE__, "Failure on load\n");
			return 0;
		}
		DBG(__FILE__, __LINE__, "m = %s\n", m->getValueString().getValue());

		r = m->determinant();

		if (r == expected)
		{
			PASS(MK, "Got expected determinant of %f\n", r);
		} else
		{
			FAIL(MK, "Expected determinant of %f, got %f\n",
			     expected, r);
		}
		m->unref();
	}

	return 1;
}


int
testMatrixDeterminant()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> Shields examples\n");
	s = testShieldsExample2x2();
	status = status && s;

	s = testShieldsExample3x3();
	status = status && s;

	s = testShieldsExample4x4();
	status = status && s;

	return status;
}
