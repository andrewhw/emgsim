/**
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
 * $Id: tlSParser.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_PARSER_HEADER__
#define		__TOOL_PARSER_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlRef.h"
#include "tlErrorManager.h"

struct attValList;

/**
CLASS
		tlSParser

	A container for various parsing tools
*/
class OS_EXPORT tlSParser
{
protected:
	////////////////////////////////////////
	// Create the parser tool set (with error handler)
	tlSParser();

	////////////////////////////////////////
	// Clean up
	virtual ~tlSParser();

protected:

	////////////////////////////////////////
	// Parse a (mathematical) function call
	static int parseNumFunction__(
				double *result,
				const char *identifier,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse the arguments for a single-arg
	// function
	static int parseNumFunction1Argument__(
				double *result,
				const char *identifier,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse the arguments for a double-arg
	// function
	static int parseNumFunction2Arguments__(
				double *resultA, double *resultB,
				const char *identifier,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse the arguments for a triple-arg
	// function
	static int parseNumFunction3Arguments__(
				double *resultA, double *resultB, double *resultC,
				const char *identifier,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// parse up args of print function
	static int parsePrintFunctionArgs__(
					FILE *ofp,
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse a unary string
	static int parseNumUnary__(
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse an expression in brackets
	static int parseNumBrackets__(
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse an exponent expression
	static int parseNumExponent__(
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse multiplication and division
	static int parseNumMulDiv__(
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse addition and subtraction
	static int parseNumPlMinus__(
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

public:
	////////////////////////////////////////
	// Parse a numeric expression, return the resulting number
	// in the <b>result</b> argument
	static int parseNumber(
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse a string expression
	static int parseString(
				char **result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		);

	////////////////////////////////////////
	// Parse a variable assignment expression
	static int parseVariable(
				attValList *parsingVariables,
				tokenizer *t,
				char *idName,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Parse function to print values on
	// stream and return a success fail result
	static int parsePrintFunction(
					FILE *ofp,
				double *result,
				tokenizer *t,
				attValList *parsingVariables,
				tlErrorManager *err
		    );
};

#endif /* __TOOL_PARSER_HEADER__ */

