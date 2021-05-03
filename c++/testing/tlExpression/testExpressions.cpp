#include <stdio.h>

#include <tlExpression.h>
#include <tokens.h>

#include <testutils.h>

#include "mathtools.h"

struct evaluations {
    const char *expression_;
    int type;
    double expectedValue_;
};

static int
testQC(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    int i;

    struct evaluations evalData[] = {
		    { "0 ? 1 : 2",  TT_INTEGER, 2 },
		    { "1 ? 1 : 2",  TT_INTEGER, 1 },
		    { "0.0 ? 1.5 : 2.5",  TT_REAL, 2.5 },
		    { "1.0 ? 1.5 : 2.5",  TT_REAL, 1.5 },
		    { NULL, 0, 0 }
		};


    ep = new tlExpression(err, 0, NULL);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, "     Source : %s\n", s);
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		exp->unref();
		tknDeleteTokenizer(t);
    }

    ep->unref();

    return 1;
}


static int
testPlus(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    int i;

    struct evaluations evalData[] = {
		    { "3 + 4 + 5",  TT_INTEGER, 12 },
		    { "2 + 2 - 4",  TT_INTEGER, 0 },
		    { "-4 + 2 + 2", TT_INTEGER, 0 },
		    { "pi + pi",  TT_REAL, M_PI * 2.0 },
		    { "e - e",  TT_REAL, 0},
		    { "1 + 0.5 + 0.25 + 0.125 + 0.075",  TT_REAL,
		    				1 + 0.5 + 0.25 + 0.125 + 0.075		},
		    { NULL, 0, 0 }
		};


    ep = new tlExpression(err, 0, NULL);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, "     Source : %s\n", s);
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		exp->unref();
		tknDeleteTokenizer(t);
    }

    ep->unref();

    return 1;
}

static int
testMinus(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    int i;

    struct evaluations evalData[] = {
		    { "3 - 4",  TT_INTEGER, -1 },
		    { "(3 - 4) - 1",  TT_INTEGER, -2 },
		    { "(4 - 2) - 2", TT_INTEGER, 0 },
		    { "(((1 - 0.5) - 0.25) - 0.125) - 0.075",  TT_REAL,
		    				1 - 0.5 - 0.25 - 0.125 - 0.075		},
		    { NULL, 0, 0 }
		};


    ep = new tlExpression(err, 0, NULL);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, "     Source : %s\n", s);
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		tknDeleteTokenizer(t);
		exp->unref();
    }

    ep->unref();

    return 1;
}

static int
testMult(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    int i;

    struct evaluations evalData[] = {
		    { "3 * 4 * 5",  TT_INTEGER, 3 * 4 * 5 },
		    { "4 * .25", TT_REAL, 1 },
		    { "1 * 0.5 * 0.25 * 0.125 * 0.075",  TT_REAL,
		    				1 * 0.5 * 0.25 * 0.125 * 0.075		},
		    { NULL, 0, 0 }
		};


    ep = new tlExpression(err, 0, NULL);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, "     Source : %s\n", s);
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		tknDeleteTokenizer(t);
		exp->unref();
    }

    ep->unref();

    return 1;
}

static int
testDiv(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    int i;

    struct evaluations evalData[] = {
		    { "3 / 4 / 5",  TT_INTEGER, 3 / 4 / 5 },
		    { "4 / .25", TT_REAL, 16 },
		    { NULL, 0, 0 }
		};


    ep = new tlExpression(err, 0, NULL);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, "     Source : %s\n", s);
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		tknDeleteTokenizer(t);
		exp->unref();
    }

    ep->unref();

    return 1;
}

static int
testComplex(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    int i;

    struct evaluations evalData[] = {
		    { "sin(2 * pi)",  TT_REAL, 0},
		    { "2 ^ 4",  TT_REAL, 16},
		    { NULL, 0, 0 }
		};


    ep = new tlExpression(err, 0, NULL);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, "     Source : %s\n", s);
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		tknDeleteTokenizer(t);
		exp->unref();
    }

    ep->unref();

    return 1;
}


static int
testVariables(tlErrorManager *err)
{
    const char *s;
    tokenizer *t;
    tlExpression *ep;
    tlExpression::Expression *exp;
    tlSrString p;
    tlExpression::Number value;
    tlExpression::DefinitionVariable *variables[4];
    int i;

    struct evaluations evalData[] = {
		    { "pi - (2 * halfpi)",  TT_REAL, 0},
//		    { "k + r",  TT_REAL, 101.25},
//		    { "(r * 2)^2",  TT_REAL, 6.25},
		    { NULL, 0, 0 }
		};

    variables[0] = new tlExpression::DefinitionVariable("k", TT_INTEGER);
    variables[0]->value_.data_.ival_ = 100;

    variables[1] = new tlExpression::DefinitionVariable("halfpi", TT_REAL);
    variables[1]->value_.data_.dval_ = (M_PI / 2.0);

    variables[2] = new tlExpression::DefinitionVariable("r", TT_REAL);
    variables[2]->value_.data_.dval_ = 1.25;

    variables[3] = new tlExpression::DefinitionVariable("q", TT_REAL);
    variables[3]->value_.data_.dval_ = 2.5;


    ep = new tlExpression(err, 4, variables);
    ep->ref();

    for (i = 0; evalData[i].expression_ != NULL; i++) {

		s = evalData[i].expression_;
		t = tknGetStringTokenizer(s);
		DBG(__FILE__, __LINE__, "     Source : %s\n", s);

		exp = ep->parseExpression(t);
		exp->ref();

		if (exp != NULL) {
		    p = exp->toString();
		    DBG(__FILE__, __LINE__, " Expression : %s\n", p.getValue());

		    value = exp->calculateExpression();

		    if (value.type_ == evalData[i].type) {
				PASS(__FILE__, __LINE__, "types match\n");
		    } else {
				FAIL(__FILE__, __LINE__, "type mismatch %s -vs- %s\n",
				    		value.type_ == TT_INTEGER ? "int" :
				    		value.type_ == TT_REAL ? "double" : "bad-type",
				    		evalData[i].type == TT_INTEGER ? "int" :
				    		evalData[i].type == TT_REAL ? "double" : "bad-type");
		    }

		    if (value.type_ == TT_INTEGER) {
				DBG(__FILE__, __LINE__, " int Result : %d\n", value.data_.ival_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.ival_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "int values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "int value mismatch %f -vs- %d\n",
								evalData[i].expectedValue_,
								value.data_.ival_);
				}

		    } else if (value.type_ == TT_REAL) {
				DBG(__FILE__, __LINE__, "real Result : %f\n",
								value.data_.dval_);
				if (fabs(evalData[i].expectedValue_
								- value.data_.dval_) < 0.0025) {
				    PASS(__FILE__, __LINE__, "real values match\n");
				} else {
				    FAIL(__FILE__, __LINE__, "real value mismatch %f -vs- %f\n",
								evalData[i].expectedValue_,
								value.data_.dval_);
				}
		    } else {
				FAIL(__FILE__, __LINE__, "bad type in value\n");
		    }
		}

		tknDeleteTokenizer(t);
		exp->unref();
    }

    ep->unref();

    return 1;
}

int
testExpressions()
{
    tlErrorManager *err;
    int status = 1;


    err = new tlErrorManager();
    err->ref();

    status = testQC(err) && status;
    status = testPlus(err) && status;
    status = testMinus(err) && status;
    status = testMult(err) && status;
    status = testDiv(err) && status;
    status = testComplex(err) && status;
    status = testVariables(err) && status;

    err->unref();

    return status;
}

