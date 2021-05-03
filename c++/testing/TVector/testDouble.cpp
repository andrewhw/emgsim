#include <stdio.h>
#include "tlTVector.h"

static int
testDoubleFunctions()
{
    tlTVector<double> t;
    double d, test;
    int i, j;

    printf("<TESTCASE> testDoubleOperatorFunctions()\n");

    for (i = 0; i < 5; i++) {
		d = (i+1)*10.0;
		t.append(d);
    }

    for (i = 0; i < 5; i++) {
		d = t.getAt(i);
		test = (i+1)*10.0;
		if (d == test) {
		    printf("<PASS> -- Value at %3d is %f\n", i, d);
		} else {
		    printf("<FAIL> -- Value at %3d is %f\n", i, d);
		}
    }

    t.clear();
    for (i = 0; i < 10; i++) {
		d = (i+1)*100.0;
		t.append(d);
    }

    for (i = 0; i < t.getLength(); i++) {
		test = (i+1)*100.0;
		d = t.getAt(i);
		if (d == test) {
		    printf("<PASS> -- Value at %3d is %f\n", i, t.getAt(i));
		} else {
		    printf("<FAIL> -- Value at %3d is %f -- expected %f\n",
						i, t.getAt(i), test);
		}
    }


    for (i = 10; i >= 0; i--) {
		j = i * 2;
		d = (j+1);
		t.insertAt(i, d);
    }

    {
		static double e[] = {
		    		1, 100,
				3, 200,
				5, 300,
				7, 400,
				9, 500,
				11, 600,
				13, 700,
				15, 800,
				17, 900,
				19, 1000,
				21
		    };
		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    d = t.getAt(i);
		    if (d == test) {
				printf("<PASS> -- Value at %3d is %f\n", i, d);
		    } else {
				printf("<FAIL> -- Value at %3d is %f -- expected %f\n",
						    i, d, test);
		    }
		}
    }

    /** set up for remove test */
    t.clear();
    for (i = 0; i < 10; i++) {
		d = (i+1)*100.0;
		t.append(d);
    }

    for (i = 8; i >= 0; i -= 2) {
		t.removeAt(i);
    }

    {
		static double e[] = {
		    		200, 400, 600, 800, 1000
		    };
		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    d = t.getAt(i);
		    if (d == test) {
				printf("<PASS> -- Value at %3d is %f\n", i, d);
		    } else {
				printf("<FAIL> -- Value at %3d is %f -- expected %f\n",
						    i, d, test);
		    }
		}
    }
    return 1;
}

int
testDoubleOperator()
{
    tlTVector<double> t;
    double d, test;
    int i;

    printf("<TESTCASE> testDoubleOperator()\n");

    for (i = 0; i < 5; i++) {
		d = (i+1)*10.0;
		t[i] = d;
    }

    for (i = 0; i < 5; i++) {
		d = t.getAt(i);
		test = (i+1)*10.0;
		if (d == test) {
		    printf("<PASS> -- Value at %3d is %f\n", i, d);
		} else {
		    printf("<FAIL> -- Value at %3d is %f\n", i, d);
		}
    }

    t.clear();

    for (i = 0; i < 10; i++) {
		d = (i+1)*100.0;
		t[i] = d;
    }

    for (i = 0; i < t.getLength(); i++) {
		test = (i+1)*100.0;
		d = t.getAt(i);
		if (d == test) {
		    printf("<PASS> -- Value at %3d is %f\n", i, t.getAt(i));
		} else {
		    printf("<FAIL> -- Value at %3d is %f -- expected %f\n",
						i, t.getAt(i), test);
		}
    }

    for (i = 8; i >= 0; i -= 2) {
		t.removeAt(i);
    }

    {
		static double e[] = {
		    		200, 400, 600, 800, 1000
		    };
		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    d = t.getAt(i);
		    if (d == test) {
				printf("<PASS> -- Value at %3d is %f\n", i, d);
		    } else {
				printf("<FAIL> -- Value at %3d is %f -- expected %f\n",
						    i, d, test);
		    }
		}
    }
    return 1;
}

int
testDouble()
{
    int status;

    printf("<TESTCASE> testDouble()\n");

    status = testDoubleFunctions();
    status = testDoubleOperator() && status;

    return status;
}
