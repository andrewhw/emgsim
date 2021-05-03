#include <stdio.h>
#include "tlTDoubleLinkedList.h"
#include "testutils.h"

int
testDouble()
{
    tlTDoubleLinkedList<double> list;
    tlTDoubleLinkedList<double>::tlTNodeDoubleLinkedList *node;
    tlTIteratorDoubleLinkedList<double> iterator(&list);
    double d, test;
    int i;


    printf("<TESTCASE> testDoubleData()\n");

    for (i = 0; i < 5; i++) {
		d = (i+1)*10.0;
		list.append(d);
    }

    node = list.getHead();
    for (i = 0; i < 5; i++) {
		test = (i+1)*10.0;
		if (node->data() == test) {
		    PASS(MK, "Value at %3d is %f\n", i, test);
		} else {
		    FAIL(MK, "Value at %3d is %f, expected %f\n", i,
		    				node->data(), test);
		}
		node = node->next_;
    }

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test = (i+1)*10.0;
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is %f\n", i, test);
		} else {
		    FAIL(MK, "Value at %3d is %f, expected %f\n", i,
		    				iterator.node()->data(), test);
		}
		iterator.next();
		i++;
    }

    DBG(MK, "Starting iterator insertion tests\n");

    list.clear();
    iterator.gotoHead();

    iterator.insertBeforeCurrent(30.0);
    iterator.insertBeforeCurrent(20.0);
    iterator.insertBeforeCurrent(10.0);

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test = (i+1)*10.0;
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is %f\n", i, test);
		} else {
		    FAIL(MK, "Value at %3d is %f, expected %f\n", i,
		    				iterator.node()->data(), test);
		}
		iterator.next();
		i++;
    }


    DBG(MK, "Starting iterator insertion tests\n");

    list.clear();
    iterator.gotoHead();

    iterator.insertBeforeCurrent(30.0);
    iterator.insertBeforeCurrent(10.0);
    iterator.next();
    iterator.insertBeforeCurrent(20.0);
    iterator.gotoTail();
    iterator.insertAfterCurrent(50.0);
    iterator.prev();
    iterator.insertAfterCurrent(40.0);

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test = (i+1)*10.0;
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is %f\n", i, test);
		} else {
		    FAIL(MK, "Value at %3d is %f, expected %f\n", i,
		    				iterator.node()->data(), test);
		}
		iterator.next();
		i++;
    }

    DBG(MK, "Deleting after iterator insert\n");

    iterator.deleteCurrent();
    iterator.gotoHead();
    iterator.deleteCurrent();


    i = 0;
    while ( ! iterator.isAtEnd() ) {
		test = (i+2)*10.0;
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is %f\n", i, test);
		} else {
		    FAIL(MK, "Value at %3d is %f, expected %f\n", i,
		    				iterator.node()->data(), test);
		}
		iterator.next();
		i++;
    }

    return 1;
}

