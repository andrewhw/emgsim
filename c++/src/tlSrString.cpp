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
 * $Id: tlSrString.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
#endif

#include "os_defs.h"
#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"

#include "tlSrString.h"


#define				MAX_NUMBER_LENGTH		128

#define FORMAT_TYPE_NOTYPE				0x01
#define FORMAT_TYPE_FLOAT				0x02
#define FORMAT_TYPE_DECIMAL				0x03
#define FORMAT_TYPE_OCTAL				0x04
#define FORMAT_TYPE_HEX						0x05
#define FORMAT_TYPE_STRING				0x06
#define FORMAT_TYPE_CHAR				0x07
#define FORMAT_TYPE_POINTER				0x08
#define FORMAT_TYPE_PERCENT				0x09


#define		DTYPE_NOTYPE						0x01
#define DTYPE_INTEGER						0x02
#define DTYPE_UNSIGNED						0x03
#define DTYPE_DOUBLE						0x04
#define DTYPE_CHAR						0x05
#define DTYPE_STRING						0x06
#define DTYPE_TL_STRING						0x07
#define DTYPE_POINTER						0x08

// output flags to control pretty printing
#define OFLAG_CLEAR						0x0000
#define OFLAG_PAD_ZERO						0x0001
#define OFLAG_LEFT_JUST						0x0002
#define OFLAG_FORCE_SIGN				0x0004
#define OFLAG_SPACE_SIGN				0x0008
#define OFLAG_UPPERCASE						0x0010
#define OFLAG_EXPONENT						0x0020
#define OFLAG_VAREXP						0x0040
#define OFLAG_HAS_PREC						0x0080
#define OFLAG_USE_ALTERNATE				0x0100
#define OFLAG_WIDTH_LOADED				0x0200
#define OFLAG_PREC_LOADED				0x0400
#define OFLAG_BAD_FORMAT				0x8000

    // types of storage size specification
#define STFLAG_CLEAR						0x01
#define STFLAG_IS_LONG						0x02
#define STFLAG_IS_LONG_LONG				0x03
#define STFLAG_IS_SHORT						0x04


typedef struct SpecifierDescription
{
		int width_;
		int precision_;
		int frmtType_;
		int dataType_;
		int storageSpec_;
		int outFlags_;
} SpecifierDescription;

typedef struct OutputBuffer
{
		char *buffer_;
		int length_;
		int blocks_;
} OutputBuffer;


/** access for static global "nil" string */
tlSrString tlSrString::nil("");

#ifdef		TL_UNICODE
const tlUNICODE tlSrString::sUnicodeUnknown_[] = {
    				0x00, 'u',
    				0x00, 'n',
    				0x00, 'k',
    				0x00, 'n',
    				0x00, 'o',
    				0x00, 'w',
    				0x00, 'n',
    				0x00, 0x00
    		};
#endif

void
tlSrString::sExitCleanup()
{
#ifdef		OS_WINDOWS_NT
# ifdef OS_WINDOWS_DELETE_NIL
	if (tlSrString::nil.data_ != NULL)
	{
		/**
		 * This is protected as on MFC programs it is difficult
		 * to determine whether or not sExitCleanup may be
		 * being called _after_ the nil string cleanup that
		 * occurs statically may already be done.  See also
		 * the "set to null" in tlSrString's destructor that
		 * is only required to maintain this if statement.
		 */
		tlSrString::nil.data_->unref();
	}
# endif
#else
	tlSrString::nil.data_->unref();
#endif
	tlSrString::nil.data_ = NULL;
}

tlSrString::tlSrString(const char *s)
{
	data_ = new tlString_(s);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	if (data_ != NULL)
	{
		data_->ref();
	}
}

tlSrString::tlSrString(const tlSrString &sibling)
{
	if (sibling.data_ != NULL)
		++sibling.data_->refCount_;
	data_ = sibling.data_;
}

tlSrString::tlSrString()
{
	if (nil.data_ != NULL)
		++nil.data_->refCount_;
	data_ = nil.data_;
}

tlSrString::~tlSrString()
{
	if (data_ != NULL)
	{
		data_->unref();
#ifdef OS_WINDOWS_NT
		data_ = NULL;
#endif
	}
}

tlSrString &
tlSrString::operator= (const tlSrString &sibling)
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

int
operator== (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	if (s1.data_ == s2.data_)
		return 1;
	return !strcmp(s1.data_->s_, s2.data_->s_);
}

int
operator!= (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	if (s1.data_ == s2.data_)
		return 0;
	return strcmp(s1.data_->s_, s2.data_->s_);
}

int
operator>= (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	int result;
	if (s1.data_ == s2.data_)
		return 1;
	result = strcmp(s1.data_->s_, s2.data_->s_);
	if (result >= 0)
		return 1;
	return 0;
}

int
operator<= (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	int result;
	if (s1.data_ == s2.data_)
		return 1;
	result = strcmp(s1.data_->s_, s2.data_->s_);
	if (result <= 0)
		return 1;
	return 0;
}

int
operator> (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	int result;
	if (s1.data_ == s2.data_)
		return 0;
	result = strcmp(s1.data_->s_, s2.data_->s_);
	if (result > 0)
		return 1;
	return 0;
}

int
operator< (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	int result;
	if (s1.data_ == s2.data_)
		return 0;
	result = strcmp(s1.data_->s_, s2.data_->s_);
	if (result < 0)
		return 1;
	return 0;
}

tlSrString
operator+ (
		const tlSrString &s1,
		const tlSrString &s2
    )
{
	tlSrString result;
	result.sprintf("%s%s", s1.getValue(), s2.getValue());
	return result;
}


tlSrString &
tlSrString::operator+= (const tlSrString &sibling)
{
	this->sprintf("%s%s", this->getValue(), sibling.getValue());

	return *this;
}

tlSrString &
tlSrString::operator+= (const char *moreData)
{
	this->sprintf("%s%s", this->getValue(), moreData);

	return *this;
}

/** ---------------------------------------------------------------- */
int
operator== (
		const tlSrString &s1,
		const char *s2
    )
{
	return !strcmp(s1.data_->s_, s2);
}

int
operator!= (
		const tlSrString &s1,
		const char *s2
    )
{
	return strcmp(s1.data_->s_, s2);
}

int
operator>= (
		const tlSrString &s1,
		const char *s2
    )
{
	int result;
	result = strcmp(s1.data_->s_, s2);
	if (result >= 0)
		return 1;
	return 0;
}

int
operator<= (
		const tlSrString &s1,
		const char *s2
    )
{
	int result;
	result = strcmp(s1.data_->s_, s2);
	if (result <= 0)
		return 1;
	return 0;
}

int
operator> (
		const tlSrString &s1,
		const char *s2
    )
{
	int result;
	result = strcmp(s1.data_->s_, s2);
	if (result > 0)
		return 1;
	return 0;
}

int
operator< (
		const tlSrString &s1,
		const char *s2
    )
{
	int result;
	result = strcmp(s1.data_->s_, s2);
	if (result < 0)
		return 1;
	return 0;
}

tlSrString
operator+ (
		const tlSrString &s1,
		const char *s2
    )
{
	tlSrString result;
	result.sprintf("%s%s", s1.getValue(), s2);
	return result;
}

tlSrString::tlString_::tlString_(const char *s)
{
	s_ = ckstrdup(s);
	sLen_ = strlen(s_);

#ifdef		TL_UNICODE
	u_ = tlUTFToUnicode(s);
	uLen_ = tlUnicodeStrlen(u_);
#endif
}

tlSrString::tlString_::tlString_(const char *s, int max)
{
	s_ = ckstrndup(s, max);
	sLen_ = strlen(s_);

#ifdef		TL_UNICODE
	u_ = tlUTFToUnicode(s);
	uLen_ = tlUnicodeStrlen(u_);
#endif
}

#ifdef		TL_UNICODE
tlSrString::tlString_::tlString_(const tlUNICODE *u)
{
	uLen_ = tlUnicodeStrlen(u);
	u_ = (tlUNICODE *) ckalloc(sizeof(tlUNICODE) * (uLen_ + 1));
	memcpy(u_, u, uLen_);

	s_ = tlUnicodeToUTF(u);
	sLen_ = strlen(s_);
}
#endif

tlSrString::tlString_::~tlString_()
{
	if (s_ != NULL)
		ckfree(s_);

#ifdef		TL_UNICODE
	if (u_ != NULL)
		ckfree(u_);
#endif
}

void
tlSrString::setValue(const char *s)
{
	if (data_ != NULL)
	{
		data_->unref();
	}

	data_ = new tlString_(s);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	if (data_ != NULL)
	{
		data_->ref();
	}
}

void
tlSrString::setValueMaxLen(const char *s, int maxLen)
{
	if (data_ != NULL)
	{
		data_->unref();
	}

	data_ = new tlString_(s, maxLen);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	if (data_ != NULL)
	{
		data_->ref();
	}
}

#ifdef		TL_UNICODE
void
tlSrString::setValue(const tlUNICODE *u)
{
	if (data_ != NULL)
	{
		data_->unref();
	}

	data_ = new tlString_(u);
	MSG_ASSERT(data_ != NULL, "Out of memory");
	if (data_ != NULL)
	{
		data_->ref();
	}
}

const tlUNICODE
tlSrString::operator[] (int index)
{
//    if ((index >= 0) && (index < data_->uLen_))
//		return data_->u_[index];

	if ((index >= 0) && (index < data_->sLen_))
		return data_->s_[index];

	return 0;
}
#endif


/**
 * this function gets called right after we saw a '%' sign, so we now
 * have to figure out what the specifier is actually about
 *
 * Here we build up the SpecifierDescription specifier, from which we
 * can simply print out all the output info.
 */
static int
parseFormatSpecifier(
		const char *format,
		SpecifierDescription *specifier
    )
{
	int leaveLoop = 0;
	int sawDigit = 0;
	int lengthRead = 1;
	/** we will always convert at least one character **/


	/**
	 * Set flags for no data
	 */
	specifier->outFlags_ = OFLAG_CLEAR;
	specifier->width_ = 0;
	specifier->precision_ = 0;
	specifier->dataType_ = DTYPE_NOTYPE;
	specifier->frmtType_ = FORMAT_TYPE_NOTYPE;
	specifier->storageSpec_ = STFLAG_CLEAR;

	while (*format != '\0')
	{
		if (isascii(*format))
		{
			/**
			 **		actual format specifiers
			 **/
			switch(*format)
			{
			case 'd':
			case 'i':
				specifier->frmtType_ = FORMAT_TYPE_DECIMAL;
				specifier->dataType_ = DTYPE_INTEGER;
				leaveLoop = 1;
				break;


			case 'o':
				specifier->frmtType_ = FORMAT_TYPE_OCTAL;
				specifier->dataType_ = DTYPE_UNSIGNED;
				leaveLoop = 1;
				break;

			case 'u':
				specifier->frmtType_ = FORMAT_TYPE_DECIMAL;
				specifier->dataType_ = DTYPE_UNSIGNED;
				leaveLoop = 1;
				break;

			case 'X':
				specifier->outFlags_ |= OFLAG_UPPERCASE;
				/** fall through **/
			case 'x':
				specifier->frmtType_ = FORMAT_TYPE_HEX;
				specifier->dataType_ = DTYPE_UNSIGNED;
				leaveLoop = 1;
				break;

			case 'f':
				specifier->frmtType_ = FORMAT_TYPE_FLOAT;
				specifier->dataType_ = DTYPE_DOUBLE;
				leaveLoop = 1;
				break;

			case 'E':
				specifier->outFlags_ |= OFLAG_UPPERCASE;
				/** fall through **/
			case 'e':
				specifier->frmtType_ = FORMAT_TYPE_FLOAT;
				specifier->dataType_ = DTYPE_DOUBLE;
				specifier->outFlags_ = OFLAG_EXPONENT;
				leaveLoop = 1;
				break;

			case 'G':
				specifier->outFlags_ |= OFLAG_UPPERCASE;
				/** fall through **/
			case 'g':
				specifier->frmtType_ = FORMAT_TYPE_FLOAT;
				specifier->dataType_ = DTYPE_DOUBLE;
				specifier->outFlags_ |= OFLAG_VAREXP;
				leaveLoop = 1;
				break;

			case 'c':
				specifier->frmtType_ = FORMAT_TYPE_CHAR;
				specifier->dataType_ = DTYPE_CHAR;
				leaveLoop = 1;
				break;

			case 's':
				specifier->frmtType_ = FORMAT_TYPE_STRING;
				specifier->dataType_ = DTYPE_STRING;
				leaveLoop = 1;
				break;

			case 'S':
				specifier->frmtType_ = FORMAT_TYPE_STRING;
				specifier->dataType_ = DTYPE_TL_STRING;
				leaveLoop = 1;
				break;

			case 'p':
				specifier->frmtType_ = FORMAT_TYPE_POINTER;
				specifier->dataType_ = DTYPE_POINTER;
				leaveLoop = 1;
				break;

			case '%':
				specifier->frmtType_ = FORMAT_TYPE_PERCENT;
				leaveLoop = 1;
				break;


			/**
			 **		size modifiers
			 **/
			case 'h':
				if (specifier->storageSpec_ != STFLAG_CLEAR)
					specifier->outFlags_ = OFLAG_BAD_FORMAT;
				specifier->storageSpec_ = STFLAG_IS_SHORT;
				break;

			case 'l':		/** and 'll'		*/
				if (specifier->storageSpec_ == STFLAG_IS_LONG)
					specifier->storageSpec_ = STFLAG_IS_LONG_LONG;
				else
					specifier->storageSpec_ = STFLAG_IS_LONG;
				break;


			/**
			 **		flag modifiers
			 **/
			case '-':
				specifier->outFlags_ |= OFLAG_LEFT_JUST;
				break;

			case '+':
				specifier->outFlags_ |= OFLAG_FORCE_SIGN;
				if (specifier->outFlags_ & OFLAG_SPACE_SIGN)
					specifier->outFlags_ &= ~(OFLAG_SPACE_SIGN);
				break;

			case ' ':
				if (!(specifier->outFlags_ & OFLAG_FORCE_SIGN))
					specifier->outFlags_ |= OFLAG_SPACE_SIGN;
				break;

			case '#':
				specifier->outFlags_ |= OFLAG_USE_ALTERNATE;
				break;

			case '0':
				if (sawDigit == 0)
				{
					specifier->outFlags_ |= OFLAG_PAD_ZERO;
				} else
				{
					/** load up one of the integer based fields **/
					if (!(specifier->outFlags_ & OFLAG_HAS_PREC))
					{
						specifier->width_ = (specifier->width_ * 10);
					} else
					{
						specifier->precision_ =
									(specifier->precision_ * 10);
					}
				}
				sawDigit = 1;
				break;


			case '.':
				specifier->outFlags_ |= OFLAG_HAS_PREC;
				break;

			case '*':		/** load sizes from stack **/
				if (!(specifier->outFlags_ & OFLAG_HAS_PREC))
				{
					specifier->outFlags_ |= OFLAG_WIDTH_LOADED;
				} else
				{
					specifier->outFlags_ |= OFLAG_PREC_LOADED;
				}
				break;


			/**
			 **		flag modifiers
			 **/
			default:
				/** we handle zeros above, but other digits here */
				if (isdigit(*format))
				{
					sawDigit = 1;
					/** load up one of the integer based fields **/
					if (!(specifier->outFlags_ & OFLAG_HAS_PREC))
					{
						specifier->width_ = (specifier->width_ * 10) +
							(*format - '0');
					} else
					{
						specifier->precision_ =
									(specifier->precision_ * 10) +
								(*format - '0');
					}

				} else
				{
					/** flag bad on through on unknown chars **/
					specifier->outFlags_ = OFLAG_BAD_FORMAT;
				}
				break;
			}

			if (leaveLoop)
				break;

		} else
		{
			/**
			 * if we don't understand something,
			 * flag this format as bad
			 */
			specifier->outFlags_ = OFLAG_BAD_FORMAT;
		}
		format++;
		lengthRead++;
	}

	if (*format == '\0')
	{
		specifier->outFlags_ = OFLAG_BAD_FORMAT;
	}

	/** check that default precision is 6 for float **/
	if ((specifier->frmtType_ == FORMAT_TYPE_FLOAT) &&
				(!(specifier->outFlags_ & OFLAG_HAS_PREC)))
	{
		specifier->precision_ = 6;
	}

	return lengthRead;
}


static void
addCharToOutputBuffer(OutputBuffer *buf, char c)
{
	int status;
	
	status = listCheckSize(
				buf->length_ + 2,
				(void **) &buf->buffer_,
				&buf->blocks_,
				64,
				sizeof(char));
	MSG_ASSERT(status, "Out of memory");
	buf->buffer_[ buf->length_++ ] = c;
	buf->buffer_[ buf->length_ ] = 0;
}

static void
addRepeatCharToOutputBuffer(OutputBuffer *buf, char c, int n)
{
	int status, i;

	/** grow the buffer enough for the whole string we are adding */
	status = listCheckSize(
				buf->length_ + n + 1,
				(void **) &buf->buffer_,
				&buf->blocks_,
				64,
				sizeof(char));
	MSG_ASSERT(status, "Out of memory");

	/**
	 * add in n chars & update the length
	 */
	for (i = 0; i < n ; i++)
	{
		buf->buffer_[ buf->length_++] = c;
	}
	buf->buffer_[ buf->length_ ] = 0;
}

static void
addStringToOutputBuffer(OutputBuffer *buf, const char *dataString)
{
	int length, status;
	
	length = strlen(dataString);

	/** grow the buffer enough for the whole dataString we are adding */
	status = listCheckSize(
				buf->length_ + length + 1,
				(void **) &buf->buffer_,
				&buf->blocks_,
				64,
				sizeof(char));
	MSG_ASSERT(status, "Out of memory");

	/** copy in the new dataString and update the length */
	memcpy(&buf->buffer_[ buf->length_ ], dataString, length);
	buf->buffer_[ buf->length_ + length ] = 0;
	buf->length_ += length;
}

// load from a local buffer into the string with padding
static void
loadFromBuffer(
		OutputBuffer *outputBufState,
		const char *localLoad,
		int nPrinted,
		SpecifierDescription *specifierDescription
    )
{
	int		nPad;
	int isPrinted = 0;

	/** add spaces to the buffer **/
	nPad = specifierDescription->width_ - nPrinted;
	if (nPad > 0)
	{
		if (specifierDescription->outFlags_ & OFLAG_LEFT_JUST)
		{
			addStringToOutputBuffer(outputBufState, localLoad);
			isPrinted = 1;
		}
		if (specifierDescription->outFlags_ & OFLAG_PAD_ZERO)
		{
			addRepeatCharToOutputBuffer(outputBufState, '0', nPad);
		} else
		{
			addRepeatCharToOutputBuffer(outputBufState, ' ', nPad);
		}
	}

	if (!isPrinted)
		addStringToOutputBuffer(outputBufState, localLoad);
}

static void
loadInteger(
		OutputBuffer *outputBufState,
		int iVal,
		SpecifierDescription *specifierDescription
    )
{
	int nPrinted = 0;
	int nPad;
	int isNeg = 0;
	char localBuf[MAX_NUMBER_LENGTH];
	char *localLoad;

	/** set up the temp buffer to load from the right **/
	localLoad = &localBuf[MAX_NUMBER_LENGTH - 1];
	*localLoad-- = 0;

	if (iVal < 0)
	{
		isNeg = 1;
		iVal *= (-1);
	}

	/** fill up the buffer **/
	while (iVal > 0)
	{
		*localLoad-- = (char) ('0' + (iVal % 10));
		iVal /= 10;
		nPrinted++;
	}

	/** add in precision **/
	if ((specifierDescription->outFlags_ & OFLAG_HAS_PREC) != 0)
	{
		if (nPrinted < specifierDescription->precision_)
		{
			specifierDescription->precision_ -= nPrinted;
			while (specifierDescription->precision_-- > 0)
			{
				*localLoad-- = '0';
				nPrinted++;
			}
		}
	}

	/** check for 0 value **/
	if (nPrinted == 0)
	{
		*localLoad-- = '0';
		nPrinted++;
	}

	/**
	 ** we have to determine whether we are padding with zeros
	 ** prior to committing to the sign
	 **/
	if (specifierDescription->outFlags_ & OFLAG_PAD_ZERO)
	{
		nPad = (specifierDescription->width_ - nPrinted) - 1;
		while (nPad-- > 0)
		{
			*localLoad-- = '0';
			nPrinted++;
		}
	}

	/** handle the sign **/
	if (isNeg)
	{
		*localLoad-- = '-';
		nPrinted++;

	} else if (specifierDescription->outFlags_ & OFLAG_FORCE_SIGN)
	{
		*localLoad-- = '+';
		nPrinted++;

	} else if (specifierDescription->outFlags_ & OFLAG_SPACE_SIGN)
	{
		*localLoad-- = ' ';
		nPrinted++;
	}

	/**
	 * we have backed up one too many chars, so pre-increment
	 * localLoad
	 */
	loadFromBuffer(
			outputBufState,
			++localLoad,
			nPrinted,
			specifierDescription
		);
}

static void
loadUnsigned(
		OutputBuffer *outputBufState,
		unsigned uVal,
		SpecifierDescription *specifierDescription
    )
{
	int nPrinted = 0;
	int outBase;
	int alternateOk = 0;
	char localBuf[MAX_NUMBER_LENGTH];
	char *localLoad;
	const char *outChars;

	/** set up the temp buffer **/
	localLoad = &localBuf[MAX_NUMBER_LENGTH - 1];
	*localLoad-- = 0;

	/** figure out our base **/
	if (specifierDescription->frmtType_ == FORMAT_TYPE_HEX)
	{

		outBase = 16;
		alternateOk = 1;
		if (specifierDescription->outFlags_ & OFLAG_UPPERCASE)
			outChars = "0123456789ABCDEFX";
		else
			outChars = "0123456789abcdefx";

	} else if (specifierDescription->frmtType_ == FORMAT_TYPE_OCTAL)
	{
		alternateOk = 1;
		outBase = 8;
		outChars = "01234567";

	} else
	{
		outBase = 10;
		outChars = "0123456789";
	}
	

	/** fill up the buffer **/
	while (uVal > 0)
	{
		*localLoad-- = outChars[0 + (uVal % outBase)];
		uVal /= outBase;
		nPrinted++;
	}

	/** add in precision **/
	if (specifierDescription->outFlags_ & OFLAG_HAS_PREC)
	{
		if (nPrinted < specifierDescription->precision_)
		{
			specifierDescription->precision_ -= nPrinted;
			while (specifierDescription->precision_-- > 0)
			{
				*localLoad-- = '0';
				nPrinted++;
			}
		}
	}

	/** check for 0 value **/
	if (nPrinted == 0)
	{
		alternateOk = 0;
		*localLoad-- = '0';
		nPrinted++;
	}

	/** handle the leader if there **/
	if ((alternateOk) &&
					(specifierDescription->outFlags_
								& OFLAG_USE_ALTERNATE))
	{
		if (specifierDescription->frmtType_ == FORMAT_TYPE_HEX)
		{
			*localLoad-- = outChars[outBase];
			nPrinted++;
		}
		*localLoad-- = '0';
		nPrinted++;
	}

	loadFromBuffer(
			outputBufState,
			++localLoad,
			nPrinted,
			specifierDescription
		);
}

static void
loadDouble(
		OutputBuffer *outputBufState,
		double dVal,
		SpecifierDescription *specifierDescription
    )
{
	int nPrinted = 0;
	int nPad;
	int isNeg = 0;
	const char *fmt;
	char localFormatBuf[MAX_NUMBER_LENGTH];
	char localPadBuf[2 * MAX_NUMBER_LENGTH];
	char *localLoad;

	if (dVal < 0.0)
	{
		isNeg = 1;
		dVal *= (-1);
	}

	/**
	 ** set up the temp buffer -- we may as well not figure out this
	 **		horrible thing all over again!
	 **/
	if (specifierDescription->outFlags_ & OFLAG_EXPONENT)
	{
		fmt = "%.*e";

	} else if (specifierDescription->outFlags_ & OFLAG_VAREXP)
	{
		fmt = "%.*g";

	} else
	{
		fmt = "%.*f";
	}

	/** first print it based on type */
	slnprintf(localFormatBuf, MAX_NUMBER_LENGTH, fmt,
					specifierDescription->precision_, dVal);

	/**
	 * now move to the right-most location in the buffer
	 * so that we can pad it
	 */
	nPrinted = strlen(localFormatBuf);
	localLoad = &localPadBuf[(2 * MAX_NUMBER_LENGTH) - (nPrinted + 1)];
	strlcpy(localLoad, localFormatBuf, nPrinted+1);
	localLoad--;


	/**
	 * we have to determine whether we are padding with zeros
	 * prior to committing to the sign
	 */
	if (specifierDescription->outFlags_ & OFLAG_PAD_ZERO)
	{
		nPad = (specifierDescription->width_ - nPrinted) - 1;
		while (nPad-- > 0)
		{
			*localLoad-- = '0';
			nPrinted++;
		}
	}

	/** handle the sign **/
	if (isNeg)
	{
		*localLoad-- = '-';
		nPrinted++;

	} else if (specifierDescription->outFlags_ & OFLAG_FORCE_SIGN)
	{
		*localLoad-- = '+';
		nPrinted++;

	} else if (specifierDescription->outFlags_ & OFLAG_SPACE_SIGN)
	{
		*localLoad-- = ' ';
		nPrinted++;
	}

	loadFromBuffer(
			outputBufState,
			++localLoad,
			nPrinted,
			specifierDescription
		);
}

static void
loadChar(
		OutputBuffer *outputBufState,
		char c,
		SpecifierDescription *specifierDescription
    )
{
	int		isPrinted = 0;

	if (specifierDescription->width_ != 0)
	{
		if (specifierDescription->outFlags_ & OFLAG_LEFT_JUST)
		{
			addCharToOutputBuffer(outputBufState, c);
			isPrinted = 1;
		}
		/** print out specified width field **/
		addRepeatCharToOutputBuffer(
				outputBufState, ' ',
				specifierDescription->width_ - 1
			);
	}
	if (!isPrinted)
		addCharToOutputBuffer(outputBufState, c);
}


static void
loadString(
		OutputBuffer *outputBufState,
		const char *strVal,
		SpecifierDescription *specifierDescription
    )
{
	char *dupVal;
	int length;

	length = strlen(strVal);

	/** make sure that we are not padding with zeros **/
	specifierDescription->outFlags_ &= ~(OFLAG_PAD_ZERO);

	/** see whether we have to limit the precision **/
	if ((specifierDescription->outFlags_ & OFLAG_HAS_PREC) &&
				(specifierDescription->precision_ < length))
	{
		dupVal = ckstrdup(strVal);

		dupVal[specifierDescription->precision_] = '\0';

		loadFromBuffer(outputBufState, dupVal,
						specifierDescription->precision_,
						specifierDescription);
		ckfree(dupVal);

	} else
	{
		loadFromBuffer(
				outputBufState,
				strVal,
				length,
				specifierDescription
			);
	}
}

static void loadBad(OutputBuffer *outputBufState)
{
	addStringToOutputBuffer(outputBufState, "<BAD>");
}

/**
 * print into this string using a "printf()" like format,
 * growing the buffer as we proceed.
 */
int tlSrString::vsprintf(const char *formatString, va_list args)
{
	OutputBuffer outputBufState;
	SpecifierDescription specifierDescription;
	int		specifierLength;
	int nConverted = 0;

	/** clear the output buffer */
	memset(&outputBufState, 0, sizeof(OutputBuffer));

	while (*formatString != '\0')
	{
		if (*formatString != '%')
		{
			addCharToOutputBuffer(&outputBufState, *formatString++);
		} else
		{

			/** read the formatString specifier **/
			specifierLength = parseFormatSpecifier(
						++formatString, &specifierDescription
					);

			if (specifierDescription.outFlags_ == OFLAG_BAD_FORMAT)
			{
				loadBad(&outputBufState);
				return (nConverted);
			}

			/** handle '*' in width spec **/
			if (specifierDescription.outFlags_ & OFLAG_WIDTH_LOADED)
			{
				specifierDescription.width_ = va_arg(args, int);
			}

			/** handle '*' in precision spec **/
			if (specifierDescription.outFlags_ & OFLAG_PREC_LOADED)
			{
				specifierDescription.precision_ = va_arg(args, int);
			}


			/** determine what data type to load for printing **/
			if (specifierDescription.frmtType_ == FORMAT_TYPE_PERCENT)
			{
				loadChar(&outputBufState, '%', &specifierDescription);


			} else switch (specifierDescription.dataType_)
			{
			case		DTYPE_INTEGER:
				{
					int iVal = va_arg(args, int);
					loadInteger(&outputBufState,
									iVal, &specifierDescription);
				}
				break;

			case		DTYPE_UNSIGNED:
				{
					unsigned uVal = va_arg(args, int);
					loadUnsigned(&outputBufState,
									uVal, &specifierDescription);
				}
				break;

			case		DTYPE_DOUBLE:
				{
					double dVal = va_arg(args, double);
					loadDouble(&outputBufState,
									dVal, &specifierDescription);
				}
				break;

			case		DTYPE_CHAR:
				{
					char cVal = (char) va_arg(args, int);
					loadChar(&outputBufState,
									cVal, &specifierDescription);
				}
				break;

			case		DTYPE_STRING:
				{
					/** get a char * off of the stack **/
					const char *strVal;
					strVal = va_arg(args, char *);
					if (strVal == NULL)
						strVal = "(nil)";
					loadString(&outputBufState,
									strVal, &specifierDescription);
				}
				break;

			case		DTYPE_TL_STRING:
				{
					const char *strVal;
					tlSrString *tlStrVal;

					/** get a tlSrString * off of the stack **/
					tlStrVal = va_arg(args, tlSrString *);
					if (tlStrVal == NULL)
					{
						strVal = "(nil)";
					} else
					{
						strVal = tlStrVal->getValue();
						if (strVal == NULL)
						{
							strVal = "(nil-tstring)";
						}
					}
					loadString(&outputBufState,
									strVal, &specifierDescription);
				}
				break;

			case		DTYPE_POINTER:
				{
					void *pVal = va_arg(args, void *);
					specifierDescription.frmtType_ = FORMAT_TYPE_HEX;
					specifierDescription.outFlags_ |= OFLAG_PAD_ZERO;
					if (specifierDescription.width_ < 8)
					{
						specifierDescription.width_ = 8;
					}
					loadUnsigned(&outputBufState,
									(unsigned long) pVal,
									&specifierDescription);
				}
				break;
			}

			formatString += specifierLength;
			nConverted++;
		}
	}

	/** save in _our_ internal buffer, free buffer from OutputBuffer **/
	setValue( outputBufState.buffer_ );
	ckfree(outputBufState.buffer_);
	memset(&outputBufState, 0, sizeof(OutputBuffer));

	return (nConverted);
}

int tlSrString::sprintf(const char *formatString, ...)
{
	va_list args;
	int retval;

	va_start(args, formatString);
	retval = tlSrString::vsprintf(formatString, args);
	va_end(args);

	return(retval);
}

tlSrString tlSrString::niceDouble(double d)
{
	// need :: to ensure that we don't recursively call ourselves,
	// but rather actually call the function by this name
	setValue(::niceDouble(d));
	return *this;
}

tlSrString tlSrString::fullyTrimmedDouble(double d)
{
	setValue(::fullyTrimmedDouble(d));
	return *this;
}

void tlSrString::setQuoted(const char *s)
{
	char *tmp;
	int len;
	int tmpIndex;
	int start, end;
	int i, ch;

	len = strlen(s);

	tmpIndex = 0;
	tmp = (char *) ckalloc(len + 1);
	MSG_ASSERT(tmp != NULL, "malloc failed");

	if (s[0] == '"' && s[len-1] == '"')
	{
		start = 1;
		end = len-1;
	} else
	{
		start = 0;
		end = len;
	}

	for (i = start; i < end; i++)
	{
		ch = s[i++];
		if (ch == '\\')
		{
			if (isdigit(ch))
			{

				int k = 0;
				int octalVal = (ch - '0');

				while (k++ < 3)
				{
					ch = s[i++];
					if ( ! isdigit(ch))
					{
						break;
					}
					octalVal = (octalVal * 8 ) + (ch - '0');
				}
				tmp[tmpIndex++] = octalVal;

			} else
			{
				switch (ch)
				{
					case 'n':  tmp[tmpIndex++] = '\n'; break;
					case 't':  tmp[tmpIndex++] = '\t'; break;
					case 'b':  tmp[tmpIndex++] = '\b'; break;
					case 'r':  tmp[tmpIndex++] = '\r'; break;
					case 'f':  tmp[tmpIndex++] = '\f'; break;

					/** handle end of string semi-gracefully */
					case 0: 
						   tmp[tmpIndex++] = 0;
						   i--;
						   break;

					default: tmp[tmpIndex++] = ch;
				}
			}
		} else
		{
			tmp[tmpIndex++] = ch;
		}
	}
	tmp[tmpIndex++] = 0;

	setValue(tmp);
	ckfree(tmp);
}

void tlSrString::setQuoted(tlSrString s)
{
	tlSrString::setQuoted(s.getValue());
}

tlSrString tlSrString::getQuoted() const
{
	tlSrString r;
	char *tmp;
	int i, j;

	tmp = (char *) ckalloc( (getLength() * 2) + 3);
	MSG_ASSERT(tmp != NULL, "Out of memory");

	j = 0;
	tmp[j++] = '"';
	for (i = 0; i < getLength(); i++)
	{
		switch(data_->s_[i])
		{
		case '\n':
			tmp[j++] = '\\';
			tmp[j++] = 'n';
			break;

		case '\t':
			tmp[j++] = '\\';
			tmp[j++] = 't';
			break;

		case '\b':
			tmp[j++] = '\\';
			tmp[j++] = 'b';
			break;

		case '\v':
			tmp[j++] = '\\';
			tmp[j++] = 'v';
			break;

		case '\a':
			tmp[j++] = '\\';
			tmp[j++] = 'a';
			break;

		case '\\':
			tmp[j++] = '\\';
			tmp[j++] = '\\';
			break;

		case '"':
			tmp[j++] = '\\';
			tmp[j++] = '"';
			break;

		default:
			tmp[j++] = data_->s_[i];
		}
	}
	tmp[j++] = '"';
	tmp[j] = 0;

	r.setValue(tmp);
	ckfree(tmp);

	return r;
}

tlSrString tlSrString::toLower() const
{
	tlSrString r;
	char *s;
	int i;

	s = ckstrdup(getValue());
	for (i = 0; i < data_->sLen_; i++)
	{
		s[i] = tolower(s[i]);
	}

	r = s;

	ckfree(s);

	return r;
}

tlSrString tlSrString::toUpper() const
{
	tlSrString r;
	char *s;
	int i;

	s = ckstrdup(getValue());
	for (i = 0; i < data_->sLen_; i++)
	{
		s[i] = toupper(s[i]);
	}

	r = s;

	ckfree(s);

	return r;
}

int tlSrString::contains(const char *substring) const
{
	const char *m;

	if (data_ == NULL)
		return -1;
	m = strstr(data_->s_, substring);
	if (m == NULL)
		return -1;

	return (int) (m - data_->s_);
}

int tlSrString::contains(char match) const
{
	const char *m;

	if (data_ == NULL)
		return -1;
	m = strchr(data_->s_, match);
	if (m == NULL)
		return -1;

	return (int) (m - data_->s_);
}

int tlSrString::containsAny(const char *match) const
{
	const char *m, *matchChar;

	if (data_ == NULL)
		return -1;
	matchChar = match;
	while (*matchChar != 0)
	{
		m = strchr(data_->s_, *matchChar);
		if (m != NULL)
			return (int) (m - data_->s_);
		matchChar++;
	}

	return -1;
}

