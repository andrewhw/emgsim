#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tlTVector.h"
#include "tlSrString.h"

int
testStringFunctions()
{
    tlTVector<tlSrString> t;
    tlSrString s;
    const char *test;
    int i;

    printf("<TESTCASE> testStringFunctions()\n");

    for (i = 0; i < 5; i++) {
		s.sprintf("string(%d)", i);
		t.append(s);
    }

    {
		static const char *e[] = {
		    		"string(0)",
		    		"string(1)",
		    		"string(2)",
		    		"string(3)",
		    		"string(4)"
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }
    
    printf("    <TEST> clear\n");
    t.clear();
    if (t.getLength() == 0) {
		printf("<PASS> -- Length is 0\n");
    } else {
		printf("<FAIL> -- Length after clear is %d\n", t.getLength());
    }


    printf("    <TEST> append\n");
    for (i = 0; i < 10; i++) {
		s.sprintf("test(%d)", i);
		t.append(s);
    }

    {
		static const char *e[] = {
		    		"test(0)",
		    		"test(1)",
		    		"test(2)",
		    		"test(3)",
		    		"test(4)",
		    		"test(5)",
		    		"test(6)",
		    		"test(7)",
		    		"test(8)",
		    		"test(9)"
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }


    printf("    <TEST> insert\n");
    for (i = 0; i < 10; i+=2) {
		s.sprintf("fred(%d)", i);
		t.insertAt(i,s);
    }

    {
		static const char *e[] = {
		    		"fred(0)",
		    		"test(0)",
		    		"fred(2)",
		    		"test(1)",
		    		"fred(4)",
		    		"test(2)",
		    		"fred(6)",
		    		"test(3)",
		    		"fred(8)",
		    		"test(4)",
		    		"test(5)",
		    		"test(6)",
		    		"test(7)",
		    		"test(8)",
		    		"test(9)"
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }

    for (i = t.getLength() - 1; i > 0; i -= 2) {
		t.removeAt(i);
    }

    {
		static const char *e[] = {
		    		"fred(0)",
		    		"test(0)",
		    		"test(1)",
		    		"test(2)",
		    		"test(3)",
		    		"test(4)",
		    		"test(6)",
		    		"test(8)"
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }

    return 1;
}

int
testStringOperator()
{
    tlTVector<tlSrString> t;
    tlSrString s;
    const char *test;
    int i;

    printf("<TESTCASE> testStringOperator()\n");

    for (i = 0; i < 5; i++) {
		s.sprintf("string(%d)", i);
		t[i] = s;
    }

    {
		static const char *e[] = {
		    		"string(0)",
		    		"string(1)",
		    		"string(2)",
		    		"string(3)",
		    		"string(4)"
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }
    t.clear();

    for (i = 0; i < 10; i++) {
		s.sprintf("fred(%d)", i);
		t[i * 2] = s;
    }

    {
		static const char *e[] = {
		    		"fred(0)",
				"",
		    		"fred(1)",
				"",
		    		"fred(2)",
				"",
		    		"fred(3)",
				"",
		    		"fred(4)",
				"",
		    		"fred(5)",
				"",
		    		"fred(6)",
				"",
		    		"fred(7)",
				"",
		    		"fred(8)",
				"",
		    		"fred(9)",
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }


    for (i = t.getLength() - 1; i > 0; i -= 2) {
		t.removeAt(i);
    }

    {
		static const char *e[] = {
		    		"fred(0)",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				"",
				""
		    };

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test);
		    }
		}
    }

    return 1;
}


int
testString()
{
    int status;

    printf("<TESTCASE> testString()\n");

    status = testStringFunctions();
    status = testStringOperator() && status;

    return status;
}
