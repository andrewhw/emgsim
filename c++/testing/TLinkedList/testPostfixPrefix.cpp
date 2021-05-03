#include <stdio.h>
#include "tlTDoubleLinkedList.h"
#include "testutils.h"

int
testPostfixPrefix()
{
    tlTDoubleLinkedList<double> list;
    tlTIteratorDoubleLinkedList<double> iterator(&list);
    double d, test;
    int i;


    printf("<TESTCASE> testPostfixPrefix()\n");

    for (i = 0; i < 5; i++) {
		d = (i+1)*10.0;
		list.append(d);
    }

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		d = iterator++;
		test = (i+1)*10.0;
		DBG(MK, "Value at %3d is %f, expecting %f\n", i, d, test);
		if (d == test) {
		    PASS(MK, "Value at %3d is %f\n", i, test);
		} else {
		    FAIL(MK, "Value at %3d is %f, expected %f\n", i,
		    				iterator.node()->data(), test);
		}
		i++;
    }

    return 1;
}

