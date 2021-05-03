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
 * $Id: tlSrValue.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_VALUE_WRAP_HEADER__
#define		__TOOL_VALUE_WRAP_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
#endif

#include "os_defs.h"

#include "tlSrString.h"

#include "tokens.h"

class tlValue_;


/**
 * define the floating-point precision for the library
 */
typedef float tlReal;

/**
CLASS
		tlValueStorage

	Union for storage of the three basic types.
*/
union tlValueStorage {
public:
	tlSrString *str_;
	int ival_;
	tlReal dval_;
};


/**
CLASS
		tlSrValue

	A wrapper for a typed data value for use in tlTuple and tlTable.
	This type will reference itself on copy, so it can be treated
	like a base type as far as memory management goes, even though
	the internal data values may be complex.
*/
class OS_EXPORT tlSrValue
{
public:
	////////////////////////////////////////
	// Values for the type indicator.
	enum Type {
		NONE		= 0,
		STRING		= TT_STRING,
		INTEGER		= TT_INTEGER,
		REAL		= TT_REAL
	};

public:
	class tlValue_ : public tlRef
	{
	protected:

		Type type_;
		tlValueStorage union_;

	public:
		////////////////////////////////////////
		// Constructor
		tlValue_();

		////////////////////////////////////////
		// Constructor
		tlValue_(tlSrValue value);

		////////////////////////////////////////
		// Constructor
		tlValue_(int ival);

		////////////////////////////////////////
		// Constructor
		tlValue_(tlReal dval);

		////////////////////////////////////////
		// Constructor
		tlValue_(const char *str);

		////////////////////////////////////////
		// Constructor
		tlValue_(tlSrString str);

	protected:
		~tlValue_();

		////////////////////////////////////////
		// Return the typename for
		// reference debugging
		const char *clsId() const;

	public:

		////////////////////////////////////////
		// get the type
		int getType() const;

		////////////////////////////////////////
		// return the value as a string
		tlSrString getStringValue() const;

		////////////////////////////////////////
		// return the value as an int
		int getIntegerValue() const;

		////////////////////////////////////////
		// return the value as a tlReal
		tlReal getRealValue() const;

		////////////////////////////////////////
		// Set from a companion value
		void set(tlSrValue v);

		////////////////////////////////////////
		// Set to a string value
		void set(tlSrString s);

		////////////////////////////////////////
		// Set to a string value
		void set(const char *s);

		////////////////////////////////////////
		// Set to an integer value
		void set(int ival);

		////////////////////////////////////////
		// Set to a tlReal value
		void set(tlReal dval);

		////////////////////////////////////////
		// Clear the old contents of the value
		void clear();

		////////////////////////////////////////
		// Return a pointer to the internal storage
		const tlValueStorage *storage() const;

		////////////////////////////////////////
		// Our friends are our wrapper for passing
		// out of the library, and the system
		// which sets us up.
		friend class tlSrValue;
	};

public:
	////////////////////////////////////////
	// Construct with an object to point to
	tlSrValue(tlValue_ *result);

	////////////////////////////////////////
	// Construct using the given storage value
	tlSrValue(tlValueStorage *storage);

	////////////////////////////////////////
	// Construct with null internal object
	tlSrValue();

	////////////////////////////////////////
	// Copy constructor
	tlSrValue(const tlSrValue &sibling);

	////////////////////////////////////////
	// Construct with string value
	tlSrValue(tlSrString s);

	////////////////////////////////////////
	// Construct with string value
	tlSrValue(const char *s);

	////////////////////////////////////////
	// Construct with integer value
	tlSrValue(int i);

	////////////////////////////////////////
	// Construct with tlReal value
	tlSrValue(tlReal d);

	////////////////////////////////////////
	// Destructor.
	~tlSrValue();

	////////////////////////////////////////
	// assignment, incrementing ref count
	tlSrValue& operator= (const tlSrValue &s);

	////////////////////////////////////////
	// return the length of the vector
	int getType() const;

	////////////////////////////////////////
	// return the value as a string
	tlSrString getStringValue() const;

	////////////////////////////////////////
	// return the value as an int
	int getIntegerValue() const;

	////////////////////////////////////////
	// return the value as a tlReal
	tlReal getRealValue() const;

	////////////////////////////////////////
	// Set from a companion value
	void set(tlSrValue v);

	////////////////////////////////////////
	// Set to a string value
	void set(tlSrString s);

	////////////////////////////////////////
	// Set to a string value
	void set(const char *s);

	////////////////////////////////////////
	// Set to an integer value
	void set(int ival);

	////////////////////////////////////////
	// Set to a tlReal value
	void set(tlReal dval);


	////////////////////////////////////////
	// Set this value to nil
	void setNil();

	////////////////////////////////////////
	// Is this value a nil value
	int isNil() const;


	////////////////////////////////////////
	// Clear the old contents of the value
	void clear();

	////////////////////////////////////////
	// return the internally managed value
	tlValue_ *data() const;

	////////////////////////////////////////
	// print out our value on the given stream
	int printValue(FILE *ofp, int columnWidth = -1) const;

	////////////////////////////////////////
	// return the name of a valid type
	static tlSrString sGetTypeName(int type);

public:
	////////////////////////////////////////
	// Common value for all nil values to
	// point to
	static tlSrValue nil;

protected:
	// the object we are a handle to
	tlValue_ *data_;
};

inline int tlSrValue::getType() const
{
	if (data_ != NULL)
		return data_->getType();
	return NONE;
}

inline tlSrString tlSrValue::getStringValue() const
{
	if (data_ != NULL)
		return data_->getStringValue();

	return tlSrString("NIL");
}

inline int tlSrValue::getIntegerValue() const
{
	if (data_ != NULL)
		return data_->getIntegerValue();

	return 0;
}

inline tlReal tlSrValue::getRealValue() const
{
	if (data_ != NULL)
		return data_->getRealValue();

	return 0.0;
}

inline void tlSrValue::set(tlSrValue v)
{
	if (data_ != NULL)
		data_->set(v);
	else {
		data_ = new tlValue_(v);
		MSG_ASSERT(data_ != NULL, "Out of memory");
		data_->ref();
	}
}

inline void tlSrValue::set(tlSrString s)
{
	if (data_ != NULL)
		data_->set(s);
	else {
		data_ = new tlValue_(s);
		MSG_ASSERT(data_ != NULL, "Out of memory");
		data_->ref();
	}
}

inline void tlSrValue::set(const char *s)
{
	if (data_ != NULL)
		data_->set(s);
	else {
		data_ = new tlValue_(s);
		MSG_ASSERT(data_ != NULL, "Out of memory");
		data_->ref();
	}
}

inline void tlSrValue::set(int i)
{
	if (data_ != NULL)
		data_->set(i);
	else {
		data_ = new tlValue_(i);
		MSG_ASSERT(data_ != NULL, "Out of memory");
		data_->ref();
	}
}

inline void tlSrValue::set(tlReal d)
{
	if (data_ != NULL)
		data_->set(d);
	else {
		data_ = new tlValue_(d);
		MSG_ASSERT(data_ != NULL, "Out of memory");
		data_->ref();
	}
}

inline void tlSrValue::clear()
{
	if (data_ != NULL)
		data_->clear();
}

inline tlSrValue::tlValue_ *tlSrValue::data() const
{
	return data_;
}

#endif /* __TOOL_VALUE_WRAP_HEADER__ */

