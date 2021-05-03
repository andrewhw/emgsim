/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: testTokenizer.c 12 2008-04-24 22:22:26Z andrew $
 **/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os_defs.h"

# ifndef	OS_WINDOWS
# include <unistd.h>
# endif

#include "tclCkalloc.h"
#include "filetools.h"
#include "stringtools.h"
#include "tokens.h"
#include "massert.h"

#include "testutils.h"

const char *sData[] = {
		"\n",
		"#this is a comment\n",
		"#\n",
		"x = 12 * 1.5;\n",
		"",
		"test() {}\n",
		NULL
	};


static int
testSimpleTokenizer(tokenizer *t)
{
	token *tok;
	char tokenPrintBuffer[BUFSIZ];

	
	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected ID, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_IDENTIFIER)
	{
		PASS(__FILE__, __LINE__, "found id\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected id, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected =, got NULL\n");
		return 0;
	}
	if (tok->type_ == '=')
	{
		PASS(__FILE__, __LINE__, "found =\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected =, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected int, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_INTEGER)
	{
		PASS(__FILE__, __LINE__, "found int\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected int, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected *, got NULL\n");
		return 0;
	}
	if (tok->type_ == '*')
	{
		PASS(__FILE__, __LINE__, "found *\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected *, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected real, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_REAL)
	{
		PASS(__FILE__, __LINE__, "found real\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected real, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected ;, got NULL\n");
		return 0;
	}
	if (tok->type_ == ';')
	{
		PASS(__FILE__, __LINE__, "found ;\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected ;, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected ID, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_IDENTIFIER)
	{
		PASS(__FILE__, __LINE__, "found id\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected id, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected '(', got NULL\n");
		return 0;
	}
	if (tok->type_ == '(')
	{
		PASS(__FILE__, __LINE__, "found '('\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected '(', found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected ')', got NULL\n");
		return 0;
	}
	if (tok->type_ == ')')
	{
		PASS(__FILE__, __LINE__, "found ')'\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected ')', found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected '{', got NULL\n");
		return 0;
	}
	if (tok->type_ == '{')
	{
		PASS(__FILE__, __LINE__, "found '{'\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected '{', found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected '}', got NULL\n");
		return 0;
	}
	if (tok->type_ == '}')
	{
		PASS(__FILE__, __LINE__, "found '}'\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected '}', found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		PASS(__FILE__, __LINE__, "found EOF\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected EOF, found %d\n", tok->type_);
	}

	return 1;
}

int
testFromFile()
{
	FILE *fp;
	tokenizer *t;
	char *filename;
	int i, status;


	filename = allocTempFileName("Tok");

	fp = fopen(filename, "w");
	if (fp == NULL)
	{
		FAIL(__FILE__, __LINE__,
				"cannot open tmp file '%s'\n", filename);
		return 0;
	}

	for (i = 0; sData[i] != NULL; i++)
	{
		fputs(sData[i], fp);
	}

	fclose(fp);


	fp = fopen(filename, "r");
	t = tknGetFileTokenizer(fp);
	status = testSimpleTokenizer(t);
	tknDeleteTokenizer(t);
	fclose(fp);

	unlink(filename);
	ckfree(filename);

	return status;
}

int
testFromString()
{
	tokenizer *t;
	char *string;
	int i, status, len;

	len = 0;
	for (i = 0; sData[i] != NULL; i++)
	{
		len += strlen(sData[i]);
	}
	len++;

	string = (char *) ckalloc(len);

	string[0] = 0;
	for (i = 0; sData[i] != NULL; i++)
	{
		strlcat(string, sData[i], len);
	}

	t = tknGetStringTokenizer(string);
	status = testSimpleTokenizer(t);
	tknDeleteTokenizer(t);

	ckfree(string);

	return status;
}


int
testFromStringList()
{
	tokenizer *t;
	int status;

	t = tknGetStrListTokenizer(sData);
	status = testSimpleTokenizer(t);
	tknDeleteTokenizer(t);

	return status;
}


int
testWithComments()
{
	tokenizer *t;
	token *tok;
	char tokenPrintBuffer[BUFSIZ];
	char *data = "\nx#comment\ny";


	t = tknGetStringTokenizer(data);

	tknSetOptions(t, TTOPT_RETURN_COMMENTS | TTOPT_RETURN_CR);
	

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected CR, got NULL\n");
		return 0;
	}
	if (tok->type_ == '\n')
	{
		PASS(__FILE__, __LINE__, "found CR\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected CR, found %d\n", tok->type_);
	}


	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected id, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_IDENTIFIER)
	{
		PASS(__FILE__, __LINE__, "found id\n");
		if (strcmp(tok->data_.strptr_, "x") == 0)
		{
			PASS(__FILE__, __LINE__, "found x data\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
					"expected \"x\", found \"%s\"\n",
					tok->data_.strptr_);
		}

	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected id, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected comment, got NULL\n");
		return 0;
	}
	if (tok->type_ == '#')
	{
		PASS(__FILE__, __LINE__, "found comment\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected comment, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected id, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_IDENTIFIER)
	{
		PASS(__FILE__, __LINE__, "found id\n");

		if (strcmp(tok->data_.strptr_, "comment") == 0)
		{
			PASS(__FILE__, __LINE__, "found comment data\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
					"expected \"comment\", found \"%s\"\n",
					tok->data_.strptr_);
		}

	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected id, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected CR, got NULL\n");
		return 0;
	}
	if (tok->type_ == '\n')
	{
		PASS(__FILE__, __LINE__, "found CR\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected CR, found %d\n", tok->type_);
	}

	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		FAIL(__FILE__, __LINE__, "expected id, got NULL\n");
		return 0;
	}
	if (tok->type_ == TT_IDENTIFIER)
	{
		PASS(__FILE__, __LINE__, "found id\n");
		if (strcmp(tok->data_.strptr_, "y") == 0)
		{
			PASS(__FILE__, __LINE__, "found y data\n");
		} else
		{
			FAIL(__FILE__, __LINE__,
					"expected \"y\", found \"%s\"\n",
					tok->data_.strptr_);
		}

	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected id, found %d\n", tok->type_);
	}


	tok = tknGetToken(t);
	tknExpandToken(tokenPrintBuffer, BUFSIZ, t);
	DBG(__FILE__, __LINE__, "%s\n", tokenPrintBuffer);
	if (tok == NULL)
	{
		PASS(__FILE__, __LINE__, "found EOF\n");
	} else
	{
		FAIL(__FILE__, __LINE__,
			"expected EOF, found %d\n", tok->type_);
	}


	tknDeleteTokenizer(t);
	return 1;
}

int
testTokenizer(int argc, char **argv)
{
	int status = 1;

	status = testFromFile();
	status = testFromString() && status;
	status = testFromStringList() && status;

	status = testWithComments() && status;

	return status;
}

