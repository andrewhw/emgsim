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
 * $Id: tlSrString.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_STRING_HANDLE_HEADER__
#define		__TOOL_STRING_HANDLE_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlRef.h"

#ifdef		TL_UNICODE
# include "tlUnicodeConvert.h"
#endif

/**
CLASS
		tlSrString

	A smart self-reffing string.  This class can be
	created as a local variable and treated as a
	string -- the class itself is a wrapper to an
	allocated array object which contains the actual
	data.

	<p>
	Reference-counting is managed internally so that
	the internal buffer is deallocated when the last
	reference to this object disappears; the strings
	can therefore safely be locally created and destroyed,
	without the need to new/delete or ref()/unref()
	them.  Essentially, objects of type tlSrString
	can be treated as simple basetype objects (such as
	int, float, <i>etc</i>)as far as allocation,
	deletion and assignment are concerned.

	<p>
	To format one of these objects, the sprintf() interface
	is provided.
*/
class OS_EXPORT tlSrString
{
public:

	////////////////////////////////////////
	// Construct with an initial data value;
	// the <b>stringVal</b> argument is copied
	// and managed internally.
	tlSrString(const char *stringVal);

	////////////////////////////////////////
	// Construct with Nil value.
	tlSrString();

	////////////////////////////////////////
	// Copy constructor -- internal string
	// storage will be common between the
	// two siblings
	tlSrString(const tlSrString &sibling);

	////////////////////////////////////////
	// Destructor.  Internal string is only
	// deleted if this is the last handle
	// to it.
	~tlSrString();

	////////////////////////////////////////
	// Make this object share the internal
	// storage of <b>sibling</b>, managing
	// the reference count for these objects
	// intelligently.
	tlSrString& operator= (const tlSrString &sibling);

	////////////////////////////////////////
	// Comparison between two strings, return true
	// if both internal buffers contain the
	// same character string.
	friend int operator== (
		    const tlSrString &s1,
		    const tlSrString &s2
		);

	////////////////////////////////////////
	// Comparison between two strings, return true
	// if the internal buffer contains the
	// same character string as that provided
	// in <b>s2</b>
	friend int operator== (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// Comparison between two strings, return true
	// if both internal buffers contain
	// different character string.
	friend int operator!= (
		    const tlSrString &s1,
		    const tlSrString &s2
		);

	////////////////////////////////////////
	// Comparison between two strings, return true
	// if the internal buffer contains a
	// different character string than that provided
	// in <b>s2</b>
	friend int operator!= (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator<= (
		    const tlSrString &s1,
		    const tlSrString &s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator>= (
		    const tlSrString &s1,
		    const tlSrString &s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator< (
		    const tlSrString &s1,
		    const tlSrString &s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator> (
		    const tlSrString &s1,
		    const tlSrString &s2
		);


	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator<= (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator>= (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator< (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// Comparison between two strings based
	// on ordinality
	friend int operator> (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// return a new string which is the
	// composite of the two arguments
	friend tlSrString operator+ (
		    const tlSrString &s1,
		    const tlSrString &s2
		);

	////////////////////////////////////////
	// return a new string which is the
	// composite of the two arguments
	friend tlSrString operator+ (
		    const tlSrString &s1,
		    const char *s2
		);

	////////////////////////////////////////
	// Add the sibling's data onto this string
	tlSrString& operator+= (const tlSrString &sibling);

	////////////////////////////////////////
	// Add the given string data onto this string
	tlSrString& operator+= (const char *moreData);

	////////////////////////////////////////
	// Convert on the fly to a char *
	operator const char *() const;

#ifdef		TL_UNICODE
	////////////////////////////////////////
	// Convert on the fly to a tlUNICODE *
	// (A
	// <a href="http://www.unicode.org">Unicode</a>
	// character)
	operator const tlUNICODE *() const;

	////////////////////////////////////////
	// Return the
	// <a href="http://www.unicode.org">Unicode</a>
	// character representing the given
	// character position
	const tlUNICODE operator[] (int index);
#endif

	////////////////////////////////////////
	// Return the internal char* buffer
	// for printing, interaction with other
	// libs, <i>etc</i>.
	const char *getValue() const;

#ifdef		TL_UNICODE
	////////////////////////////////////////
	// Return the internal 
	// <a href="http://www.unicode.org">Unicode</a>
	// buffer for printing, interaction with other
	// libs, <i>etc</i>.
	const tlUNICODE *getUValue() const;
#endif

	////////////////////////////////////////
	// Return the length of the internal string.
	// Note that this length is managed statically
	// internally, so this is a cheap call;
	// that is, not as expensive as strlen().
	int getLength() const;

#ifdef		TL_UNICODE
	////////////////////////////////////////
	// Return the length of the internal
	// <a href="http://www.unicode.org">Unicode</a>
	// representation string.
	// Note that this length is managed statically
	// internally, so this is a cheap call;
	// that is, not as expensive as strlen().
	int getULength() const;
#endif

	////////////////////////////////////////
	// Throw away the old internal representation
	// and set it to be a copy of <b>newValue</b>
	void setValue(const char *newValue);

	////////////////////////////////////////
	// Throw away the old internal representation
	// and set it to be shared with <b>newValue</b>
	void setValue(tlSrString newValue);

#ifdef		TL_UNICODE
	////////////////////////////////////////
	// Throw away the old internal representation
	// and set it to be a copy of the unicode value newValue
	void setValue(const tlUNICODE *newValue);
#endif

	////////////////////////////////////////
	// Throw away the old internal representation
	// and set it to be a copy of <b>newValue</b>,
	// with a maximum of <b>maxLen</b> characters.
	void setValueMaxLen(const char *newValue, int maxLen);

	////////////////////////////////////////
	// Returns true if the length of the string is zero
	int isEmpty() const;

	////////////////////////////////////////
	// sets the value to a "nice double" representation of
	// the argument (trailing zeros removed, leaving one
	// after decimal place if no other digits would remain)
	tlSrString niceDouble(double d);

	////////////////////////////////////////
	// Trims all zeros following the decimal
	tlSrString fullyTrimmedDouble(double d);

	////////////////////////////////////////
	// Get a version of the contents
	// quoted and ready for inclusion
	// in an escape protected file.
	// <p>
	// There will be double quotes at the ends
	// of this new string, and all the following
	// characters will be preceeded by '\'
	// characters:
	// <ul>
	// <li>'\'
	// <li>"
	// <li>[TAB]
	// <li>[NEWLINE]
	// </ul>
	tlSrString getQuoted() const;

	////////////////////////////////////////
	// Set the value from the sort of string
	// generated by the getQuoted() function.
	// <p>
	// The resultant internal represenation
	// will have deflated the escape sequences
	// introduced by getQuoted().
	void setQuoted(tlSrString s);

	////////////////////////////////////////
	// Set the value from the sort of string
	// generated by the getQuoted() function
	// <p>
	// The resultant internal represenation
	// will have deflated the escape sequences
	// introduced by getQuoted().
	void setQuoted(const char *s);

	////////////////////////////////////////
	// return a lowercase version of the string
	tlSrString toLower() const;

	////////////////////////////////////////
	// return an uppercase version of the string
	tlSrString toUpper() const;

	////////////////////////////////////////
	// return the first index of any of the
	// characters in <b>match</b>
	// or (-1) if no match
	int containsAny(const char *match) const;

	////////////////////////////////////////
	// return the index of the
	// substring in <b>match</b>
	// or (-1) if no match
	int contains(const char *match) const;

	////////////////////////////////////////
	// return the index of the
	// character in <b>match</b>
	// or (-1) if no match
	int contains(char match) const;

	////////////////////////////////////////
	// format interface.  The results of the
	// sprintf end up in the tlSrString the
	// function is called on.  Space is internally
	// allocated for all arguments.
	// <p>
	// The major part of the format specifier
	// set for standard printf() are supported,
	// including:
	// <ul>
	// <li>width
	// <li>precision
	// <li>'*' operators
	// <li>The integer types d, i, o, u, x, X,
	//     character type c,
	//     floating types f, g, G, e, E,
	//     and string types s, and an extra string type 'S'
	//     for arguments of
	//     type tlSrString *.
	// </ul>
	int sprintf(const char *formatString, ...);

	////////////////////////////////////////
	// format interface.  Used by sprintf()
	// above, as well as available for use
	// like vsprintf() etc in libc.
	int vsprintf(const char *formatString, va_list args);

protected:
	class tlString_ : public tlRef
	{
	public:
		char *s_;
		int sLen_;

#ifdef		TL_UNICODE
		tlUNICODE *u_;
		int uLen_;

		tlString_(const tlUNICODE *data);
#endif

		tlString_(const char *data);
		tlString_(const char *data, int max);

		~tlString_();

		friend class tlSrString;
	};

	// the object we are a handle to
	tlString_ *data_;

#ifdef		TL_UNICODE
	static const tlUNICODE sUnicodeUnknown_[];
#endif

public:
	////////////////////////////////////////
	// Statically declared string which is
	// guaranteed to be 'nil'.  This is
	// provided for assignment and comparison
	// purposes.
	static tlSrString nil;

protected:
	////////////////////////////////////////
	// clean up static refs
	static void sExitCleanup();

public:
	friend class tlRefManager;
};

inline const char *tlSrString::getValue() const
{
	if (data_ != NULL)
		return data_->s_;
	return "<unknown>";
}

inline int tlSrString::getLength() const
{
	if (data_ != NULL)
		return data_->sLen_;
	return 0;
}

inline int tlSrString::isEmpty() const
{
	if (data_ == NULL)
		return 1;

	return (data_->sLen_ == 0) ? 1 : 0;
}

inline void tlSrString::setValue(tlSrString s)
{
	*this = s;
}

inline tlSrString::operator const char * () const
{
	return getValue();
}

#ifdef		TL_UNICODE
inline const tlUNICODE *tlSrString::getUValue() const
{
	if (data_ != NULL)
		return data_->u_;
	return sUnicodeUnknown_;
}

inline int tlSrString::getULength() const
{
	if (data_ != NULL)
		return data_->uLen_;
	return 0;
}

inline tlSrString::operator const tlUNICODE * () const
{
	return getUValue();
}
#endif

#endif /* __TOOL_STRING_HANDLE_HEADER__ */

