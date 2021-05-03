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
 * $Id: tlSrValue.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
#endif

#include "stringtools.h"

#include "tlSrValue.h"


tlSrValue tlSrValue::nil;

tlSrValue::tlSrValue()
{
	data_ = NULL;
}

tlSrValue::tlSrValue(tlValue_ *obj)
{
	data_ = obj;
	if (data_ != NULL)
	{
		data_->ref();
	}
}

tlSrValue::tlSrValue(const tlSrValue &sibling)
{
	if (sibling.data_ != NULL)
		++sibling.data_->refCount_;
	data_ = sibling.data_;
}

tlSrValue::tlSrValue(int i)
{
	data_ = new tlValue_(i);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	data_->ref();
}

tlSrValue::tlSrValue(tlReal d)
{
	data_ = new tlValue_(d);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	data_->ref();
}

tlSrValue::tlSrValue(const char *s)
{
	data_ = new tlValue_(s);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	data_->ref();
}

tlSrValue::tlSrValue(tlSrString s)
{
	data_ = new tlValue_(s);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	data_->ref();
}

tlSrValue::~tlSrValue()
{
	if (data_ != NULL)
	{
		data_->unref();
	}
}

tlSrValue &
tlSrValue::operator= (const tlSrValue &sibling)
{
	if (sibling.data_ != NULL)
		++sibling.data_->refCount_;

	if (data_ != NULL)
	{
		data_->unref();
	}
	data_ = sibling.data_;

	return *this;
}

tlSrValue::tlValue_::tlValue_()
{
	type_ = tlSrValue::NONE;
}

tlSrValue::tlValue_::tlValue_(tlSrValue v)
{
	type_ = tlSrValue::NONE;
	set(v);
}

tlSrValue::tlValue_::tlValue_(int i)
{
	type_ = tlSrValue::NONE;
	set(i);
}

tlSrValue::tlValue_::tlValue_(tlReal d)
{
	type_ = tlSrValue::NONE;
	set(d);
}

tlSrValue::tlValue_::tlValue_(const char *s)
{
	type_ = tlSrValue::NONE;
	set(s);
}

tlSrValue::tlValue_::tlValue_(tlSrString s)
{
	type_ = tlSrValue::NONE;
	set(s);
}

tlSrValue::tlValue_::~tlValue_()
{
	clear();
}

const char *
tlSrValue::tlValue_::clsId() const
{
	return "tlValue_";
}

void
tlSrValue::tlValue_::clear()
{
	if (type_ == tlSrValue::STRING)
	{
		delete union_.str_;
	}
	union_.ival_ = 0;
	type_ = tlSrValue::NONE;
}

void
tlSrValue::tlValue_::set(tlSrValue v)
{
	clear();
	type_ = v.data()->type_;
	if (type_ == tlSrValue::STRING)
	{
		union_.str_ = new tlSrString(*v.data()->union_.str_);
		MSG_ASSERT(union_.str_ != NULL, "Out of memory");
	} else
	{
		memcpy(&union_, &v.data()->union_, sizeof(union_));
	}
}

void
tlSrValue::tlValue_::set(int i)
{
	clear();
	type_ = tlSrValue::INTEGER;
	union_.ival_ = i;
}

void
tlSrValue::tlValue_::set(tlReal d)
{
	clear();
	type_ = tlSrValue::REAL;
	union_.dval_ = d;
}

void
tlSrValue::tlValue_::set(const char *s)
{
	clear();
	type_ = tlSrValue::STRING;
	union_.str_ = new tlSrString(s);
	MSG_ASSERT(union_.str_ != NULL, "Out of memory");
}

void
tlSrValue::tlValue_::set(tlSrString s)
{
	clear();
	type_ = tlSrValue::STRING;
	union_.str_ = new tlSrString(s);
	MSG_ASSERT(union_.str_ != NULL, "Out of memory");
}

int
tlSrValue::tlValue_::getType() const
{
	return type_;
}

void
tlSrValue::setNil()
{
	*this = nil;
}

int
tlSrValue::isNil() const
{
	if (data_ == nil.data_)
	{
		return 1;
	}
	return 0;
}

int
tlSrValue::tlValue_::getIntegerValue() const
{
	if (type_ == tlSrValue::INTEGER)
	{
		return union_.ival_;
	}

	if (type_ == tlSrValue::REAL)
	{
		return (int) union_.dval_;
	}

	return 0;
}

tlReal
tlSrValue::tlValue_::getRealValue() const
{
	if (type_ == tlSrValue::REAL)
	{
		return union_.dval_;
	}

	if (type_ == tlSrValue::INTEGER)
	{
		return (tlReal) union_.ival_;
	}

	return 0.0;
}

tlSrString
tlSrValue::tlValue_::getStringValue() const
{
	if (type_ == tlSrValue::STRING)
	{
		return *union_.str_;
	} else
	{
		/** create an empty string and populate it if valid data */
		tlSrString val;

		if (type_ == tlSrValue::REAL)
		{
			val.sprintf("%f", union_.dval_);

		} else if (type_ == tlSrValue::INTEGER)
		{
			val.sprintf("%d", union_.ival_);

		} else
		{
			val.setValue("<unknown>");
		}

		return val;
	}
	/** NOTREACHED */
}

int
tlSrValue::printValue(FILE *ofp, int columnWidth) const
{
	tlSrString s;

	if (getType() == tlSrValue::STRING)
	{
		s.sprintf("\"%s\"", getStringValue().getValue());
	} else
	{
		s = getStringValue().getValue();
	}
	if (columnWidth > 0)
	{
		fprintf(ofp, "%*s", columnWidth, s.getValue());
	} else
	{
		fprintf(ofp, "%s", s.getValue());
	}
	return !ferror(ofp);
}

tlSrString
tlSrValue::sGetTypeName(int type)
{
	if (type == tlSrValue::NONE)
	{
		return tlSrString("no type");

	} else if (type == tlSrValue::STRING)
	{
		return tlSrString("string");

	} else if (type == tlSrValue::INTEGER)
	{
		return tlSrString("integer");

	} else if (type == tlSrValue::REAL)
	{
		return tlSrString("real");

	}

	{
		tlSrString s;
		s.sprintf("<invalid type %d>", type);
		return s;
	}
}

const tlValueStorage *
tlSrValue::tlValue_::storage() const
{
	return &union_;
}

