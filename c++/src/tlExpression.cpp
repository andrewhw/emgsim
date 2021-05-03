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
 * $Id: tlExpression.cpp 57 2013-12-13 21:33:01Z andrew $
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

#include "tlExpression.h"


tlExpression::tlExpression(
		tlErrorManager *err,
		int nVariables,
		tlExpression::DefinitionVariable **variableList
    )
{
	int i;

	err_ = err;
	if (err_ != NULL)
		err_->ref();

	/* store a copy of the variable list */
	if (nVariables > 0)
	{
		nVariables_ = nVariables;
		variableList_ = (tlExpression::DefinitionVariable **)
					ckalloc(sizeof(tlExpression::DefinitionVariable *)
								* nVariables_);
		for (i = 0; i < nVariables_; i++)
		{
			variableList_[i] = variableList[i];
			variableList_[i]->ref();
		}
	} else
	{
		nVariables_ = 0;
		variableList_ = NULL;
	}
}

tlExpression::~tlExpression()
{
	int i;

	if (variableList_ != NULL)
	{
		for (i = 0; i < nVariables_; i++)
		{
			variableList_[i]->unref();
		}
		ckfree(variableList_);
	}

	if (err_ != NULL)
		err_->unref();
}

int
tlExpression::addDefinitionVariable(
		DefinitionVariable *newVariable
    )
{
	DefinitionVariable **oldList;
	int i;

	oldList = variableList_;

	variableList_ = (tlExpression::DefinitionVariable **)
					ckalloc(sizeof(tlExpression::DefinitionVariable *)
								* (nVariables_ + 1));

	if (oldList != NULL)
	{
		for (i = 0; i < nVariables_; i++)
		{
			variableList_[i] = oldList[i];
		}
		ckfree(oldList);
	}
	variableList_[nVariables_] = newVariable;
	variableList_[nVariables_]->ref();
	nVariables_++;

	return 1;
}

int
tlExpression::parseExpFunction1Argument__(
		Expression **argumentA,
		const char *functionName,
		tokenizer *t
    )
{
	token *token;

	if ( ! parseExpParseTreeTop__(argumentA, t) )
	{
		if (err_ != NULL)
			err_->addError(
					"Failed parsing argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ')'))
	{
		if (err_ != NULL)
			err_->addError("Expected ')' after one argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	return 1;


FAIL:
	if ((*argumentA) != NULL)
	{
		(*argumentA)->ref();
		(*argumentA)->unref();
		(*argumentA) = NULL;
	}
	return 0;
}

int
tlExpression::parseExpFunction2Arguments__(
		Expression **argumentA,
		Expression **argumentB,
		const char *functionName,
		tokenizer *t
    )
{
	token *token;

	if ( ! parseExpParseTreeTop__(argumentA, t) )
	{
		if (err_ != NULL)
			err_->addError(
				"Failed parsing first argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ','))
	{
		if (err_ != NULL)
			err_->addError(
					"Expected ',' after first argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	if ( ! parseExpParseTreeTop__(argumentB, t) )
	{
		if (err_ != NULL)
			err_->addError(
				"Failed parsing second argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ')'))
	{
		if (err_ != NULL)
			err_->addError(
				"Expected ')' after second argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	return 1;


FAIL:
	if ((*argumentA) != NULL)
	{
		(*argumentA)->ref();
		(*argumentA)->unref();
		(*argumentA) = NULL;
	}
	if ((*argumentB) != NULL)
	{
		(*argumentB)->ref();
		(*argumentB)->unref();
		(*argumentB) = NULL;
	}
	return 0;
}

int
tlExpression::parseExpFunction3Arguments__(
		Expression **argumentA,
		Expression **argumentB,
		Expression **argumentC,
		const char *functionName,
		tokenizer *t
    )
{
	token *token;

	if ( ! parseExpParseTreeTop__(argumentA, t) )
	{
		if (err_ != NULL)
			err_->addError(
				"Failed parsing first argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ','))
	{
		if (err_ != NULL)
			err_->addError(
					"Expected ',' after first argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	if ( ! parseExpParseTreeTop__(argumentB, t) )
	{
		if (err_ != NULL)
			err_->addError(
				"Failed parsing second argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}


	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ','))
	{
		if (err_ != NULL)
			err_->addError(
				"Expected ',' after second argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	if ( ! parseExpParseTreeTop__(argumentC, t) )
	{
		if (err_ != NULL)
			err_->addError(
				"Failed parsing third argument to function '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ')'))
	{
		if (err_ != NULL)
			err_->addError(
				"Expected ')' after third argument to '%s' at line %d",
				functionName, tknGetLineNo(t));
		goto FAIL;
	}

	return 1;


FAIL:
	if ((*argumentA) != NULL)
	{
		(*argumentA)->ref();
		(*argumentA)->unref();
		(*argumentA) = NULL;
	}
	if ((*argumentB) != NULL)
	{
		(*argumentB)->ref();
		(*argumentB)->unref();
		(*argumentB) = NULL;
	}
	if ((*argumentC) != NULL)
	{
		(*argumentC)->ref();
		(*argumentC)->unref();
		(*argumentC) = NULL;
	}
	return 0;
}

static double
bound__(double arg1, double arg2, double arg3)
{
	return MAX(arg1, MIN(arg2, arg3));
}

static double
max__(double arg1, double arg2)
{
	return MAX(arg1, arg2);
}

static double
min__(double arg1, double arg2)
{
	return MIN(arg1, arg2);
}

static double
finite__(double arg)
{
	return isfinite(arg);
}

int
tlExpression::parseExpFunction__(
		Expression **result,
		const char *identifier,
		tokenizer *t
    )
{
	Expression		*arg1 = NULL,
					*arg2 = NULL,
				*arg3 = NULL;


	*result = NULL;

	/**
	 * One-argument functions
	 */
	if (strcmp(identifier, "sqrt") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError(
					"Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "sqrt", sqrt, arg1);
		return 1;
	}

	if (strcmp(identifier, "log") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "log", log, arg1);
		return 1;
	}

	if (strcmp(identifier, "log10") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "log10", log10, arg1);
		return 1;
	}

	if (strcmp(identifier, "exp") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "exp", exp, arg1);
		return 1;
	}

	if (strcmp(identifier, "abs") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "fabs", fabs, arg1);
		return 1;
	}

	if (strcmp(identifier, "fabs") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "fabs", fabs, arg1);
		return 1;
	}

	if (strcmp(identifier, "sin") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "sin", sin, arg1);
		return 1;
	}

	if (strcmp(identifier, "cos") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "cos", cos, arg1);
		return 1;
	}

	if (strcmp(identifier, "atan") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "atan", atan, arg1);
		return 1;
	}

	if (strcmp(identifier, "finite") == 0)
	{
		if ( ! parseExpFunction1Argument__(&arg1, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction1Arg(this, "finite", finite__, arg1);
		return 1;
	}



	/**
	 * Two-argument functions
	 */
	if (strcmp(identifier, "atan2") == 0)
	{
		if ( ! parseExpFunction2Arguments__(&arg1, &arg2, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction2Arg(this, "atan2", atan2, arg1, arg2);
		return 1;
	}

	if (strcmp(identifier, "pow") == 0)
	{
		if ( ! parseExpFunction2Arguments__(&arg1, &arg2, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction2Arg(this, "pow", pow, arg1, arg2);
		return 1;
	}

	if (strcmp(identifier, "max") == 0)
	{
		if ( ! parseExpFunction2Arguments__(&arg1, &arg2, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction2Arg(this, "max", max__, arg1, arg2);
		return 1;
	}

	if (strcmp(identifier, "min") == 0)
	{
		if ( ! parseExpFunction2Arguments__(&arg1, &arg2, identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction2Arg(this, "min", min__, arg1, arg2);
		return 1;
	}



	/**
	 * Three-argument functions
	 */
	if (strcmp(identifier, "gaussN") == 0)
	{
		if ( ! parseExpFunction3Arguments__(&arg1, &arg2, &arg3,
						identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction3Arg(this, "calculateNormalizedGaussian",
						calculateNormalizedGaussian, arg1, arg2, arg3);
		return 1;
	}

	if (strcmp(identifier, "gauss") == 0)
	{
		if ( ! parseExpFunction3Arguments__(&arg1, &arg2, &arg3,
						identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction3Arg(this, "calculateScaledGaussian",
						calculateScaledGaussian, arg1, arg2, arg3);
		return 1;
	}

	if (strcmp(identifier, "bound") == 0)
	{
		if ( ! parseExpFunction3Arguments__(&arg1, &arg2, &arg3,
						identifier, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed reading arguments to \"%s\" at line %d",
					identifier, tknGetLineNo(t));
			goto FAIL;
		}
		*result = new ExpFunction3Arg(this, "bound", bound__, arg1, arg2, arg3);
		return 1;
	}


	if (err_ != NULL)
		err_->addError("Unknown function \"%s\" found at line %d",
					identifier, tknGetLineNo(t));
FAIL:
	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}

int
tlExpression::parseExpUnary__(
		Expression **result,
		tokenizer *t
    )
{
	token *token;
	const char *identifier = NULL;
	int status = 0;
	int i, foundIndex;

	token = tknGetToken(t);
	if ((token != NULL) && (token->type_ == '-'))
	{
		Expression *constMinus = NULL;

		if ( ! parseExpParseTreeTop__(result, t) )
		{
			if (err_ != NULL)
				err_->addError("Failed parsing number after '-' on line %d",
						tknGetLineNo(t));
			goto FAIL;
		}
		constMinus = new ExpConstant(this, -1);
		*result = new ExpMult(this, *result, constMinus);
		return 1;
	}


	tknPushToken(t);

	/** the number */
	token = tknGetToken(t);
	if (token == NULL)
	{
		if (err_ != NULL)
			err_->addError("Unexpected EOF when expecting number on line %d",
						tknGetLineNo(t));
		goto FAIL;
	} else if (token->type_ == TT_INTEGER)
	{
		*result = new ExpConstant(this, t->token_.data_.ival_);

	} else if (token->type_ == TT_REAL)
	{
		*result = new ExpConstant(this, t->token_.data_.dval_);

		/** the number may be a variable */
	} else if (token->type_ == TT_IDENTIFIER)
	{

		/** check for constants */
		char *lowerid;

		lowerid = ckstrdup(token->data_.strptr_);
		MSG_ASSERT(lowerid != NULL, "malloc failed");

		/**
		 * infinity comes in mixed case on some systems,
		 * notably BSD, so check in a case-insensitive way
		 */
		(void) strtolower(lowerid);

		if ((strcmp(lowerid, "inf") == 0)
				||(strcmp(lowerid, "infinity") == 0))
		{
			*result = new ExpConstant(this, (-1) * log((float)0));
			return 1;
		}

		if (strcmp(lowerid, "pi") == 0)
		{
			*result = new ExpConstant(this, M_PI);
			return 1;
		}

		ckfree(lowerid);


		if (strcmp(token->data_.strptr_, "e") == 0)
		{
			*result = new ExpConstant(this, M_E);
			return 1;
		}


		/**
		 * copy the identifier value so we can look ahead
		 * without overwriting this part of the buffer
		 */
		identifier = ckstrdup(token->data_.strptr_);


		/** check if this is a function */
		token = tknGetToken(t);
		if ((token != NULL) && (token->type_ == '('))
		{
			status = parseExpFunction__(result, identifier, t);
			ckfree((void *) identifier);
			return status;
		}
		tknPushToken(t);


		/** must be a variable reference */
		if (nVariables_ > 0)
		{
			foundIndex = (-1);
			for (i = 0; i < nVariables_; i++)
			{
				if (variableList_[i]->name_ == identifier)
				{
					foundIndex = i;
					break;
				}
			}
			if (foundIndex < 0)
			{
				if (err_ != NULL)
					err_->addError("Found reference to unknown variable"
							" '%s' in number at line %d",
								identifier,
								tknGetLineNo(t));
				goto FAIL;
			}
		
			(*result) = new ExpVariable(this, variableList_[i]);
		} else
		{
			if (err_ != NULL)
				err_->addError(
						"No variable list at variable reference at line %d",
						tknGetLineNo(t));
			goto FAIL;
		}

	} else
	{
		if (err_ != NULL)
			err_->addError("Expecting number on line %d", tknGetLineNo(t));
		goto FAIL;
	}

	if (identifier != NULL)
		ckfree((void *) identifier);
	return 1;


FAIL:
	if (identifier != NULL)
		ckfree((void *) identifier);

	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}

int
tlExpression::parseExpBrackets__(
		Expression **result,
		tokenizer *t
    )
{
	token *token;

	token = tknGetToken(t);
	if ((token != NULL) && (token->type_ != '('))
	{

		/** return sub-expression */
		tknPushToken(t);

		return parseExpUnary__(result, t);
	}

	if ( ! parseExpParseTreeTop__(result, t) )
	{
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token == NULL) || (token->type_ != ')'))
	{
		if (err_ != NULL)
			err_->addError("Expecting ')' in numeric expression on line %d",
						tknGetLineNo(t));
		goto FAIL;
	}

	return 1;


FAIL:
	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}


int
tlExpression::parseExpExponent__(
		Expression **result,
		tokenizer *t
    )
{
	Expression *subResultBase = NULL;
	Expression *subResultExponent = NULL;
	token *token;

	if ( ! parseExpBrackets__(&subResultBase, t) )
	{
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token != NULL) && (token->type_ == '^'))
	{
		if ( ! parseExpParseTreeTop__(&subResultExponent, t) )
		{
			goto FAIL;
		}

		(*result) = new ExpExponent(this, subResultBase, subResultExponent);

	} else
	{
		tknPushToken(t);
		(*result) = subResultBase;
	}
	return 1;


FAIL:
	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}


int
tlExpression::parseExpMulDiv__(
		Expression **result,
		tokenizer *t
    )
{
	Expression *subResultL = NULL;
	Expression *subResultR = NULL;
	token *token;
	char type;

	if ( ! parseExpExponent__(&subResultL, t) )
	{
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token != NULL) && (token->type_ == '*' || token->type_ == '/'))
	{
		type = token->type_;
		if ( ! parseExpParseTreeTop__(&subResultR, t) )
		{
			goto FAIL;
		}

		if (type == '*')
		{
			(*result) = new ExpMult(this, subResultL, subResultR);
		} else
		{
			(*result) = new ExpDiv(this, subResultL, subResultR);
		}

	} else
	{
		tknPushToken(t);
		(*result) = subResultL;
	}
	return 1;


FAIL:
	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}


int
tlExpression::parseExpPlMinus__(
		Expression **result,
		tokenizer *t
    )
{
	Expression *subResultL = NULL;
	Expression *subResultR = NULL;
	token *token;
	char type;

	if ( ! parseExpMulDiv__(&subResultL, t) )
	{
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token != NULL) && (token->type_ == '+' || token->type_ == '-'))
	{
		type = token->type_;
		if ( ! parseExpParseTreeTop__(&subResultR, t) )
		{
			goto FAIL;
		}

		if (type == '+')
		{
			(*result) = new ExpPlus(this, subResultL, subResultR);
		} else
		{
			(*result) = new ExpMinus(this, subResultL, subResultR);
		}

	} else
	{
		tknPushToken(t);
		(*result) = subResultL;
	}
	return 1;


FAIL:
	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}


int
tlExpression::parseExpQuestionColon__(
		Expression **result,
		tokenizer *t
    )
{
	Expression *subResultQ = NULL;
	Expression *subResultR = NULL;
	Expression *subResultL = NULL;
	token *token;

	if ( ! parseExpPlMinus__(&subResultQ, t) )
	{
		goto FAIL;
	}

	token = tknGetToken(t);
	if ((token != NULL) && token->type_ == '?')
	{
		if ( ! parseExpQuestionColon__(&subResultL, t) )
		{
			goto FAIL;
		}

		token = tknGetToken(t);
		if ((token == NULL) || token->type_ != ':')
		{
			goto FAIL;
		}

		if ( ! parseExpQuestionColon__(&subResultR, t) )
		{
			goto FAIL;
		}

		(*result) = new ExpQuestionColon(this,
						subResultQ,
						subResultL,
						subResultR
					);

	} else
	{
		tknPushToken(t);
		(*result) = subResultQ;
	}
	return 1;


FAIL:
	if ((*result) != NULL)
	{
		(*result)->ref();
		(*result)->unref();
		(*result) = NULL;
	}
	return 0;
}


int
tlExpression::parseExpParseTreeTop__(
		Expression **result,
		tokenizer *t
    )
{
	return parseExpQuestionColon__(result, t);
}

/**
 * Parse a number, which could be a numeric expression
 */
tlExpression::Expression *
tlExpression::parseExpression(tokenizer *t)
{
	Expression *result = NULL;

	if (parseExpQuestionColon__(&result, t))
	{
		return result;
	}

	if (result != NULL)
	{
		result->ref();
		result->unref();
	}
	return NULL;
}


/**
 * Methods from the buried classes in this class
 */

tlSrString
tlExpression::Number::toString() const
{
	tlSrString s;

	if (type_ == TT_INTEGER)
		s.sprintf("%d", data_.ival_);
	else if (type_ == TT_REAL)
		s.sprintf("%f", data_.dval_);
	else
		s.sprintf("<bad_type>");

	return s;
}

tlExpression::Expression::Expression(tlExpression *parser)
{
	parser_ = parser;
	parser_->ref();
}

tlExpression::Expression::~Expression()
{
	if (parser_ != NULL)
	{
		parser_->unref();
		parser_ = NULL;
	}
}

tlExpression *
tlExpression::Expression::getParser()
{
	return parser_;
}

tlExpression::DefinitionVariable::DefinitionVariable(
		const char *name,
		int type
    )
{
	name_ = name;
	value_.type_ = type;
}

tlExpression::DefinitionVariable::DefinitionVariable(
		tlSrString name,
		int type
    )
{
	name_ = name;
	value_.type_ = type;
}

tlSrString
tlExpression::DefinitionVariable::toString() const
{
	tlSrString s;

	s.sprintf("%s %S",
			value_.type_ == TT_INTEGER ? "int" :
			value_.type_ == TT_REAL ? "double" :
		"<bad-type>",
		&name_);

	return s;
}



tlExpression::ExpVariable::ExpVariable(
		tlExpression *parser,
		DefinitionVariable *definition
    ) : tlExpression::Expression(parser)
{
	name_ = definition->name_;
	type_ = definition->value_.type_;
	data_ = definition;
	data_->ref();
}

tlExpression::ExpVariable::~ExpVariable()
{
	if (data_ != NULL)
	{
		data_->unref();
		data_ = NULL;
	}
}

tlSrString
tlExpression::ExpVariable::toString() const
{
	tlSrString s;
	s.sprintf("<%s-variable-%S>",
			type_ == TT_INTEGER ? "int" :
			type_ == TT_REAL ? "double" :
		"<bad-type>", &name_);

	return s;
}

tlExpression::Number
tlExpression::ExpVariable::calculateExpression() const
{
	return data_->value_;
}



tlExpression::ExpConstant::ExpConstant(
		tlExpression *parser,
		int value
    ) : tlExpression::Expression(parser)
{
	type_ = value_.type_ = TT_INTEGER;
	value_.data_.ival_ = value;
}

tlExpression::ExpConstant::ExpConstant(
		tlExpression *parser,
		double value
    ) : tlExpression::Expression(parser)
{
	type_ = value_.type_ = TT_REAL;
	value_.data_.dval_ = value;
}

tlExpression::ExpConstant::~ExpConstant()
{
}

tlSrString
tlExpression::ExpConstant::toString() const
{
	tlSrString s;
	if (type_ == TT_INTEGER)
		s.sprintf("%d", value_.data_.ival_);
	else if (type_ == TT_REAL)
		s.sprintf("%f", value_.data_.dval_);
	else
		s.sprintf("<bad-constant>");

	return s;
}

tlExpression::Number
tlExpression::ExpConstant::calculateExpression() const
{
	return value_;
}



tlExpression::ExpFunction3Arg::ExpFunction3Arg(
		tlExpression *parser,
		const char *name,
		double (*function)(double, double, double),
		Expression *arg0,
		Expression *arg1,
		Expression *arg2
    ) : tlExpression::Expression(parser)
{
	name_ = name;
	func_ = function;
	type_ = TT_REAL;
	arg_[0] = arg0;
	arg_[0]->ref();
	arg_[1] = arg1;
	arg_[1]->ref();
	arg_[2] = arg2;
	arg_[2]->ref();
}

tlExpression::ExpFunction3Arg::~ExpFunction3Arg()
{
	if (arg_[0] != NULL)
	{
		arg_[0]->unref();
		arg_[0] = NULL;
	}
	if (arg_[1] != NULL)
	{
		arg_[1]->unref();
		arg_[1] = NULL;
	}
	if (arg_[2] != NULL)
	{
		arg_[2]->unref();
		arg_[2] = NULL;
	}
}

tlSrString
tlExpression::ExpFunction3Arg::toString() const
{
	tlSrString s, a0, a1, a2;

	a0 = arg_[0]->toString();
	a1 = arg_[1]->toString();
	a2 = arg_[2]->toString();
	s.sprintf("%S(%S, %S, %S)", &name_, &a0, &a1, &a2);

	return s;
}

tlExpression::Number
tlExpression::ExpFunction3Arg::calculateExpression() const
{
	Number result;
	Number arg0, arg1, arg2;

	result.type_ = type_;
	MSG_ASSERT(type_ == TT_REAL, "non-real type function");
	arg0 = arg_[0]->calculateExpression();
	if (arg0.type_ != TT_REAL)
		arg0.data_.dval_ = (double) arg0.data_.ival_;
	arg1 = arg_[1]->calculateExpression();
	if (arg1.type_ != TT_REAL)
		arg1.data_.dval_ = (double) arg1.data_.ival_;
	arg2 = arg_[2]->calculateExpression();
	if (arg2.type_ != TT_REAL)
		arg2.data_.dval_ = (double) arg2.data_.ival_;
	result.data_.dval_ = (*func_)(
					arg0.data_.dval_,
					arg1.data_.dval_,
					arg2.data_.dval_);
	return result;
}



tlExpression::ExpFunction2Arg::ExpFunction2Arg(
		tlExpression *parser,
		const char *name,
		double (*function)(double, double),
		Expression *arg0,
		Expression *arg1
    ) : tlExpression::Expression(parser)
{
	name_ = name;
	func_ = function;
	type_ = TT_REAL;
	arg_[0] = arg0;
	arg_[0]->ref();
	arg_[1] = arg1;
	arg_[1]->ref();
}

tlExpression::ExpFunction2Arg::~ExpFunction2Arg()
{
	if (arg_[0] != NULL)
	{
		arg_[0]->unref();
		arg_[0] = NULL;
	}
	if (arg_[1] != NULL)
	{
		arg_[1]->unref();
		arg_[1] = NULL;
	}
}

tlSrString
tlExpression::ExpFunction2Arg::toString() const
{
	tlSrString s, a0, a1;
	a0 = arg_[0]->toString();
	a1 = arg_[1]->toString();
	s.sprintf("%S(%S, %S)", &name_, &a0, &a1);

	return s;
}

tlExpression::Number
tlExpression::ExpFunction2Arg::calculateExpression() const
{
	Number result;
	Number arg0, arg1;

	result.type_ = type_;
	MSG_ASSERT(type_ == TT_REAL, "non-real type function");
	arg0 = arg_[0]->calculateExpression();
	if (arg0.type_ != TT_REAL)
		arg0.data_.dval_ = (double) arg0.data_.ival_;
	arg1 = arg_[1]->calculateExpression();
	if (arg1.type_ != TT_REAL)
		arg1.data_.dval_ = (double) arg1.data_.ival_;
	result.data_.dval_ = (*func_)(
					arg0.data_.dval_,
					arg1.data_.dval_);
	return result;
}



tlExpression::ExpFunction1Arg::ExpFunction1Arg(
		tlExpression *parser,
		const char *name,
		double (*function)(double),
		Expression *arg
    ) : tlExpression::Expression(parser)
{
	name_ = name;
	func_ = function;
	type_ = TT_REAL;
	arg_[0] = arg;
	arg_[0]->ref();
}

tlExpression::ExpFunction1Arg::~ExpFunction1Arg()
{
	if (arg_[0] != NULL)
	{
		arg_[0]->unref();
		arg_[0] = NULL;
	}
}

tlSrString
tlExpression::ExpFunction1Arg::toString() const
{
	tlSrString s, a0;
	a0 = arg_[0]->toString();
	s.sprintf("%S(%S)", &name_, &a0);

	return s;
}

tlExpression::Number
tlExpression::ExpFunction1Arg::calculateExpression() const
{
	Number result;
	Number arg0;

	result.type_ = type_;
	MSG_ASSERT(type_ == TT_REAL, "non-real type function");
	arg0 = arg_[0]->calculateExpression();
	if (arg0.type_ != TT_REAL)
		arg0.data_.dval_ = (double) arg0.data_.ival_;
	result.data_.dval_ = (*func_)(arg0.data_.dval_);
	return result;
}



tlExpression::ExpExponent::ExpExponent(
		tlExpression *parser,
		Expression *base,
		Expression *exponent
    ) : tlExpression::Expression(parser)
{
	base_ = base;
	exponent_ = exponent;
	base_->ref();
	exponent_->ref();
}

tlExpression::ExpExponent::~ExpExponent()
{
	if (base_ != NULL)
	{
		base_->unref();
		base_ = NULL;
	}
	if (exponent_ != NULL)
	{
		exponent_->unref();
		exponent_ = NULL;
	}
}

tlSrString
tlExpression::ExpExponent::toString() const
{
	tlSrString s, baseString, expString;

	baseString = base_->toString();
	expString = exponent_->toString();
	s.sprintf("(%S)^(%S)", &baseString, &expString);

	return s;
}

tlExpression::Number
tlExpression::ExpExponent::calculateExpression() const
{
	Number result;
	Number base;
	Number exponent;

	base = base_->calculateExpression();
	if (base.type_ != TT_REAL)
		base.data_.dval_ = (double) base.data_.ival_;
	exponent = exponent_->calculateExpression();
	if (exponent.type_ != TT_REAL)
		exponent.data_.dval_ = (double) exponent.data_.ival_;

	result.type_ = TT_REAL;
	result.data_.dval_ = pow(base.data_.dval_, exponent.data_.dval_);

	return result;
}



tlExpression::ExpBinary::ExpBinary(
		tlExpression *parser,
		Expression *left,
		Expression *right
    ) : tlExpression::Expression(parser)
{
	left_ = left;
	right_ = right;
	left_->ref();
	right_->ref();
}

tlExpression::ExpBinary::~ExpBinary()
{
	if (left_ != NULL)
	{
		left_->unref();
		left_ = NULL;
	}
	if (right_ != NULL)
	{
		right_->unref();
		right_ = NULL;
	}
}

tlExpression::ExpMult::ExpMult(
		tlExpression *parser,
		Expression *left,
		Expression *right
    ) : ExpBinary(parser, left, right)
{
}

tlExpression::ExpMult::~ExpMult()
{
}

tlSrString
tlExpression::ExpMult::toString() const
{
	tlSrString s, left, right;
	left = left_->toString();
	right = right_->toString();
	s.sprintf("(%S) * (%S)", &left, &right);

	return s;
}

tlExpression::Number
tlExpression::ExpMult::calculateExpression() const
{
	Number result;
	Number left;
	Number right;

	left = left_->calculateExpression();
	right = right_->calculateExpression();

	result.type_ = TT_REAL;
	if ((left.type_ == TT_REAL) && (right.type_ == TT_REAL))
	{
		result.data_.dval_ = left.data_.dval_ * right.data_.dval_;

	} else if ((left.type_ == TT_REAL) && (right.type_ == TT_INTEGER))
	{
		result.data_.dval_ = left.data_.dval_ * right.data_.ival_;

	} else if ((left.type_ == TT_INTEGER) && (right.type_ == TT_REAL))
	{
		result.data_.dval_ = left.data_.ival_ * right.data_.dval_;

	} else
	{
		result.type_ = TT_INTEGER;
		result.data_.ival_ = left.data_.ival_ * right.data_.ival_;
	}

	return result;
}



tlExpression::ExpDiv::ExpDiv(
		tlExpression *parser,
		Expression *left,
		Expression *right
    ) : ExpBinary(parser, left, right)
{
}

tlExpression::ExpDiv::~ExpDiv()
{
}

tlSrString
tlExpression::ExpDiv::toString() const
{
	tlSrString s, left, right;
	left = left_->toString();
	right = right_->toString();
	s.sprintf("(%S) / (%S)", &left, &right);

	return s;
}

tlExpression::Number
tlExpression::ExpDiv::calculateExpression() const
{
	Number result;
	Number left;
	Number right;

	left = left_->calculateExpression();
	right = right_->calculateExpression();

	result.type_ = TT_REAL;
	if ((left.type_ == TT_REAL) && (right.type_ == TT_REAL))
	{
		if (right.data_.dval_ != 0)
			result.data_.dval_ = left.data_.dval_ / right.data_.dval_;
		else
			result.data_.dval_ = 0;

	} else if ((left.type_ == TT_REAL) && (right.type_ == TT_INTEGER))
	{
		if (right.data_.ival_ != 0)
			result.data_.dval_ = left.data_.dval_ / ((double) right.data_.ival_);
		else
			result.data_.dval_ = 0;

	} else if ((left.type_ == TT_INTEGER) && (right.type_ == TT_REAL))
	{
		if (right.data_.dval_ != 0)
			result.data_.dval_ = ((double) left.data_.ival_) / right.data_.dval_;
		else
			result.data_.dval_ = 0;

	} else
	{
		result.type_ = TT_INTEGER;
		if (right.data_.ival_ != 0)
			result.data_.ival_ = left.data_.ival_ / right.data_.ival_;
		else
			result.data_.ival_ = 0;
	}

	return result;
}


tlExpression::ExpQuestionColon::ExpQuestionColon(
		tlExpression *parser,
		Expression *question,
		Expression *left,
		Expression *right
    ) : ExpBinary(parser, left, right)
{
	question_ = question;
	question_->ref();
}

tlExpression::ExpQuestionColon::~ExpQuestionColon()
{
	if (question_ != NULL)
	{
		question_->unref();
		question_ = NULL;
	}
}

tlSrString
tlExpression::ExpQuestionColon::toString() const
{
	tlSrString s, question, left, right;
	question = question_->toString();
	left = left_->toString();
	right = right_->toString();
	s.sprintf("(%S) ? (%S) : (%S)", &question, &left, &right);

	return s;
}

tlExpression::Number
tlExpression::ExpQuestionColon::calculateExpression() const
{
	Number result;
	Number question;
	int logicTest;
	Number left;
	Number right;
	Number *source;

	question = question_->calculateExpression();

	if (question.type_ == TT_REAL)
	{
		logicTest = (question.data_.dval_ != 0.0);
	} else
	{
		logicTest = (question.data_.ival_ != 0);
	}

	if (logicTest)
	{
		left = left_->calculateExpression();
		source = &left;
	} else
	{
		right = right_->calculateExpression();
		source = &right;
	}

	result.type_ = TT_REAL;
	if (source->type_ == TT_REAL)
	{
		result.type_ = TT_REAL;
		result.data_.dval_ = source->data_.dval_;

	} else
	{
		result.type_ = TT_INTEGER;
		result.data_.ival_ = source->data_.ival_;
	}

	return result;
}


tlExpression::ExpPlus::ExpPlus(
		tlExpression *parser,
		Expression *left,
		Expression *right
    ) : ExpBinary(parser, left, right)
{
}

tlExpression::ExpPlus::~ExpPlus()
{
}

tlSrString
tlExpression::ExpPlus::toString() const
{
	tlSrString s, left, right;
	left = left_->toString();
	right = right_->toString();
	s.sprintf("(%S) + (%S)", &left, &right);

	return s;
}

tlExpression::Number
tlExpression::ExpPlus::calculateExpression() const
{
	Number result;
	Number left;
	Number right;

	left = left_->calculateExpression();
	right = right_->calculateExpression();

	result.type_ = TT_REAL;
	if ((left.type_ == TT_REAL) && (right.type_ == TT_REAL))
	{
		result.data_.dval_ = left.data_.dval_ + right.data_.dval_;

	} else if ((left.type_ == TT_REAL) && (right.type_ == TT_INTEGER))
	{
		result.data_.dval_ = left.data_.dval_ + right.data_.ival_;

	} else if ((left.type_ == TT_INTEGER) && (right.type_ == TT_REAL))
	{
		result.data_.dval_ = left.data_.ival_ + right.data_.dval_;

	} else
	{
		result.type_ = TT_INTEGER;
		result.data_.ival_ = left.data_.ival_ + right.data_.ival_;
	}

	return result;
}



tlExpression::ExpMinus::ExpMinus(
		tlExpression *parser,
		Expression *left,
		Expression *right
    ) : ExpBinary(parser, left, right)
{
}

tlExpression::ExpMinus::~ExpMinus()
{
}

tlSrString
tlExpression::ExpMinus::toString() const
{
	tlSrString s, left, right;
	left = left_->toString();
	right = right_->toString();
	s.sprintf("(%S) - (%S)", &left, &right);

	return s;
}

tlExpression::Number
tlExpression::ExpMinus::calculateExpression() const
{
	Number result;
	Number left;
	Number right;

	left = left_->calculateExpression();
	right = right_->calculateExpression();

	result.type_ = TT_REAL;
	if ((left.type_ == TT_REAL) && (right.type_ == TT_REAL))
	{
		result.data_.dval_ = left.data_.dval_ - right.data_.dval_;

	} else if ((left.type_ == TT_REAL) && (right.type_ == TT_INTEGER))
	{
		result.data_.dval_ = left.data_.dval_ - right.data_.ival_;

	} else if ((left.type_ == TT_INTEGER) && (right.type_ == TT_REAL))
	{
		result.data_.dval_ = left.data_.ival_ - right.data_.dval_;

	} else
	{
		result.type_ = TT_INTEGER;
		result.data_.ival_ = left.data_.ival_ - right.data_.ival_;
	}

	return result;
}

