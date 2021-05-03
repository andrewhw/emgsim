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
 * $Id: tlSParser.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <stdarg.h>
# include <math.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"
#include "mathtools.h"
#include "massert.h"
#include "tokens.h"
#include "attvalfile.h"

#include "tlSParser.h"


int
tlSParser::parseNumFunction1Argument__(
		double *result,
		const char *functionName,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;

	if ( ! parseNumber(result, t, parsingVariables, err) )
	{
		err->addError("Failed parsing argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ != ')')
	{
		err->addError("Expected ')' after one argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	return 1;
}

int
tlSParser::parseNumFunction2Arguments__(
		double *resultA, double *resultB,
		const char *functionName,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;

	if ( ! parseNumber(resultA, t, parsingVariables, err) )
	{
		err->addError(
				"Failed parsing first argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ != ',')
	{
		err->addError("Expected ',' after first argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	if ( ! parseNumber(resultB, t, parsingVariables, err) )
	{
		err->addError(
				"Failed parsing second argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ != ')')
	{
		err->addError("Expected ')' after second argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	return 1;
}

int
tlSParser::parseNumFunction3Arguments__(
		double *resultA, double *resultB, double *resultC,
		const char *functionName,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;

	if ( ! parseNumber(resultA, t, parsingVariables, err) )
	{
		err->addError(
				"Failed parsing first argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ != ',')
	{
		err->addError("Expected ',' after first argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	if ( ! parseNumber(resultB, t, parsingVariables, err) )
	{
		err->addError(
				"Failed parsing second argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}


	token = tknGetToken(t);
	if (token->type_ != ',')
	{
		err->addError("Expected ',' after second argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	if ( ! parseNumber(resultC, t, parsingVariables, err) )
	{
		err->addError(
				"Failed parsing third argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ != ')')
	{
		err->addError("Expected ')' after third argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		return 0;
	}

	return 1;
}

int
tlSParser::parseNumFunction__(
		double *result,
		const char *identifier,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	double arg1, arg2, arg3;


	/**
	 * One-argument functions
	 */
	if (strcmp(identifier, "sqrt") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = sqrt(arg1);
		return 1;
	}

	if (strcmp(identifier, "log") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = log(arg1);
		return 1;
	}

	if (strcmp(identifier, "log10") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = log10(arg1);
		return 1;
	}

	if (strcmp(identifier, "exp") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = exp(arg1);
		return 1;
	}

	if (strcmp(identifier, "abs") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = fabs(arg1);
		return 1;
	}

	if (strcmp(identifier, "fabs") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = fabs(arg1);
		return 1;
	}

	if (strcmp(identifier, "sin") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = sin(arg1);
		return 1;
	}

	if (strcmp(identifier, "cos") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = cos(arg1);
		return 1;
	}

	if (strcmp(identifier, "atan") == 0)
	{
		if ( ! parseNumFunction1Argument__(
							&arg1, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = atan(arg1);
		return 1;
	}



	/**
	 * Two-argument functions
	 */
	if (strcmp(identifier, "atan2") == 0)
	{
		if ( ! parseNumFunction2Arguments__(
							&arg1, &arg2, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = atan2(arg1, arg2);
		return 1;
	}

	if (strcmp(identifier, "pow") == 0)
	{
		if ( ! parseNumFunction2Arguments__(
							&arg1, &arg2, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = pow(arg1, arg2);
		return 1;
	}

	if (strcmp(identifier, "max") == 0)
	{
		if ( ! parseNumFunction2Arguments__(
							&arg1, &arg2, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = arg1 > arg2 ? arg1 : arg2;
		return 1;
	}

	if (strcmp(identifier, "min") == 0)
	{
		if ( ! parseNumFunction2Arguments__(
							&arg1, &arg2, identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = arg1 < arg2 ? arg1 : arg2;
		return 1;
	}



	/**
	 * Three-argument functions
	 */
	if (strcmp(identifier, "gaussN") == 0)
	{
		if ( ! parseNumFunction3Arguments__(
							&arg1, &arg2, &arg3,
						identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = calculateNormalizedGaussian(arg1, arg2, arg3);
		return 1;
	}

	if (strcmp(identifier, "gauss") == 0)
	{
		if ( ! parseNumFunction3Arguments__(
							&arg1, &arg2, &arg3,
						identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = calculateScaledGaussian(arg1, arg2, arg3);
		return 1;
	}

	if (strcmp(identifier, "bound") == 0)
	{
		if ( ! parseNumFunction3Arguments__(
							&arg1, &arg2, &arg3,
						identifier, t, parsingVariables, err
					) )
		{
			err->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			return 0;
		}
		*result = MAX(arg1, MIN(arg2, arg3));
		return 1;
	}


	/**
	 * Debug print function
	 */
	if (strcmp(identifier, "print") == 0)
	{
		return parsePrintFunctionArgs__(stdout, result, t, parsingVariables, err);
	}



	err->addError("Unknown function \"%s\" found at line %d",
					identifier, tknGetLineNo(t));
	return 0;
}

int
tlSParser::parsePrintFunctionArgs__(
		FILE *ofp,
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	double arg;
	token *token;


	fprintf(ofp, "DBG : %3d  [", tknGetLineNo(t));

	token = tknGetToken(t);
	while (token->type_ != ')')
	{
		if (token->type_ == TT_STRING)
		{
			fputs(token->data_.strptr_, ofp);
		} else
		{
			tknPushToken(t);


			if ( ! parseNumber(&arg, t, parsingVariables, err) )
			{
				err->addError("Failed parsing print function at line %d",
						tknGetLineNo(t));
				return 0;
			}

			fprintf(ofp, "%g", arg);
		}

		token = tknGetToken(t);
		if (token->type_ == ',')
		{
			token = tknGetToken(t);
		}
	}

	fprintf(ofp, "]\n");
	*result = !ferror(ofp);
	return 1;
}


int
tlSParser::parsePrintFunction(
		FILE *ofp,
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;

	token = tknGetToken(t);
	if (token->type_ != '(')
	{
		err->addError("Failed parsing print function at line %d",
				tknGetLineNo(t));
		return 0;
	}

	return parsePrintFunctionArgs__(ofp, result, t, parsingVariables, err);
}

int
tlSParser::parseNumUnary__(
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;
	attVal *variable;
	char *identifier = NULL;
	int status = 0;

	token = tknGetToken(t);
	if (token->type_ == '-')
	{

		if ( ! parseNumber(result, t, parsingVariables, err) )
		{
			err->addError("Failed parsing number after '-' on line %d",
						tknGetLineNo(t));
			return 0;
		}

		*result = (*result) * (-1);
		return 1;
	}


	tknPushToken(t);

	/** the number */
	token = tknGetToken(t);
	if (token->type_ == TT_INTEGER)
	{
		*result = t->token_.data_.ival_;

	} else if (token->type_ == TT_REAL)
	{
		*result = t->token_.data_.dval_;

		/** the number may be a variable */
	} else if (token->type_ == TT_IDENTIFIER)
	{

		/** check for constants */
		if ((strcmp(token->data_.strptr_, "inf") == 0)
				||(strcmp(token->data_.strptr_, "infinity") == 0))
		{
			*result = (-1) * log((float)0);
			return 1;
		}

		if (strcmp(token->data_.strptr_, "pi") == 0)
		{
			*result = M_PI;
			return 1;
		}

		if (strcmp(token->data_.strptr_, "e") == 0)
		{
			*result = M_E;
			return 1;
		}


		/** remember the identifier value so we can look ahead */
		identifier = ckstrdup(token->data_.strptr_);


		/** check if this is a function */
		token = tknGetToken(t);
		if (token->type_ == '(')
		{
			status = parseNumFunction__(
							result,
						identifier,
						t,
						parsingVariables,
						err
					);
			ckfree(identifier);
			return status;
		}
		tknPushToken(t);


		/** must be a variable reference */
		if (parsingVariables != NULL)
		{
			variable = getAttVal(parsingVariables, identifier);
			if (variable == NULL)
			{
				err->addError("Found reference to unknown variable"
							" '%s' in number at line %d",
								identifier,
								tknGetLineNo(t));
				goto CLEANUP;
			}
		
			if (variable->type_ == TT_REAL)
			{
				(*result) = variable->data_.dval_;

			} else if (variable->type_ == TT_INTEGER)
			{
				(*result) = variable->data_.ival_;

			} else
			{
				err->addError("Reference to variable '%s' with "
							"bad type at line %d",
							identifier,
							tknGetLineNo(t));
				goto CLEANUP;
			}
		} else
		{
			err->addError(
				"No variable list at variable reference at line %d",
				tknGetLineNo(t));
			return 0;
		}

	} else
	{
		err->addError("Expecting number on line %d", tknGetLineNo(t));
		return 0;
	}

	status = 1;


CLEANUP:
	if (identifier != NULL)
		ckfree(identifier);

	return status;
}

int
tlSParser::parseNumBrackets__(
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;

	token = tknGetToken(t);
	if (token->type_ != '(')
	{

		/** return sub-expression */
		tknPushToken(t);

		return parseNumUnary__(result, t, parsingVariables, err);
	}

	if ( ! parseNumPlMinus__(result, t, parsingVariables, err) )
	{
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ != ')')
	{
		err->addError("Expecting ')' in numeric expression on line %d",
						tknGetLineNo(t));
		return 0;
	}

	return 1;
}


int
tlSParser::parseNumExponent__(
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	double subResultBase;
	double subResultExponent;
	token *token;

	if ( ! parseNumBrackets__(&subResultBase, t, parsingVariables, err) )
	{
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ == '^')
	{
		if ( ! parseNumber(&subResultExponent, t, parsingVariables, err) )
		{
			return 0;
		}

		(*result) = pow(subResultBase, subResultExponent);

	} else
	{
		tknPushToken(t);
		(*result) = subResultBase;
	}
	return 1;
}


int
tlSParser::parseNumMulDiv__(
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	double subResultL;
	double subResultR;
	token *token;
	char type;

	if ( ! parseNumExponent__(&subResultL, t, parsingVariables, err) )
	{
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ == '*' || token->type_ == '/')
	{
		type = token->type_;
		if ( ! parseNumber(&subResultR, t, parsingVariables, err) )
		{
			return 0;
		}

		if (type == '*')
		{
			(*result) = subResultL * subResultR;
		} else
		{
			(*result) = subResultL / subResultR;
		}

	} else
	{
		tknPushToken(t);
		(*result) = subResultL;
	}
	return 1;
}


int
tlSParser::parseNumPlMinus__(
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	double subResultL;
	double subResultR;
	token *token;
	char type;

	if ( ! parseNumMulDiv__(&subResultL, t, parsingVariables, err) )
	{
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ == '+' || token->type_ == '-')
	{
		type = token->type_;
		if ( ! parseNumber(&subResultR, t, parsingVariables, err) )
		{
			return 0;
		}

		if (type == '+')
		{
			(*result) = subResultL + subResultR;
		} else
		{
			(*result) = subResultL - subResultR;
		}

	} else
	{
		tknPushToken(t);
		(*result) = subResultL;
	}
	return 1;
}

/**
 * Parse a number, which could be a numeric expression
 */
int
tlSParser::parseNumber(
		double *result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	return parseNumPlMinus__(result, t, parsingVariables, err);
}


/**
 * Parse a string, which may include a variable expression
 */
int
tlSParser::parseString(
		char **result,
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;
	attVal *variable;

	token = tknGetToken(t);
	if (token->type_ == TT_STRING)
	{
		(*result) = (char *) token->data_.strptr_;
		return 1;
	}

	if (token->type_ != TT_IDENTIFIER)
	{
		err->addError("Expected string at  line %d", tknGetLineNo(t));
		return 0;
	}

	/**
	 * if we saw an identifier, then lookup from the
	 * variable list
	 */
	if (parsingVariables != NULL)
	{
		variable = getAttVal(parsingVariables, token->data_.strptr_);
		if (variable == NULL)
		{
			err->addError("Found reference to "
					"unknown variable '%s' in string at line %d",
							token->data_.strptr_,
								tknGetLineNo(t));
			return 0;
		}
	} else
	{
		err->addError("Variable reference with null list at line %d",
				tknGetLineNo(t));
		return 0;
	}
	
	if (variable->type_ == TT_STRING)
	{
		(*result) = (char *) variable->data_.strptr_;

	} else
	{
		err->addError(
				"Reference to variable '%s' with bad type at line %d",
						token->data_.strptr_,
						tknGetLineNo(t));
		return 0;
	}
	return 1;
}


int
tlSParser::parseVariable(
		attValList *parsingVariables,
		tokenizer *t,
		char *idName,
		tlErrorManager *err
    )
{
	attVal *newAttribute;
	token *token;
	double numericValue;

	token = tknGetToken(t);
	if (token->type_ != '=')
	{
		err->addError("Expecting '=' on line %d", tknGetLineNo(t));
		return 0;
	}

	token = tknGetToken(t);
	if (token->type_ == TT_STRING)
	{
		newAttribute = createStringAttribute(
						idName,
						t->token_.data_.strptr_
					);

	} else
	{
		tknPushToken(t);
		if ( ! parseNumber(&numericValue, t, parsingVariables, err) )
		{
			err->addError(
				"Failed parsing numeric value in variable assignment");
			return 0;
		}

		newAttribute = createRealAttribute(idName, (double) numericValue);
	}

	token = tknGetToken(t);
	if (token->type_ != ';')
	{
		err->addError("Expecting ';' on line %d", tknGetLineNo(t));
		return 0;
	}


	if (parsingVariables == NULL)
	{
		err->addError("Cannot add variable to null list at line %d",
								tknGetLineNo(t));
		return 0;
	}

	if ( ! updateAttVal(parsingVariables, newAttribute) )
	{
		err->addError("Failure adding variable to internal list at line %d",
								tknGetLineNo(t));
		return 0;
	}

	return 1;
}

