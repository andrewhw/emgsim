#include <stdio.h>

#include <tlMatrix.h>

#include <testutils.h>

static int
testLoad1Element()
{
	tlMatrix *m;
	tlSrString s, t;
	int i;

	m = new tlMatrix(1, 1);
	m->ref();
	for (i = 0; i < 10; i++)
	{
		s.sprintf("[ %-9.3g ]", (float) i);
		if (m->setValues(s) < 0)
		{
			FAIL(__FILE__, __LINE__, "setValues('%s')\n", s.getValue());
		}
		t = m->getValueString();
		if (t == s)
		{
			PASS(__FILE__, __LINE__, "Load ok\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
			     "got '%s', expected '%s'\n", t.getValue(), s.getValue());
		}
	}

	m->unref();

	return 1;
}

static int
testLoad2Elements()
{
	tlMatrix *m;
	tlSrString s, t;
	int i;

	m = new tlMatrix(1, 2);
	m->ref();
	for (i = 0; i < 10; i++)
	{
		s.sprintf("[ %-9.3g %-9.3g ]", (float) i, (float) i);
		if (m->setValues(s) < 0)
		{
			FAIL(__FILE__, __LINE__, "setValues('%s')\n", s.getValue());
		}
		t = m->getValueString();
		if (t == s)
		{
			PASS(__FILE__, __LINE__, "Load ok\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
			     "got '%s', expected '%s'\n",
			     t.getValue(), s.getValue());
		}
	}
	m->unref();

	m = new tlMatrix(2, 1);
	m->ref();
	for (i = 0; i < 10; i++)
	{
		s.sprintf("[ %-9.3g ; %-9.3g ]", (float) i, (float) i * 10);
		if (m->setValues(s) < 0)
		{
			FAIL(__FILE__, __LINE__, "setValues('%s')\n", s.getValue());
		}
		t = m->getValueString();
		//DBG(__FILE__, __LINE__, "s = %s\n", s.getValue());
		//DBG(__FILE__, __LINE__, "t = %s\n", t.getValue());
		if (t == s)
		{
			PASS(__FILE__, __LINE__, "Load ok\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
			     "got '%s', expected '%s'\n",
			     t.getValue(), s.getValue());
		}
	}

	m->unref();

	return 1;
}

static int
testLoad2x2()
{
	const int MAX = 2;
	tlMatrix *m;
	tlSrString s, t;
	int i, j, k;
	double v;

	m = new tlMatrix(MAX, MAX);
	m->ref();
	for (i = 0; i < 10; i++)
	{
		s.sprintf("[");
		for (j = 0; j < MAX; j++)
		{
			for (k = 0; k < MAX; k++)
			{
				v = (float) (j * 10) + (float) (k + 1);
				(*m)[j][k] = v;
				s.sprintf("%S %-9.3g", &s, v);
			}
			if (j != (MAX - 1))
				s.sprintf("%S ;", &s);
			else
				s.sprintf("%S ]", &s);
		}

		t = m->getValueString();
		if (t == s)
		{
			PASS(__FILE__, __LINE__, "Load ok\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
			     "got '%s', expected '%s'\n",
			     t.getValue(), s.getValue());
		}
	}
	m->unref();

	return 1;
}

static int
testLoad3x3()
{
	const int MAX = 3;
	tlMatrix *m;
	tlSrString s, t;
	int i, j, k;
	double v;

	m = new tlMatrix(MAX, MAX);
	m->ref();
	for (i = 0; i < 10; i++)
	{
		s.sprintf("[");
		for (j = 0; j < MAX; j++)
		{
			for (k = 0; k < MAX; k++)
			{
				v = (float) (j * 10) + (float) (k + 1);
				(*m)[j][k] = v;
				s.sprintf("%S %-9.3g", &s, v);
			}
			if (j != (MAX - 1))
				s.sprintf("%S ;", &s);
			else
				s.sprintf("%S ]", &s);
		}

		t = m->getValueString();
		if (t == s)
		{
			PASS(__FILE__, __LINE__, "Load ok\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
			     "got '%s', expected '%s'\n",
			     t.getValue(), s.getValue());
		}
	}
	m->unref();

	return 1;
}

static int
testLoad3x6()
{
	const int MAXX = 3;
	const int MAXY = 6;
	tlMatrix *m;
	tlSrString s, t;
	int i, j, k;
	double v;

	m = new tlMatrix(MAXX, MAXY);
	m->ref();
	for (i = 0; i < 10; i++)
	{
		s.sprintf("[");
		for (j = 0; j < MAXX; j++)
		{
			for (k = 0; k < MAXY; k++)
			{
				v = (float) (j * 10) + (float) (k + 1);
				(*m)[j][k] = v;
				s.sprintf("%S %-9.3g", &s, v);
			}
			if (j != (MAXX - 1))
				s.sprintf("%S ;", &s);
			else
				s.sprintf("%S ]", &s);
		}

		t = m->getValueString();
		if (t == s)
		{
			PASS(__FILE__, __LINE__, "Load ok\n");
		} else
		{
			FAIL(__FILE__, __LINE__, " got '%s'\n", t.getValue());
			FAIL(__FILE__, __LINE__, "want '%s'\n", s.getValue());
		}
	}
	m->unref();

	return 1;
}

int
testMatrixLoad()
{
	int status, s;

	status = 1;

	printf("<SUBTESTCASE> 1 element test\n");
	s = testLoad1Element();
	status = status && s;

	printf("<SUBTESTCASE> 2 element test\n");
	s = testLoad2Elements();
	status = status && s;

	printf("<SUBTESTCASE> 2x2 test\n");
	s = testLoad2x2();
	status = status && s;

	printf("<SUBTESTCASE> 3x3 test\n");
	s = testLoad3x3();
	status = status && s;

	printf("<SUBTESTCASE> 3x3 test\n");
	s = testLoad3x6();
	status = status && s;

	return status;
}
