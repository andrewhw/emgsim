#include <stdio.h>
#include "tlTDoubleLinkedList.h"
#include "tlSrString.h"
#include "testutils.h"

static int
testStringLocal()
{
    tlTDoubleLinkedList<tlSrString> list;
    tlTDoubleLinkedList<tlSrString>::tlTNodeDoubleLinkedList *node;
    tlTIteratorDoubleLinkedList<tlSrString> iterator(&list);
    tlSrString s, test;
    int i;


    printf("<TESTCASE> testStringData()\n");

    for (i = 0; i < 5; i++) {
		s.sprintf("string(%d)", (i+1)*10);
		list.append(s);
    }

    node = list.getHead();
    for (i = 0; i < 5; i++) {
		test.sprintf("string(%d)", (i+1)*10);
		if (node->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				node->data().getValue(),
		    				test.getValue());
		}
		node = node->next_;
    }

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test.sprintf("string(%d)", (i+1)*10);
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator.node()->data().getValue(),
		    				test.getValue());
		}
		iterator.next();
		i++;
    }

    DBG(MK, "Starting iterator insertion tests\n");

    list.clear();
    iterator.gotoHead();

    s.sprintf("string(%d)", 30);
    iterator.insertBeforeCurrent(s);
    s.sprintf("string(%d)", 20);
    iterator.insertBeforeCurrent(s);
    s.sprintf("string(%d)", 10);
    iterator.insertBeforeCurrent(s);

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test.sprintf("string(%d)", (i+1)*10);
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator.node()->data().getValue(),
		    				test.getValue());
		}
		iterator.next();
		i++;
    }


    DBG(MK, "Starting iterator insertion tests\n");

    list.clear();
    iterator.gotoHead();

    s.sprintf("string(%d)", 30);
    iterator.insertBeforeCurrent(s);
    s.sprintf("string(%d)", 10);
    iterator.insertBeforeCurrent(s);
    iterator.next();
    s.sprintf("string(%d)", 20);
    iterator.insertBeforeCurrent(s);
    iterator.gotoTail();
    s.sprintf("string(%d)", 50);
    iterator.insertAfterCurrent(s);
    iterator.prev();
    s.sprintf("string(%d)", 40);
    iterator.insertAfterCurrent(s);

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test.sprintf("string(%d)", (i+1)*10);
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator.node()->data().getValue(),
		    				test.getValue());
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
		test.sprintf("string(%d)", (i+2)*10);
		if (iterator.node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator.node()->data().getValue(),
		    				test.getValue());
		}
		iterator.next();
		i++;
    }

    return 1;
}

static int
testStringPtr()
{
    tlTDoubleLinkedList<tlSrString> *list;
    tlTDoubleLinkedList<tlSrString>::tlTNodeDoubleLinkedList *node;
    tlTIteratorDoubleLinkedList<tlSrString> *iterator;
    tlSrString s, test;
    int i;


    list = new tlTDoubleLinkedList<tlSrString>();
    iterator = new tlTIteratorDoubleLinkedList<tlSrString>(list);

    printf("<TESTCASE> testStringData()\n");

    for (i = 0; i < 5; i++) {
		s.sprintf("string(%d)", (i+1)*10);
		list->append(s);
    }

    node = list->getHead();
    for (i = 0; i < 5; i++) {
		test.sprintf("string(%d)", (i+1)*10);
		if (node->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				node->data().getValue(),
		    				test.getValue());
		}
		node = node->next_;
    }

    i = 0;
    iterator->gotoHead();
    while ( ! iterator->isAtEnd() ) {
		test.sprintf("string(%d)", (i+1)*10);
		if (iterator->node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator->node()->data().getValue(),
		    				test.getValue());
		}
		iterator->next();
		i++;
    }

    DBG(MK, "Starting iterator insertion tests\n");

    list->clear();
    iterator->gotoHead();

    s.sprintf("string(%d)", 30);
    iterator->insertBeforeCurrent(s);
    s.sprintf("string(%d)", 20);
    iterator->insertBeforeCurrent(s);
    s.sprintf("string(%d)", 10);
    iterator->insertBeforeCurrent(s);

    i = 0;
    iterator->gotoHead();
    while ( ! iterator->isAtEnd() ) {
		test.sprintf("string(%d)", (i+1)*10);
		if (iterator->node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator->node()->data().getValue(),
		    				test.getValue());
		}
		iterator->next();
		i++;
    }


    DBG(MK, "Starting iterator insertion tests\n");

    list->clear();
    iterator->gotoHead();

    s.sprintf("string(%d)", 30);
    iterator->insertBeforeCurrent(s);
    s.sprintf("string(%d)", 10);
    iterator->insertBeforeCurrent(s);
    iterator->next();
    s.sprintf("string(%d)", 20);
    iterator->insertBeforeCurrent(s);
    iterator->gotoTail();
    s.sprintf("string(%d)", 50);
    iterator->insertAfterCurrent(s);
    iterator->prev();
    s.sprintf("string(%d)", 40);
    iterator->insertAfterCurrent(s);

    i = 0;
    iterator->gotoHead();
    while ( ! iterator->isAtEnd() ) {
		test.sprintf("string(%d)", (i+1)*10);
		if (iterator->node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator->node()->data().getValue(),
		    				test.getValue());
		}
		iterator->next();
		i++;
    }

    DBG(MK, "Deleting after iterator insert\n");

    iterator->deleteCurrent();
    iterator->gotoHead();
    iterator->deleteCurrent();


    i = 0;
    while ( ! iterator->isAtEnd() ) {
		test.sprintf("string(%d)", (i+2)*10);
		if (iterator->node()->data() == test) {
		    PASS(MK, "Value at %3d is '%s'\n", i, test.getValue());
		} else {
		    FAIL(MK, "Value at %3d is '%s', expected '%s'\n", i,
		    				iterator->node()->data().getValue(),
		    				test.getValue());
		}
		iterator->next();
		i++;
    }

    return 1;
}

int
testString()
{
    int status;

    status = testStringLocal();
    status = testStringPtr() && status;

    return status;
}
