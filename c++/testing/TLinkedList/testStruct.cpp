#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tlTDoubleLinkedList.h"
#include "testutils.h"


class aClass {
public:
    int i_;
    double d_;
    const char *str_;
    char o_[BUFSIZ];

public:
    aClass() {
		i_ = 0;
		d_ = 0;
		str_ = NULL;
    }

    ~aClass() {
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

    char * getValue() {
		sprintf(o_, "i(%d) d(%f) s(%s)", i_, d_, str_);
		return o_;
    }

    aClass & operator= (const aClass &sibling) {
		i_ = sibling.i_;
		d_ = sibling.d_;
		str_ = sibling.str_;
		return *this;
    }

    int operator== (const aClass &sibling) {
		if (i_ != sibling.i_)		return 0;
		if (d_ != sibling.d_)		return 0;
		if (strcmp(str_, sibling.str_) != 0)		return 0;
		return 1;
    }

};

static const char *stringset[] = {
				"fred(0)",
				"fred(1)",
				"fred(2)",
				"fred(3)",
				"fred(4)",
				NULL
		};


int
testStruct()
{
    tlTDoubleLinkedList<aClass> list;
    tlTDoubleLinkedList<aClass>::tlTNodeDoubleLinkedList *node;
    tlTIteratorDoubleLinkedList<aClass> iterator(&list);
    aClass data[5], test;
    int i;


    printf("<TESTCASE> testStructData()\n");

    for (i = 0; i < 5; i++) {
		data[i].i_ = i;
		data[i].d_ = 10.0 + i;
		data[i].str_ = stringset[i];
    }

    for (i = 0; i < 5; i++) {
		list.append(data[i]);
		DBG(MK, "Struct[%d] = {%s}\n", i, data[i].getValue());
    }

    node = list.getHead();
    for (i = 0; i < 5; i++) {
		test = data[i];
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
		test = data[i];
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

    iterator.insertBeforeCurrent(data[2]);
    iterator.insertBeforeCurrent(data[1]);
    iterator.insertBeforeCurrent(data[0]);

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test = data[i];
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

    iterator.insertBeforeCurrent(data[2]);
    iterator.insertBeforeCurrent(data[0]);
    iterator.next();
    iterator.insertBeforeCurrent(data[1]);
    iterator.gotoTail();
    iterator.insertAfterCurrent(data[4]);
    iterator.prev();
    iterator.insertAfterCurrent(data[3]);

    i = 0;
    iterator.gotoHead();
    while ( ! iterator.isAtEnd() ) {
		test = data[i];
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
		test = data[i+1];
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

