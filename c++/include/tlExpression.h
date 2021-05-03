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
 * $Id: tlExpression.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_EXPRESSION_HEADER__
#define		__TOOL_EXPRESSION_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlRef.h"
#include "tlErrorManager.h"
#include "tlSrString.h"

#include "tokens.h"

class tlExpression;

/**
CLASS
		tlExpression

	A container for various parsing tools
*/
class OS_EXPORT tlExpression : public tlRef
{
public:

	class Number {
	public:
		int type_;
		union {
		    double dval_;
		    int ival_;
		} data_;

		tlSrString toString() const;
	};

	class DefinitionVariable : public tlRef {
	public:
		tlSrString name_;
		Number value_;

		DefinitionVariable(const char *name, int type);
		DefinitionVariable(tlSrString name, int type);

		tlSrString toString() const;
		Number calculateExpression() const;
	};



	class Expression : public tlRef {
	public:
		int type_;
		tlExpression *parser_;

		Expression(tlExpression *parser);
		virtual ~Expression();

		virtual tlSrString toString() const = 0;
		virtual Number calculateExpression() const = 0;
		tlExpression *getParser();
	};

	class ExpVariable : public Expression {
	public:
		tlSrString name_;
		DefinitionVariable *data_;

		ExpVariable(tlExpression *parser, DefinitionVariable *definition);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpVariable();
	};

	class ExpConstant : public Expression {
	public:
		Number value_;

		ExpConstant(tlExpression *parser, int value);
		ExpConstant(tlExpression *parser, double value);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpConstant();
	};

	class ExpFunction3Arg : public Expression {
	public:
		tlSrString name_;
		Expression *arg_[3];
		double (*func_)(double, double, double);

		ExpFunction3Arg(
				tlExpression *parser,
				const char *name,
				double (*function)(double, double, double),
				Expression *arg0,
				Expression *arg1,
				Expression *arg2
		    );

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpFunction3Arg();
	};

	class ExpFunction2Arg : public Expression {
	public:
		tlSrString name_;
		Expression *arg_[2];
		double (*func_)(double, double);

		ExpFunction2Arg(
				tlExpression *parser,
				const char *name,
				double (*function)(double, double),
				Expression *arg0,
				Expression *arg1
		    );

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpFunction2Arg();
	};

	class ExpFunction1Arg : public Expression {
	public:
		tlSrString name_;
		Expression *arg_[1];
		double (*func_)(double);

		ExpFunction1Arg(
				tlExpression *parser,
				const char *name,
				double (*function)(double),
				Expression *arg
		    );

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpFunction1Arg();
	};

	class ExpExponent : public Expression {
	public:
		Expression *base_;
		Expression *exponent_;

		ExpExponent(tlExpression *parser, Expression *left, Expression *right);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpExponent();
	};

	class ExpBinary : public Expression {
	public:
		Expression *left_;
		Expression *right_;

		ExpBinary(tlExpression *parser, Expression *left, Expression *right);
	protected:
		virtual ~ExpBinary();
	};

	class ExpMult : public ExpBinary {
	public:
		ExpMult(tlExpression *parser, Expression *left, Expression *right);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpMult();
	};

	class ExpDiv : public ExpBinary {
	public:
		ExpDiv(tlExpression *parser, Expression *left, Expression *right);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpDiv();
	};

	class ExpPlus : public ExpBinary {
	public:
		ExpPlus(tlExpression *parser, Expression *left, Expression *right);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpPlus();
	};

	class ExpMinus : public ExpBinary {
	public:
		ExpMinus(tlExpression *parser, Expression *left, Expression *right);

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpMinus();
	};

	class ExpQuestionColon : public ExpBinary {
	public:
		Expression *question_;

		ExpQuestionColon(
				tlExpression *parser,
				Expression *question,
				Expression *left,
				Expression *right
		    );

		virtual tlSrString toString() const;
		virtual Number calculateExpression() const;
	protected:
		virtual ~ExpQuestionColon();
	};


public:
	////////////////////////////////////////
	// Create a parser
	tlExpression(
				tlErrorManager *err,
				int nVariables,
				DefinitionVariable **variableList
		    );

protected:
	////////////////////////////////////////
	// Clean up
	virtual ~tlExpression();

protected:

	////////////////////////////////////////
	// Parse a (mathematical) function call
	int parseExpFunction__(
				Expression **result,
				const char *identifier,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse the arguments for a single-arg
	// function
	int parseExpFunction1Argument__(
				Expression **result,
				const char *identifier,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse the arguments for a double-arg
	// function
	int parseExpFunction2Arguments__(
				Expression **resultA,
				Expression **resultB,
				const char *identifier,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse the arguments for a triple-arg
	// function
	int parseExpFunction3Arguments__(
				Expression **resultA,
				Expression **resultB,
				Expression **resultC,
				const char *identifier,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse a unary string
	int parseExpUnary__(
				Expression **result,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse an expression in brackets
	int parseExpBrackets__(
				Expression **result,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse an exponent expression
	int parseExpExponent__(
				Expression **result,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse multiplication and division
	int parseExpMulDiv__(
				Expression **result,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse addition and subtraction
	int parseExpPlMinus__(
				Expression **result,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Parse a XXX ? YYY : ZZZ statement
	int parseExpQuestionColon__(
				Expression **result,
				tokenizer *t
		    );

	////////////////////////////////////////
	// Re-parse a sub-expression starting at the top
	// of the parse tree
	int parseExpParseTreeTop__(
				Expression **result,
				tokenizer *t
		    );

public:
	////////////////////////////////////////
	// Parse a numeric expression, return the resulting number
	// in the <b>result</b> argument.
	//
	// The expression keeps a pointer to the parser in order
	// to have a handle to the variable list within it.
	Expression *parseExpression(tokenizer *t);

	////////////////////////////////////////
	// Return the number of definition variables we have
	int getNumDefinitionVariables() const;

	////////////////////////////////////////
	// Return a definition variables from our internal list
	DefinitionVariable *getDefinitionVariable(int index);


	////////////////////////////////////////
	// Add a new definition variable to the list
	int addDefinitionVariable(DefinitionVariable *newVariable);

private:
	tlErrorManager *err_;

	int nVariables_;
	DefinitionVariable **variableList_;
};

inline int tlExpression::getNumDefinitionVariables() const
{
	return nVariables_;
}

inline tlExpression::DefinitionVariable *
tlExpression::getDefinitionVariable(int index)
{
	return variableList_[index];
}

#endif /* __TOOL_EXPRESSION_HEADER__ */

