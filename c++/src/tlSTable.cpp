/** const
 * Copyright (c) 2013
 * All rights reserved.
 *
 * This code is part of the reseach work of
 * Andrew Hamilton-Wright (andrewhw@ieee.org).
 *
 * ----------------------------------------------------------------
 *
 * Redistribution and use in source and binary forms, with or with-
 * out modification, are permitted provided that recognition of the
 * author as the original contributor is provided in any source or
 * documentation relating to this code.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WAR-
 * RANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL THE AUTHOR OR ANY ASSOCIATED INSTITUTION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PRO-
 * CUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * $Id: tlSTable.cpp 64 2017-02-05 21:09:46Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <stdarg.h>
# include <math.h>
#endif

#include "os_defs.h"

#ifdef OS_WINDOWS
		// disable _CRT_SECURE_NO_WARNINGS related flags for now,
		// as they completely break the POSIX interface, as we
		// will have to re-write wrappers for things like fopen
		// to make this work more gracefully
# pragma warning(disable : 4996)
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"
#include "massert.h"
#include "tokens.h"
#include "attvalfile.h"

#include "tlSrString.h"
#include "tlSParser.h"
#include "tlTable.h"
#include "tlTuple.h"
#include "tlSrValue.h"
#include "tlErrorManager.h"

#include "tlSTable.h"


int
tlSTable::loadTable(
		tlTable *dataTable,
		FILE *ifp,
		const char *delimiters,
		tlErrorManager *err,
		const char *filename,
		FILE *verboseFP
    )
{
	tokenizer *t;


	/** get a tokenizer, which we will use for this input */
	t = tknGetTokenizer(ifp);

	/**
	 * we want to see newlines, to flag that they are not like spaces
	 */
	tknSetOptions(t, TTOPT_RETURN_CR);

	if (dataTable->getNumRows() == 0)
	{
		if ( ! tlSTable::parseDataHeader(
					dataTable, t, filename, delimiters, err
				) )
		{
			goto FAIL;
		}
	} else
	{
		if ( ! tlSTable::verifyDataHeader(
					dataTable, t, filename, delimiters, err
				) )
		{
			goto FAIL;
		}
	}

	if ( ! tlSTable::parseDataLines(
				dataTable, t, filename, delimiters, err, verboseFP
			) )
		{
		goto FAIL;
	}

	tknDeleteTokenizer(t);
	return 1;


FAIL:
	if (err != NULL)
	{
		if (filename != NULL)
		{
			err->addError("Failed loading table from file '%s'", filename);
		} else
		{
			err->addError("Failed loading table");
		}
	}
	/** clean up and bail */
	tknDeleteTokenizer(t);
	return 0;
}


int
tlSTable::loadTable(
		tlTable *dataTable,
		const char *filename,
		const char *delimiters,
		tlErrorManager *err,
		FILE *verboseFP
    )
{
	FILE * fp;
	int status;

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		if (err != NULL)
		{
			err->addError("Cannot open input file '%s' : %s",
					filename,
					strerror(errno));
		}
		return 0;
	}

	status = loadTable(dataTable, fp, delimiters, err, filename, verboseFP);

	fclose(fp);
	return status;
}


int
tlSTable::verifyDataHeader(
		tlTable *data,
		tokenizer *t,
		const char *filename,
		const char *delimiters,
		tlErrorManager *err
    )
{
	token *token;
	int sawSomething = 0;
	int existingColumnIndex;
	int columnIndex = (-1);


	data->clearColumnLoadMappings();

	while (! sawSomething)
	{
		token = tknGetToken(t);

		/** if we didn't read anything, just return failure */
		if (token == NULL)
		{
			if (err != NULL)
				err->addError("There is no data in this file");
			return 0;
		}

		if (token->type_ == '\n')
		{
			continue;
		}
		sawSomething = 1;

		tknPushToken(t);
		if ((token->type_ != TT_IDENTIFIER) &&
					(token->type_ != TT_STRING))
		{
			return 1;
		}
	}

	while (1)
	{
		token = tknGetToken(t);

		/** if we see the end of the line, we are done */
		if (token->type_ == '\n')
		{
			return 1;
		}

		/**
		 * Use the columnIndex variable to indicate what
		 * column we are in; break if we see something
		 * other than a string value or a delimiter
		 */
		if ((token->type_ == TT_IDENTIFIER) || (token->type_ == TT_STRING))
		{
			columnIndex++;

		} else if ( tknIsDelim(token, delimiters) )
		{
			/** skip the add part and get the next token */
			continue;

		} else
		{
			err->addError(
					"Expected identifier in header line, in '%s'",
					filename == NULL ? "stdin" : filename);
			break;
		}


		/**
		 * If first time in, we want to load columns;
		 * if a later time in, we want to verify that
		 * the column names we see match what we
		 * had before
		 */
		existingColumnIndex =
					data->getColumnIndex(token->data_.strptr_);
		if (existingColumnIndex >= 0)
		{
			data->setColumnLoadMapping(
							existingColumnIndex,
						columnIndex
					);
		} else
		{
			err->addError(
					"Found unexpected column '%s' in data file '%s'",
					token->data_.strptr_,
					filename == NULL ? "stdin" : filename);
			return 0;
			break;
		}
	}

	return 0;
}


int
tlSTable::parseDataHeader(
		tlTable *data,
		tokenizer *t,
		const char *filename,
		const char *delimiters,
		tlErrorManager *err
    )
{
	token *token;
	int sawSomething = 0;
	int columnIndex = (-1);

	while (! sawSomething)
	{
		token = tknGetToken(t);

		/** if we didn't read anything, just return failure */
		if (token == NULL)
		{
			if (err != NULL)
				err->addError("There is no data in this file");
			return 0;
		}

		if (token->type_ == '\n')
		{
			continue;
		}
		sawSomething = 1;

		tknPushToken(t);
		if ((token->type_ != TT_IDENTIFIER) && (token->type_ != TT_STRING))
		{
			return 1;
		}
	}

	while (1)
	{
		token = tknGetToken(t);

		/** if we see the end of the line, we are done */
		if (token->type_ == '\n')
		{
				return 1;
		}

		/**
		 * Use the columnIndex variable to indicate what
		 * column we are in; break if we see something
		 * other than a string value or a delimiter
		 */
		if ((token->type_ == TT_IDENTIFIER) || (token->type_ == TT_STRING))
		{
			columnIndex++;

		} else if ( tknIsDelim(token, delimiters) )
		{
			/** skip the add part and get the next token */
			continue;

		} else
		{
			if (err != NULL)
				err->addError(
					"Expected identifier in header line, in '%s'",
					filename == NULL ? "stdin" : filename);
			break;
		}


		/**
		 * If first time in, we want to load columns;
		 * if a later time in, we want to verify that
		 * the column names we see match what we
		 * had before
		 */
		if (data->getNumRows() == 0)
		{
			data->addColumn(token->data_.strptr_);
		} else
		{
			if (tknCompareNoCase(token,
					data->getColumnName(
								columnIndex
							).getValue()) != 0)
			{
				if (err != NULL)
					err->addError(
						"Mismatch in column name '%s' vs '%s' in '%s'",
						token->data_.strptr_,
						data->getColumnName(columnIndex).getValue(),
						filename == NULL ? "stdin" : filename
					);
			}
		}
	}

	return 0;
}

int
tlSTable::parseSingleDataLine(
		tokenizer *t,
		tlTuple *tuple,
		const char *filename,
		const char *delimiters,
		tlErrorManager *err
    )
{
	token *token;

	tuple->clear();

	while (1)
	{

		token = tknGetToken(t);

		/** return on EOF */
		if (token == NULL)
		{
			if (tuple->getNumValues() == 0)
			{
				return 0;
			} else
			{
				if (err != NULL)
					err->addError(
						"Partial data at EOF on line %d of '%s'",
						tknGetLineNo(t),
						filename == NULL ? "stdin" : filename);
				return (-1);
			}

		}

		/** return on EOL */
		if (token->type_ == '\n')
			break;

		/** make sure the data is valid */
		if (token->type_ == TT_INTEGER)
		{
			tlSrValue val(token->data_.ival_);
			tuple->addValue(val);

		} else if (token->type_ == TT_REAL)
		{
			tlSrValue val((tlReal) token->data_.dval_);
			tuple->addValue(val);

		} else if (token->type_ == '?')
		{
			tuple->addNil();

		} else if (token->type_ == TT_STRING || token->type_ == TT_IDENTIFIER)
		{
			if (tknCompareNoCase(token, "NIL") == 0)
			{
				tuple->addNil();

			} else if (tknCompareNoCase(token, "???") == 0)
			{
				tuple->addNil();

			} else if (tknCompareNoCase(token, "infinity") == 0)
			{
				tuple->addValue((float) (-log((float)0)));

			} else if (tknCompareNoCase(token, "inf") == 0)
			{
				tuple->addValue((float) (-log((float)0)));

			} else
			{
				tlSrValue val(token->data_.strptr_);
				tuple->addValue(val);
			}

		} else if (! tknIsDelim(token, delimiters) )
		{
			if (err != NULL)
				err->addError("Bad data on line %d of '%s' . . .",
					tknGetLineNo(t),
					filename == NULL ? "stdin" : filename);
				err->addError("Found '%s' when expecting delimiter",
						token->data_.strptr_);
			return (-1);
		}
	}

	return 1;
}

int
tlSTable::parseAndReorgSingleDataLine(
		tlTuple *reorgTuple,
		tlTable *data,
		tokenizer *t,
		const char *filename,
		const char *delimiters,
		tlTuple *readTuple,
		tlErrorManager *err
    )
{
	int nColumns;
	int status;
	int mappingIndex;
	int i;

	nColumns = data->getNumColumns();
	status = parseSingleDataLine(
			t, readTuple, filename,
			delimiters,
			err
		);
	if (status == 0)
		return (-1);

	if (status < 0)
	{
		if (err != NULL)
			err->addError("Failure parsing data file '%s'",
					filename == NULL ? "stdin" : filename);
		return -2;
	}


	/** skip blank lines */
	if (readTuple->getNumValues() == 0)
		return 0;

	/** check the number of columns is correct */
	if (nColumns == 0)
	{
		nColumns = readTuple->getNumValues();
	} else
	{

		if (data->hasColumnLoadMappings())
		{
			reorgTuple->setNumValues(nColumns);
			for (i = 0; i < nColumns; i++)
			{
				mappingIndex = data->getColumnLoadMapping(i);

				if (mappingIndex < 0)
				{
					reorgTuple->setNil(i);
				} else
				{
					reorgTuple->setValue(i,
							readTuple->getValue(mappingIndex)
						);
				}
			}
		} else
		{
			reorgTuple->setValues( readTuple );
		}

		if (nColumns != reorgTuple->getNumValues())
		{
			if (err != NULL)
				err->addError(
							"Num data items bad at line %d in '%s'"
							"-- expected %d got %d",
							tknGetLineNo(t),
							filename == NULL ? "stdin" : filename,
							readTuple->getNumValues(),
							nColumns);
			return -2;
		}
	}

	return 1;
}


int
tlSTable::parseDataLines(
		tlTable *data,
		tokenizer *t,
		const char *filename,
		const char *delimiters,
		tlErrorManager *err,
		FILE *verboseFP 
    )
{
	tlTuple *workingTuple;
	tlTuple *newlyReadTuple;
	int status;
	int lineNo = 0;

	workingTuple = new tlTuple();
	MSG_ASSERT(workingTuple != NULL, "Out of memory");
	workingTuple->ref();

	newlyReadTuple = new tlTuple();
	MSG_ASSERT(newlyReadTuple != NULL, "Out of memory");
	newlyReadTuple->ref();

	while (1)
	{
		status = parseAndReorgSingleDataLine(
					newlyReadTuple,
					data, t, filename, delimiters, workingTuple,
					err);

		/** skip blanks and comments */
		if (status == 0)
			continue;

		/** if we have hit EOF, return */
		if (status == (-1))
			break;

		if (status < 0)
		{
			continue;
			//goto FAIL;
		}

		/**
		 * append the new row to the array
		 */
		data->addRow(newlyReadTuple);

		if (verboseFP != NULL)
		{
			if ((lineNo % 25000) == 0)
			{
				fprintf(verboseFP, " . . . loaded line %d\n", lineNo);
			}
		}
		lineNo++;
	}

	/** clean up */
	workingTuple->unref();
	newlyReadTuple->unref();
	return 1;

//FAIL:
//	workingTuple->unref();
//	newlyReadTuple->unref();
//	return 0;
}

