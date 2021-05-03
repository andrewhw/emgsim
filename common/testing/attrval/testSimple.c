#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tclCkalloc.h"
#include "attvalfile.h"
#include "tokens.h"

#include "testutils.h"

#define	NELEM	4
#define	EPSILON	0.0001


#define	TAG_SIMPLE	"Simple file"
#define	TESTFILE	"simpledata.txt"

#define	CHECK_MATCH(s, v) \
		((strcmp(s.attribute_, v->attribute_) == 0) \
			&& (s.type_ == v->type_) \
			&& ((s.type_ == TT_INTEGER) ? \
					(s.data_.ival_ == v->data_.ival_) : \
				(s.type_ == TT_REAL) ? \
					(fabs(s.data_.dval_ - v->data_.dval_) < EPSILON) : \
				(s.type_ == TT_STRING) ? \
					(strcmp(s.data_.strptr_, v->data_.strptr_) == 0) : \
				0))

static struct attVal sData[NELEM];
static int doneInit = 0;

static void
initData()
{
	if (doneInit)	return;
	
	doneInit = 1;

	sData[0].type_ = TT_INTEGER;
	sData[0].attribute_ = "intval";
	sData[0].data_.ival_ = 42;

	sData[1].type_ = TT_REAL;
	sData[1].attribute_ = "rval";
	sData[1].data_.dval_ = 123.456;

	sData[2].type_ = TT_REAL;
	sData[2].attribute_ = "small";
	sData[2].data_.dval_ = 0.000001;

	sData[3].type_ = TT_STRING;
	sData[3].attribute_ = "aString";
	sData[3].data_.strptr_ = "pancakes!";
}

int
testWrite(char *filename)
{
	FILE *ofp;
	attValList *list;
	attVal *value;
	int status = 1;

	if ((list = createAttValList()) == NULL) {
		FAIL(__FILE__, __LINE__, "Failed creating list\n");
		return 0;
	}


	value = createIntegerAttribute(sData[0].attribute_, sData[0].data_.ival_);
	if (addAttVal(list, value) <= 0)
		FAIL(__FILE__, __LINE__, "Failed adding (int) data element 0\n");
	if (CHECK_MATCH(sData[0], value))
		PASS(__FILE__, __LINE__, "match for (int) data element 0\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (int) data element 0\n");
		status = 0;
	}

	value = createRealAttribute(sData[1].attribute_, sData[1].data_.dval_);
	if (addAttVal(list, value) <= 0)
		FAIL(__FILE__, __LINE__, "Failed adding (real) data element 1\n");
	if (CHECK_MATCH(sData[1], value))
		PASS(__FILE__, __LINE__, "match for (real) data element 1\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (real) data element 1\n");
		status = 0;
	}

	value = createRealAttribute(sData[2].attribute_, sData[2].data_.dval_);
	if (addAttVal(list, value) <= 0)
		FAIL(__FILE__, __LINE__, "Failed adding (real) data element 2\n");
	if (CHECK_MATCH(sData[2], value))
		PASS(__FILE__, __LINE__, "match for (real) data element 2\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (real) data element 2\n");
		status = 0;
	}

	value = createStringAttribute(sData[3].attribute_, sData[3].data_.strptr_);
	if (addAttVal(list, value) <= 0)
		FAIL(__FILE__, __LINE__, "Failed adding (string) element 3\n");
	if (CHECK_MATCH(sData[3], value))
		PASS(__FILE__, __LINE__, "match for (string) element 3\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (string) element 3\n");
		status = 0;
	}

	
	if ((ofp = fopen(filename, "w")) == NULL) {
		FAIL(__FILE__, __LINE__, "Cannot open output file '%s' : %s\n",
				filename, strerror(errno));
		return 0;
	}
		
	if (writeAttValList(ofp, list, TAG_SIMPLE) <= 0) {
		FAIL(__FILE__, __LINE__, "Failed writing data file\n");
		status = 0;
	}

	fclose(ofp);

	return(status);
}


int
testRead(char *filename)
{
	attValList *list;
	attVal *value;
	int status = 1;

	if ((list = loadAttValFile(filename)) == NULL) {
		FAIL(__FILE__, __LINE__, "Failed reading list\n");
		return 0;
	}

	/** look up each one */
	if ((value = getAttVal(list, sData[0].attribute_)) == NULL)
		FAIL(__FILE__, __LINE__,
					"Cannot find attribute 0 '%s'\n", sData[0].attribute_);

	if ((value = getAttVal(list, sData[1].attribute_)) == NULL)
		FAIL(__FILE__, __LINE__,
					"Cannot find attribute 1 '%s'\n", sData[1].attribute_);

	if ((value = getAttVal(list, sData[2].attribute_)) == NULL)
		FAIL(__FILE__, __LINE__,
					"Cannot find attribute 2 '%s'\n", sData[2].attribute_);

	if ((value = getAttVal(list, sData[3].attribute_)) == NULL)
		FAIL(__FILE__, __LINE__,
					"Cannot find attribute 3 '%s'\n", sData[3].attribute_);


	/** now iterate through the list comparing the data */
	value = list->list_[0];
	if (CHECK_MATCH(sData[0], value))
		PASS(__FILE__, __LINE__, "match for (int) data element 0\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (int) data element 0\n");
		status = 0;
	}

	value = list->list_[1];
	if (CHECK_MATCH(sData[1], value))
		PASS(__FILE__, __LINE__, "match for (int) data element 1\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (int) data element 1\n");
		status = 0;
	}

	value = list->list_[2];
	if (CHECK_MATCH(sData[2], value))
		PASS(__FILE__, __LINE__, "match for (int) data element 2\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (int) data element 2\n");
		status = 0;
	}

	value = list->list_[3];
	if (CHECK_MATCH(sData[3], value))
		PASS(__FILE__, __LINE__, "match for (int) data element 3\n");
	else {
		FAIL(__FILE__, __LINE__, "mismatch for (int) data element 3\n");
		status = 0;
	}

	return(status);
}


int
testSimple(argc, argv)
	int argc;
	char **argv;
{
	initData();

	if (testWrite(TESTFILE) <= 0)
	{
		FAIL(__FILE__, __LINE__, "Writing failed\n");
		return -1;
	}

	if (testRead(TESTFILE) <= 0)
	{
		FAIL(__FILE__, __LINE__, "Reading failed\n");
		return -1;
	}

	return 1;
}

