/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: attvalue.c 98 2012-05-09 13:37:45Z andrew $
 **/

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#endif

#include "os_defs.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#include        "tclCkalloc.h"
#include        "listalloc.h"
#include        "attvalfile.h"
#include        "tokens.h"
#include        "stringtools.h"

#include        "log.h"

#define         ATTVAL_BLOCKSIZE        8


/** forward declarations */
static int      getAttValIndex(attValList * list, const char *key);

/*
 * ---------------------------------------------
 * Open the given file, parsing it for attribute
 * value pairs, loading everything up int an
 * attval_list structure
 * ---------------------------------------------
 */
OS_EXPORT attValList *
loadAttValFile(const char *filename)
{
	FILE           *fp;
	attValList     *list;

	fp = fopen(filename, "rb");

	if (fp == NULL)
		return NULL;

	list = parseAttValFile(fp);

	fclose(fp);

	return list;
}


/*
 * ---------------------------------------------
 * parsing the file for attribute
 * value pairs, loading everything up int an
 * attValList structure
 * ---------------------------------------------
 */
OS_EXPORT attValList *
parseAttValFile(FILE * ifp)
{
	attValList     *list;
	attVal         *attValPair;
	tokenizer      *t;

	t = tknGetTokenizer(ifp);


	list = (attValList *) ckalloc(sizeof(attValList));
	if (list == NULL)
		return NULL;
	memset(list, 0, sizeof(attValList));


	while ((attValPair = parseAttValFromTokenizer(t)) != NULL)
	{

		/** grow the list of values */
		listMkCheckSize(list->size_ + 1,
						(void **) &list->list_,
						&list->curBlocks_,
						ATTVAL_BLOCKSIZE,
						sizeof(attVal *),
						__FILE__, __LINE__);
		list->list_[list->size_++] = attValPair;
	}
	tknDeleteTokenizer(t);

	return list;

}

OS_EXPORT attVal *
createStringAttribute(const char *tag, const char *value)
{
	attVal         *result;

	result = (attVal *) ckalloc(sizeof(attVal));
	memset(result, 0, sizeof(attVal));

	result->type_ = TT_STRING;
	result->attribute_ = ckstrdup(tag);
	result->data_.strptr_ = ckstrdup(value);

	return result;
}


OS_EXPORT attVal *
createIntegerAttribute(const char *tag, int value)
{
	attVal         *result;

	result = (attVal *) ckalloc(sizeof(attVal));
	memset(result, 0, sizeof(attVal));

	result->type_ = TT_INTEGER;
	result->attribute_ = ckstrdup(tag);
	result->data_.ival_ = value;

	return result;
}

OS_EXPORT attVal *
createRealAttribute(const char *tag, double value)
{
	attVal         *result;

	result = (attVal *) ckalloc(sizeof(attVal));
	memset(result, 0, sizeof(attVal));

	result->type_ = TT_REAL;
	result->attribute_ = ckstrdup(tag);
	result->data_.dval_ = value;

	return result;
}

/*
 * ---------------------------------------------
 * add an attribute value pair to a list
 * ---------------------------------------------
 */
OS_EXPORT int
addAttVal(attValList * list, attVal * data)
{
	if (getAttVal(list, data->attribute_) != NULL)
	{
		LogError("Duplicate attribute '%s'\n", data->attribute_);
		return 0;
	}
	/** grow the list of values */
	listMkCheckSize(list->size_ + 1,
					(void **) &list->list_,
					&list->curBlocks_,
					ATTVAL_BLOCKSIZE,
					sizeof(attVal *),
					__FILE__, __LINE__);
	list->list_[list->size_++] = data;

	return 1;
}

/*
 * ---------------------------------------------
 * update or add an attribute value pair to a list
 * ---------------------------------------------
 */
OS_EXPORT int
updateAttVal(attValList * list, attVal * data)
{
	int             listIndex;
	attVal         *oldListItem;

	/**
	 * if we have an item in the list, replace it with our new item,
	 * and delete the old entry from the list
	 */
	if ((listIndex = getAttValIndex(list, data->attribute_)) >= 0)
	{
		oldListItem = list->list_[listIndex];
		list->list_[listIndex] = data;
		deleteAttVal(oldListItem);
		return 1;
	}
	/** otherwise, grow the list of values */
	listMkCheckSize(list->size_ + 1,
					(void **) &list->list_,
					&list->curBlocks_,
					ATTVAL_BLOCKSIZE,
					sizeof(attVal *),
					__FILE__, __LINE__);
	list->list_[list->size_++] = data;

	return 1;
}

#define EXPECT_IDENTIFIER       1
#define EXPECT_VALUE            2
#define EXPECT_EQUALS           3
#define EXPECT_DELIMETER        4

/*
 * ---------------------------------------------
 * load up the stuff on this line into a attVal
 * pair
 * ---------------------------------------------
 */
OS_EXPORT attVal *
parseAttVal(FILE * ifp)
{
	attVal         *result;
	tokenizer      *t;

	t = tknGetTokenizer(ifp);
	result = parseAttValFromTokenizer(t);
	tknDeleteTokenizer(t);

	return result;
}


/*
 * ---------------------------------------------
 * load up the stuff on this line into a attVal
 * pair
 * ---------------------------------------------
 */
OS_EXPORT attVal *
parseAttValFromTokenizer(tokenizer * t)
{
	attVal         *result;
	token          *token;
	int             state = EXPECT_IDENTIFIER;

	result = (attVal *) ckalloc(sizeof(attVal));
	memset(result, 0, sizeof(attVal));

	while (1)
	{
		token = tknGetToken(t);

		if (token == NULL)
		{
			break;
		}
		if (token->type_ == TT_EOF)
			break;

		/** if we are done, get out of here */
		if ((state == EXPECT_DELIMETER) && (token->type_ == ';'))
		{
			return result;
		}
		if (state == EXPECT_IDENTIFIER &&
			(token->type_ == TT_IDENTIFIER || token->type_ == TT_STRING))
		{

			result->attribute_ = ckstrdup(token->data_.strptr_);
			state = EXPECT_EQUALS;
			continue;

		} else if (state == EXPECT_EQUALS && token->type_ == '=')
		{
			state = EXPECT_VALUE;

		} else if (state == EXPECT_VALUE)
		{
			result->type_ = token->type_;
			if (token->type_ == TT_STRING)
			{
				result->data_.strptr_ = ckstrdup(token->data_.strptr_);

			} else if (token->type_ == TT_INTEGER)
			{
				result->data_.ival_ = token->data_.ival_;

			} else if (token->type_ == TT_REAL)
			{
				result->data_.dval_ = token->data_.dval_;
			}
			state = EXPECT_DELIMETER;

		} else
		{
			printf("Unknown data in file on line %d\n",
				   tknGetLineNo(t));
			goto FAIL;
			break;
		}
	}

FAIL:
	/** clean up and bail */
	if (result->attribute_ != NULL)
		ckfree((void *) result->attribute_);
	ckfree(result);

	return NULL;
}

static int
getAttValIndex(attValList * list, const char *key)
{
	int             i;

	for (i = 0; i < list->size_; i++)
	{
		if (strcmp(list->list_[i]->attribute_, key) == 0)
			return i;
	}

	return (-1);
}

/*
 * ---------------------------------------------
 * find an attribute in the list
 * ---------------------------------------------
 */
OS_EXPORT attVal *
getAttVal(attValList * list, const char *key)
{
	int             index;

	index = getAttValIndex(list, key);
	if (index < 0)
		return 0;
	return list->list_[index];
}

static int
sDoesAttributeNeedQuoting(const char *name)
{
	while (*name)
	{
		if (! tknIsIdChar(*name++))
			return 1;
	}

	return 0;
}

/*
 * Process the string, writing to the file pointer if not null
 * and returning the length (that would be) written.
 */
static int
sQuoteMangleWrite(FILE *ofp, const char *s)
{
	int dataLen, outLen = 0;
	int i;

	dataLen = strlen(s);

	/* handle leading quote */
	if (ofp != NULL) putc('"', ofp);
	outLen++;

	/* handle string, escaping special characters */
	for (i = 0; i < dataLen; i++)
	{
		if (s[i] == '\n')
		{
			if (ofp != NULL) putc('\\', ofp);
			outLen++;

			if (ofp != NULL) putc('n', ofp);
			outLen++;

		} else if (s[i] == '\t')
		{
			if (ofp != NULL) putc('\\', ofp);
			outLen++;

			if (ofp != NULL) putc('t', ofp);
			outLen++;

		} else
		{
			if (s[i] == '\\' || s[i] == '"')
			{
				if (ofp != NULL) putc('\\', ofp);
				outLen++;
			}
			if (ofp != NULL) putc(s[i], ofp);
			outLen++;
		}
	}

	/* handle trailing quote */
	if (ofp != NULL) putc('"', ofp);
	outLen++;

	return outLen;
}

/*
 * ---------------------------------------------
 * write out the list to a file
 * ---------------------------------------------
 */
OS_EXPORT int
writeAttValList(FILE * ofp, attValList * list, const char *comment)
{
	attVal *item;
	char *namebuf;
	int maxTagLen = 0;
	int i, len;

	for (i = 0; i < list->size_; i++)
	{
		item = list->list_[i];
		len = strlen(item->attribute_);

		/** add in space for quotes */
		if (sDoesAttributeNeedQuoting(item->attribute_))
		{
			len = sQuoteMangleWrite(NULL, item->attribute_);
		}
		if (maxTagLen < len)
			maxTagLen = len;
	}

	namebuf = ckalloc(maxTagLen + 1);

	fprintf(ofp, "#\n");
	fprintf(ofp, "# %s\n", comment);
	fprintf(ofp, "#\n");
	for (i = 0; i < list->size_; i++)
	{
		item = list->list_[i];

		if (sDoesAttributeNeedQuoting(item->attribute_))
		{
			len = sQuoteMangleWrite(ofp, item->attribute_);
			if (len < maxTagLen)
				fprintf(ofp, "%*s", maxTagLen - len, "");
		} else
		{
			fprintf(ofp, "%*s", maxTagLen, item->attribute_);
		}

		/** the delimiter */
		fputs(" = ", ofp);

		/** now the data */
		if (item->type_ == TT_STRING)
		{
			sQuoteMangleWrite(ofp, item->data_.strptr_);

		} else if (item->type_ == TT_INTEGER)
		{
			fprintf(ofp, "%d", item->data_.ival_);

		} else if (item->type_ == TT_REAL)
		{
			fprintf(ofp, "%s", niceDouble(item->data_.dval_));

		} else
		{
			fprintf(ofp, "\"<unknown>\"");
		}

		/** end of line */ 
		fputs(";\n", ofp);
	}

	ckfree(namebuf);

	return 1;
}

/*
 * ---------------------------------------------
 * clean up a list
 * ---------------------------------------------
 */
OS_EXPORT void
deleteAttVal(attVal * item)
{
	if (item->type_ == TT_STRING)
	{
		if (item->data_.strptr_ != NULL)
			ckfree((void *) item->data_.strptr_);
	}
	ckfree((void *) item->attribute_);
	ckfree(item);
}


/*
 * ---------------------------------------------
 * clean up a list
 * ---------------------------------------------
 */
OS_EXPORT void
deleteAttValList(attValList * list)
{
	int             i;
	attVal         *item;

	if (list->list_ != NULL)
	{
		for (i = 0; i < list->size_; i++)
		{
			item = list->list_[i];
			deleteAttVal(item);
		}
		ckfree(list->list_);
	}
	ckfree(list);
}

/*
 * ---------------------------------------------
 * create an empty list
 * ---------------------------------------------
 */
OS_EXPORT attValList *
createAttValList()
{
	attValList * list;

	list = (attValList *) ckalloc(sizeof(attValList));
	if (list == NULL)
		return NULL;
	memset(list, 0, sizeof(attValList));

	return(list);
}

