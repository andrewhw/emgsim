#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tlTVector.h"

class aClass {
private:
    int i_;
    double d_;
    char *str_;
    char o_[BUFSIZ];

public:
    aClass() {
		i_ = 0;
		d_ = 0;
		str_ = NULL;
    }
    aClass(int i, double d, const char *str) {
		i_ = i;
		d_ = d;
		str_ = strdup(str);
    }

    ~aClass() {
		if (str_ != NULL) {
		    free(str_);
		}
    }

    int getI() const {
		return i_;
    }
    double getD() const {
		return d_;
    }
    const char * getS() const {
		return str_;
    }

    const char * getValue() {
		sprintf(o_, "i(%d) d(%f) s(%s)", i_, d_, str_);
		return o_;
    }

    aClass & operator= (const aClass &sibling) {
		i_ = sibling.i_;
		d_ = sibling.d_;
		str_ = strdup(sibling.str_);
		return *this;
    }

    int operator== (const aClass &sibling) {
		if (i_ != sibling.i_)		return 0;
		if (d_ != sibling.d_)		return 0;
		if (strcmp(str_, sibling.str_) != 0)		return 0;
		return 1;
    }

};

int
testStruct()
{
    char buffer[128];
    tlTVector<aClass> t;
    aClass s, *test;
    int i;

    printf("<TESTCASE> testStruct()\n");

    printf("<TEST> simple Append\n");
    for (i = 0; i < 5; i++) {
		sprintf(buffer, "fred(%d)", i);
		aClass q(i + 1, i + 10, buffer);
		t.append(q);
    }

    {
		static aClass *e[20];

		e[0] = new aClass( 1, 10, "fred(0)" );
		e[1] = new aClass( 2, 11, "fred(1)" );
		e[2] = new aClass( 3, 12, "fred(2)" );
		e[3] = new aClass( 4, 13, "fred(3)" );
		e[4] = new aClass( 5, 14, "fred(4)" );

		for (i = 0; i < t.getLength(); i++) {
		    test = e[i];
		    s = t.getAt(i);
		    if (s == *test) {
				printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
		    } else {
				printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
				    		i, s.getValue(), test->getValue());
		    }
		}
    }
    t.clear();

    printf("<TEST> append then remove\n");

    for (i = 0; i < 10; i++) {
		sprintf(buffer, "fred(%d)", i);
		aClass q(i + 10, i + 100, buffer);
		t.append(q);
    }

    {
		static aClass *e[20];

		e[0] = new aClass( 10, 100, "fred(0)" );
		e[1] = new aClass( 11, 101, "fred(1)" );
		e[2] = new aClass( 12, 102, "fred(2)" );
		e[3] = new aClass( 13, 103, "fred(3)" );
		e[4] = new aClass( 14, 104, "fred(4)" );
		e[5] = new aClass( 15, 105, "fred(5)" );
		e[6] = new aClass( 16, 106, "fred(6)" );
		e[7] = new aClass( 17, 107, "fred(7)" );
		e[8] = new aClass( 18, 108, "fred(8)" );
		e[9] = new aClass( 19, 109, "fred(9)" );

		for (i = 0; i < t.getLength(); i++) {
		    s = t.getAt(i);
		    test = e[i];
		    if (test == NULL) {
				printf("<FAIL> -- Value at %3d is : %s -- expected EOL\n",
				    		i, s.getValue());
		    } else {
				if (s == *test) {
				    printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
				} else {
				    printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
						    i, s.getValue(), test->getValue());
				}
		    }
		}
    }

    for (i = t.getLength() - 1; i > 0; i -= 2) {
		t.removeAt(i);
    }

    {
		static aClass *e[20];

		e[0] = new aClass( 10, 100, "fred(0)" );
		e[1] = new aClass( 12, 102, "fred(2)" );
		e[2] = new aClass( 14, 104, "fred(4)" );
		e[3] = new aClass( 16, 106, "fred(6)" );
		e[4] = new aClass( 18, 108, "fred(8)" );

		for (i = 0; i < t.getLength(); i++) {
		    s = t.getAt(i);
		    test = e[i];
		    if (test == NULL) {
				printf("<FAIL> -- Value at %3d is : %s -- expected EOL\n",
				    		i, s.getValue());
		    } else {
				if (s == *test) {
				    printf("<PASS> -- Value at %3d is : %s\n", i, s.getValue());
				} else {
				    printf("<FAIL> -- Value at %3d is : %s -- expected %s\n",
						    i, s.getValue(), test->getValue());
				}
		    }
		}
    }
    return 1;
}



