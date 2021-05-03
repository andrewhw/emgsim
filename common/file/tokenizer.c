/**
 * ------------------------------------------------------------
 * Parse up input data to produce a series of c-like tokens
 * ------------------------------------------------------------
 * $Id: tokenizer.c 89 2011-11-17 22:55:45Z andrew $
 */

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#include        <ctype.h>
#endif

#include        "tclCkalloc.h"
#include        "attvalfile.h"
#include        "stringtools.h"
#include        "tokens.h"
#include        "massert.h"
#include        "log.h"
#include        "os_defs.h"


static int      rToken(tokenizer * t);
static int      literalCh(tokenizer * t, int ch);

OS_EXPORT int
tknGetLineNo(tokenizer * t)
{
	return t->lineNo_ + 1;
}


OS_EXPORT void
tknSetOptions(tokenizer * t, int flags)
{
	t->options_ = flags;
}

OS_EXPORT int
tknGetOptions(tokenizer * t)
{
	return t->options_;
}

OS_EXPORT void
tknReset(tokenizer * t)
{
	memset(t, 0, sizeof(tokenizer));

	t->saveCh_ = (-1);
	t->options_ = 0;
	t->token_.data_.strptr_ = NULL;
	t->verbose_.echoFlush_ = 0;
	t->verbose_.sawNL_ = 0;
}

OS_EXPORT void
tknFileReset(tokenizer * t, FILE * ifp)
{
	tknReset(t);

	MSG_ASSERT(t->type_ == TKNIZER_TYPE_FILE, "type mismatch");
	t->typeData_.file_.ifp_ = ifp;
}


OS_EXPORT void
tknStringReset(tokenizer * t, const char *data)
{
	tknReset(t);

	MSG_ASSERT(t->type_ == TKNIZER_TYPE_STRING, "type mismatch");
	if (t->typeData_.string_.data_ != NULL)
		ckfree(t->typeData_.string_.data_);

	if (data != NULL)
		t->typeData_.string_.data_ = ckstrdup(data);
	else
		t->typeData_.string_.data_ = NULL;

	t->typeData_.string_.offset_ = 0;
}


OS_EXPORT void
tknStrListReset(tokenizer * t, const char ** newData)
{
	tknReset(t);

	MSG_ASSERT(t->type_ == TKNIZER_TYPE_STRLIST, "type mismatch");
	if (newData != NULL)
		t->typeData_.strList_.data_ = newData;
	else
		t->typeData_.strList_.data_ = NULL;

	t->typeData_.strList_.lineOffset_ = 0;
	t->typeData_.strList_.charOffset_ = 0;
}

OS_EXPORT tokenizer *
tknGetTokenizer(FILE * ifp)
{
	return tknGetFileTokenizer(ifp);
}

OS_EXPORT tokenizer *
tknGetFileTokenizer(FILE * ifp)
{
	tokenizer      *t;

	t = (tokenizer *) ckalloc(sizeof(tokenizer));
	t->token_.data_.strptr_ = NULL;

	tknReset(t);
	t->type_ = TKNIZER_TYPE_FILE;

	t->typeData_.file_.ifp_ = ifp;

	return t;
}


OS_EXPORT tokenizer *
tknGetStringTokenizer(const char *string)
{
	tokenizer      *t;

	t = (tokenizer *) ckalloc(sizeof(tokenizer));
	t->token_.data_.strptr_ = NULL;

	tknReset(t);
	t->type_ = TKNIZER_TYPE_STRING;

	t->typeData_.string_.data_ = ckstrdup(string);
	t->typeData_.string_.offset_ = 0;

	return t;
}

OS_EXPORT tokenizer *
tknGetStrListTokenizer(const char **stringList)
{
	tokenizer      *t;

	t = (tokenizer *) ckalloc(sizeof(tokenizer));
	t->token_.data_.strptr_ = NULL;

	tknReset(t);
	t->type_ = TKNIZER_TYPE_STRLIST;

	t->typeData_.strList_.data_ = stringList;
	t->typeData_.strList_.lineOffset_ = 0;
	t->typeData_.strList_.charOffset_ = 0;

	return t;
}

OS_EXPORT void
tknDeleteTokenizer(tokenizer * t)
{
	if (t == NULL)
		return;

	if (t->type_ == TKNIZER_TYPE_FILE)
	{
		t->typeData_.file_.ifp_ = NULL;

	} else if (t->type_ == TKNIZER_TYPE_STRING)
	{
		if (t->typeData_.string_.data_ != NULL)
			ckfree(t->typeData_.string_.data_);
		t->typeData_.string_.data_ = NULL;
		t->typeData_.string_.offset_ = 0;

	} else if (t->type_ == TKNIZER_TYPE_STRLIST)
	{
		t->typeData_.strList_.data_ = NULL;
		t->typeData_.strList_.lineOffset_ = 0;
		t->typeData_.strList_.charOffset_ = 0;
	} else {
		MSG_FAIL("Unknown tokenizer type in delete!");
	}
	tknReset(t);
	ckfree(t);
}

/*
 * --------------------------------------------
 * return the next token
 * ---------------------------------------------
 */
OS_EXPORT token *
tknGetToken(tokenizer * t)
{
	if (t->pushedToken_)
	{
		t->pushedToken_ = 0;
		return &t->token_;
	}
	if ((t->options_ & TTOPT_VERBOSE_PARSE) != 0)
	{
		if (t->verbose_.sawNL_ == 1)
		{
			fprintf(stderr, "tkn<line %d[", tknGetLineNo(t));
			t->verbose_.sawNL_ = 0;
		} else
		{
			fprintf(stderr, "tkn<[");
		}
	}
	t->token_.type_ = rToken(t);
	if (t->verbose_.echoFlush_)
	{
		fputs("]tkn<\n", stderr);
	}
	if (t->token_.type_ == TT_EOF)
		return NULL;

	return &t->token_;
}

/**
 * Push back the last token
 */
OS_EXPORT void
tknPushToken(tokenizer * t)
{
	t->pushedToken_ = 1;
}

OS_EXPORT int
tknExpect(tokenizer * t, const char *s)
{
	token          *tok;

	tok = tknGetToken(t);
	if (tok == NULL)
		return (-1);

	if (tok->type_ <= TT_MAX_LITERAL)
	{
		if (s[0] == tok->type_)
		{
			if (strlen(s) > 1)
			{
				return (tknExpect(t, &s[1]));
			}
			return 1;
		}
		return 0;
	}
	if (tok->type_ == TT_CHARACTER)
	{
		if (s[0] == (char) tok->data_.ival_)
		{
			if (strlen(s) > 1)
			{
				return (tknExpect(t, &s[1]));
			}
			return 1;
		}
		return 0;
	}
	return (tknCompare(tok, s) == 0);
}

OS_EXPORT int
tknCompare(token * tok, const char *s)
{
	return (strcmp(tok->data_.strptr_, s));
}

OS_EXPORT int
tknCompareNoCase(token * tok, const char *s)
{
	char           *lowertoken;
	char           *lowercomp;
	int             result;

	if ((tok->type_ != TT_STRING) && (tok->type_ != TT_IDENTIFIER))
		return 1;

	lowertoken = ckstrdup(tok->data_.strptr_);
	strtolower(lowertoken);

	lowercomp = ckstrdup(s);
	strtolower(lowercomp);

	result = strcmp(lowertoken, lowercomp);

	ckfree(lowertoken);
	ckfree(lowercomp);

	return result;
}

OS_EXPORT int
tknIsDelim(token * tok, const char *delimset)
{
	int             i, len;

	len = strlen(delimset);
	for (i = 0; i < len; i++)
	{
		if (tok->type_ == delimset[i])
			return 1;
	}

	return 0;
}

int
getAChar(tokenizer * t)
{
	int c = 0;

	if (t->saveCh_ >= 0)
	{
		c = t->saveCh_;
		if (t->saveCh_ != TT_EOF)
			t->saveCh_ = (-1);
		return c;
	}

	if (t->type_ == TKNIZER_TYPE_FILE)
	{
		c = fgetc(t->typeData_.file_.ifp_);
	} else if (t->type_ == TKNIZER_TYPE_STRING)
	{
		c = t->typeData_.string_.data_[
				t->typeData_.string_.offset_++
			];
		if (c == 0)
			c = EOF;

	} else if (t->type_ == TKNIZER_TYPE_STRLIST)
	{
		c = t->typeData_.strList_.data_[
				t->typeData_.strList_.lineOffset_
			][
				t->typeData_.strList_.charOffset_++
			];
		while (c == 0)
		{
			/** check for data on the next line */
			if (t->typeData_.strList_.data_[
							++t->typeData_.strList_.lineOffset_
						] == NULL)
			{
				/** end of file, leave the loop */
				c = EOF;
				break;
			} else
			{
				/** reset counter within this line */
				t->typeData_.strList_.charOffset_ = 0;
			}

			/*
			 * get the next character; if this is a zero-length
			 * string, this will be null and the loop will advance
			 * to the next string, or end of file if no next string
			 */
			c = t->typeData_.strList_.data_[
						t->typeData_.strList_.lineOffset_
					][
						t->typeData_.strList_.charOffset_++
					];
		}

	} else {
			MSG_FAIL("Bad tokenizer type in getAChar()");
	}

	if (c == '\n')
		t->lineNo_++;

	if (c == EOF)
	{
		t->saveCh_ = TT_EOF;
		return TT_EOF;
	}
	if ((t->options_ & TTOPT_VERBOSE_PARSE) != 0)
	{
		fputs(chunctrl(c), stderr);
		t->verbose_.echoFlush_ = 1;
		if (c == '\n')
			t->verbose_.sawNL_ = 1;
	}
	return c;
}

int
skipSpaces(tokenizer * t)
{
	int c;

	while ((c = getAChar(t)) != TT_EOF)
	{
		if ((c == '\n') && ((t->options_ & TTOPT_RETURN_CR) != 0))
			return c;

		if (!isspace(c))
		{
			return c;
		}
	}

	return TT_EOF;
}

/* getString - get a string */
int
getString(tokenizer * t)
	{
	int readch;
	int ch = 0;     /** if no chars are read, ch should not == EOF */

	if (t->tokenStringLen_ == 0)
	{
		t->token_.data_.strptr_ = t->tokenString_;
	}
	/* get the string */
	readch = getAChar(t);
	while ((readch != EOF) && (readch != '"') && (readch != '\n'))
	{

		ch = literalCh(t, readch);

		if (ch != 0)
		{
			t->tokenString_[t->tokenStringLen_++] = (char) ch;
		}
		readch = getAChar(t);
	}

	/** terminate the string */
	t->tokenString_[t->tokenStringLen_] = 0;

	if (readch == '\n')
	{
		LogError("Quoted string reaches end of line on line %d\n",
				 t->lineNo_);
		return (-1);
	}
	if (ch == EOF)
	{
		LogError("Quoted string reaches to end of file");
		t->saveCh_ = EOF;
		return (-1);
	}
	return (TT_STRING);
	}


/* getCharacter - get a character constant */
int
getCharacter(tokenizer * t)
{
	int ch;
	t->token_.data_.ival_ = literalCh(t, getAChar(t));

	if ((ch = getAChar(t)) != '\'')
	{
		LogError(
				 "Found more than one character in single-quote expression");
		return (-1);
	}
	t->saveCh_ = ch;
	return (TT_CHARACTER);
}

/* literalCh - get a character from a literal string */
static int
literalCh(tokenizer * t, int ch)
{
	if (ch == '\\')
	{
		ch = getAChar(t);
		if (isdigit(ch))
		{

			int             i = 0;
			int             octalVal = (ch - '0');

			while (i++ < 3)
			{
				ch = getAChar(t);
				if (!isdigit(ch))
				{
					t->saveCh_ = ch;
					break;
				}
				octalVal = (octalVal * 8) + (ch - '0');
			}
			ch = octalVal;

		} else
		{
			switch (ch)
			{
			case 'n':
				ch = '\n';
				break;
			case 't':
				ch = '\t';
				break;
			case 'b':
				ch = '\b';
				break;
			case 'r':
				ch = '\r';
				break;
			case 'f':
				ch = '\f';
				break;
				/* case '\n':  ch = 0; break; */
			case TT_EOF:
				ch = '\\';
				t->saveCh_ = TT_EOF;
				break;
			}
		}
	}
	return (ch);
}

OS_EXPORT int 
tknIsIdChar(int ch)
{
	return (isalpha(ch) || isdigit(ch) || ch == '_');
}

int
getNumber(tokenizer * t, int ch)
{
	char           *endp;
	int             sign = 1;
	int             returnType = TT_INTEGER;

	if (ch == '-')
	{
		sign = (-1);
		returnType = ch;
	}
	if (ch == '.' || ch == '+')
	{
		returnType = ch;
	}
	if (ch != '.')
	{

		/* if we got a '.', we will handle it in the float routines */
		if (t->tokenStringLen_ == 0)
		{
			t->token_.data_.strptr_ = t->tokenString_;
		}
		t->tokenString_[t->tokenStringLen_++] = (char) ch;
		if (ch != '-' && ch != '+')
		{
			returnType = TT_INTEGER;
			t->token_.data_.ival_ = (ch - '0');
		} else
		{
			t->token_.data_.ival_ = 0;
		}


		while ((ch = getAChar(t)) != TT_EOF && isdigit(ch))
		{
			returnType = TT_INTEGER;
			t->token_.data_.ival_ =
				t->token_.data_.ival_ * 10 + (ch - '0');
			t->tokenString_[t->tokenStringLen_++] = (char) ch;
		}
		t->token_.data_.ival_ = t->token_.data_.ival_ * sign;
	}
	t->tokenString_[t->tokenStringLen_] = 0;
	/*
	 * terminate in case it is not a float
	 * t->saveCh_ is not set until after we
	 * test for a float, as that would
	 * screw up our test
	 */

	/* check if we are actually parsing a float */
	if (ch == '.')
	{
		t->tokenString_[t->tokenStringLen_++] = (char) ch;

		while ((ch = getAChar(t)) != TT_EOF && isdigit(ch))
		{
			t->tokenString_[t->tokenStringLen_++] = (char) ch;
		}
		t->tokenString_[t->tokenStringLen_] = 0;
		t->saveCh_ = ch;

		/* get the value out of the string */
		t->token_.data_.dval_ = strtod(t->tokenString_, &endp);
		returnType = TT_REAL;
	} else
	{
		t->saveCh_ = ch;
	}


	/*
	 * check if we are actually parsing an exponent,
	 * and if so, replace the value of the double calculated above
	 */
	ch = getAChar(t);
	if (ch == 'e')
	{

		t->tokenString_[t->tokenStringLen_++] = (char) ch;
		t->tokenString_[t->tokenStringLen_] = 0;

		/** parse and eat a possible sign */
		ch = getAChar(t);
		if (ch == '+' || ch == '-')
		{
			t->tokenString_[t->tokenStringLen_++] = (char) ch;
			t->tokenString_[t->tokenStringLen_] = 0;

		} else
		{
			t->saveCh_ = ch;
		}

		/** handle the value of the exponent */
		while ((ch = getAChar(t)) != TT_EOF && isdigit(ch))
		{
			t->tokenString_[t->tokenStringLen_++] = (char) ch;
		}
		t->tokenString_[t->tokenStringLen_] = 0;
		t->saveCh_ = ch;

		/* get the value out of the string */
		t->token_.data_.dval_ = strtod(t->tokenString_, &endp);
		return (TT_REAL);
	} else
	{
		t->saveCh_ = ch;
	}

	return (returnType);
}

int
getIdentifier(tokenizer * t, int ch)
{
	if (t->tokenStringLen_ == 0)
	{
		t->token_.data_.strptr_ = t->tokenString_;
	}
	t->tokenString_[t->tokenStringLen_++] = (char) ch;

	while ((ch = (int) getAChar(t)) != TT_EOF && tknIsIdChar(ch))
		t->tokenString_[t->tokenStringLen_++] = (char) ch;
	t->saveCh_ = ch;
	t->tokenString_[t->tokenStringLen_] = 0;

	return (TT_IDENTIFIER);
}

static int
rToken(tokenizer * t)
{
	int ch;

	t->tokenStringLen_ = 0;
	memset(&t->token_, 0, sizeof(token));


	/* check the next character */
	for (;;)
	{

		switch (ch = skipSpaces(t))
		{

		case TT_EOF:
			return (TT_EOF);

		case '"':
			return (getString(t));

		case '\'':
			return (getCharacter(t));

		case '=':
			return ('=');

		case ';':
			return (';');

		case '#':
			if ((t->options_ & TTOPT_RETURN_COMMENTS) != 0)
			{
				return ch;
			}
			while ((ch = getAChar(t)) != TT_EOF)
			{
				if (ch == '\n')
					break;
			}
			if (ch == TT_EOF)
				t->saveCh_ = TT_EOF;
			break;

		case '+':
		case '-':
			return (getNumber(t, ch));
			OS_BREAK;

		case '.':
			/*
			 * test for a number in the form ".5"
			 * if this _is_ what we have, push
			 * the digit back into the stream,
			 * and seed getNumber with a dot
			 */
			ch = getAChar(t);
			t->saveCh_ = ch;
			if (isdigit(ch))
			{
				return (getNumber(t, '.'));
			}
			return ('.');
			OS_BREAK;


		default:
			if (isdigit(ch))
			{
				return (getNumber(t, ch));

			} else if (tknIsIdChar(ch))
			{
				/**
				 ** getIdentifier returns (-1) on bad token, so
				 ** the 'bad value' can just fall through
				 **/
				return (getIdentifier(t, ch));

			} else
			{
				return (ch);
			}
		}
	}

	/* NOTREACHED */
}


int
tknExpandToken(char *buffer, int size, tokenizer * t)
{
	char *load;
	int len, used, remain;

	used = 0;
	remain = size;
	load = buffer;


	slnprintf(load, size, "Tok: ");
	len = strlen(load);
	used += len;
	remain -= len;
	load = &buffer[used];


	if (t->token_.type_ <= TT_MAX_LITERAL)
	{
		slnprintf(load, remain,
				"'%s' (%d)",
				chunctrl(t->token_.type_),
				t->token_.type_);
		len = strlen(load);
		used += len;
		remain -= len;
		load = &buffer[used];

	} else if (t->token_.type_ <= TT_MAX__)
	{
		char *tag = "error";
		switch(t->token_.type_)
		{
		case TT_STRING:
			tag = "<string>"; break;
		case TT_CHARACTER:
			tag = "<char>"; break;
		case TT_INTEGER:
			tag = "<integer>"; break;
		case TT_REAL:
			tag = "<real>"; break;
		case TT_IDENTIFIER:
			tag = "<identifier>"; break;
		case TT_UNKNOWN:
			tag = "<unknown>"; break;
		case TT_EOF:
			tag = "<end-of-file>"; break;
		case TT_MAX__:
			tag = "<max token>"; break;
		}
		slnprintf(load, remain, "'%s' (%d)", tag, t->token_.type_);
		len = strlen(load);
		used += len;
		remain -= len;
		load = &buffer[used];
	} else
	{
		slnprintf(load, remain, "(unknown:%d)", t->token_.type_);
		len = strlen(load);
		used += len;
		remain -= len;
		load = &buffer[used];
	}

	if (t->token_.type_ == TT_STRING
			|| t->token_.type_ == TT_IDENTIFIER)
	{
		slnprintf(load, remain, " [%s]",
			strunctrl(t->token_.data_.strptr_,
				strlen(t->token_.data_.strptr_)));
		len = strlen(load);
		used += len;
		remain -= len;
		load = &buffer[used];

	} else if (t->token_.type_ == TT_REAL)
	{
		slnprintf(load, remain, " R:[%f]", t->token_.data_.dval_);
		len = strlen(load);
		used += len;
		remain -= len;
		load = &buffer[used];

	} else if (t->token_.type_ == TT_INTEGER)
	{
		slnprintf(load, remain, " I:[%d]", t->token_.data_.ival_);
		len = strlen(load);
		used += len;
		remain -= len;
		load = &buffer[used];
	}

	return used >= 0 ? used : -1;
}


int
tknPrintToken(FILE *fp, tokenizer * t)
{
	char buffer[BUFSIZ*4];

	tknExpandToken(buffer, BUFSIZ * 4, t);

	fprintf(fp, "%s\n", buffer);

	return ferror(fp);
}


int
tknDumpToken(tokenizer * t)
{
	return tknPrintToken(stdout, t);
}
