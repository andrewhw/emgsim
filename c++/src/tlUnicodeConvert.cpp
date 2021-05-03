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
 * $Id: tlUnicodeConvert.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <string.h>
# include <ctype.h>
#endif

#include "tclCkalloc.h"

#include "tlUnicodeConvert.h"


static const char *BAD_UNICODE = "<BADUNICODE>";
static const char *BAD_UTF = "<BADUTF>";

/**
 * Count the characters in a Unicode string
 */
OS_EXPORT int
tlUnicodeStrlen(const tlUNICODE *unicodeString)
{
	int i = 0;

	while (unicodeString[i] != 0)
		i++;

	return i;
}

/**
 * Convert from Unicode to UTF:
 *
 * UNI data ranges		UTF width		UTF convert
 *		
 * 0x0000 - 0x007F		1				buf[0] = (char) uVal
 *
 * 0x0080 - 0x07FF		2				buf[0] = (char) (uVal / 0x40) | 0xC0
 * 										buf[1] = (char) (uVal % 0x40) | 0x80
 *
 * 0x0800 - 0x0FFF		3				buf[0] = (char) (uVal / 0x1000) | 0xE0
 * 										buf[1] = (char) (uVal / 0x40) % 0x40
 * 										buf[2] = (char) (uVal % 0x40)
 */
OS_EXPORT char *
tlUnicodeToUTF(const tlUNICODE *unicodeString)
{
	size_t i, j, k;
	size_t len;
	int requiredLen = 1;
	char *charBuffer;

	len = tlUnicodeStrlen(unicodeString);
	for (i = 0; i < len; i++)
	{
		if (unicodeString[i] < 0x007f)
		{
			requiredLen += 1;

		} else if (unicodeString[i] < 0x07ff)
		{
			requiredLen += 2;

		} else if (unicodeString[i] < 0x0fff)
		{
			requiredLen += 3;

		} else
		{
			requiredLen += strlen(BAD_UNICODE);
		}
	}


	/** allocate the buffer we will fill */
	charBuffer = (char *) ckalloc(sizeof(char) * requiredLen);

	j = 0;
	for (i = 0; i < len; i++)
	{
		if (unicodeString[i] < 0x007f)
		{
			charBuffer[j++] = (char) unicodeString[i];

		} else if (unicodeString[i] < 0x07ff)
		{
			charBuffer[j++] = (char) (unicodeString[i] / 0x40) | 0xc0;
			charBuffer[j++] = (char) (unicodeString[i] % 0x40) | 0x80;

		} else if (unicodeString[i] < 0x0fff)
		{
			charBuffer[j++] = (char) (unicodeString[i] / 0x1000) | 0xe0;
			charBuffer[j++] = (char) (unicodeString[i] / 0x40) % 0x40;
			charBuffer[j++] = (char) (unicodeString[i] % 0x40);

		} else
		{
			for (k = 0; k < strlen(BAD_UNICODE); k++)
			{
				charBuffer[j++] = BAD_UNICODE[k];
			}
		}
	}

	/** terminate the buffer */
	charBuffer[j++] = 0;

	return charBuffer;
}


OS_EXPORT tlUNICODE *
tlUTFToUnicode(const char *userCharString)
{
	size_t i, j, k;
	size_t len;
	int requiredLen = 1;
	tlUNICODE *unicodeBuffer;
	unsigned char *uCharString;

	/** make sure we are dealing with an unsigned string */
	uCharString = (unsigned char *) userCharString;


	/** figure out how many unicode charaters we need */
	len = strlen(userCharString);
	i = 0;
	while (i < len)
	{

		/** handle 1:1 data */
		if (uCharString[i] < 0x80)
		{
			requiredLen++;
			i += 1;

		} else if ((uCharString[i] >= 0xC0) && (uCharString[i] < 0xF0))
		{
			/** otherwise we have a longer character sequence */

			/** handle 1:2 data */
			if (uCharString[i] < 0xE0)
			{
				if ((uCharString[i+1] >= 0x80)
								&& (uCharString[i+1] < 0xC0))
								{
					i += 2;
				} else
				{
					/** missing second octet */
					i += 1;
					requiredLen += strlen(BAD_UTF);
				}
				requiredLen++;

			} else
			{
				/** handle 1:3 data */
				if (uCharString[i+1] >= 0x80 && uCharString[i+i] < 0xC0)
				{
					if ((uCharString[i+2] >= 0x80) && (uCharString[i+2] < 0xC0))
					{
						i += 3;
						requiredLen++;
					} else
					{
						/** missing third octet */
						i += 2;
						requiredLen += strlen(BAD_UTF);
						requiredLen++;
					}
				} else
				{
					/** second octet was bad -- restart with next char */
					i += 1;
					requiredLen += strlen(BAD_UTF);
					requiredLen++;
				}
			}
		} else
		{
			i += 1;
			requiredLen += strlen(BAD_UTF);
			requiredLen++;
		}
	}

	/** allocate the buffer we will fill */
	unicodeBuffer = (tlUNICODE *) ckalloc(sizeof(tlUNICODE) * requiredLen);


	/** now fill the buffer of UNICODE characters */
	j = 0;
	while (i < len)
	{

		/** handle 1:1 data */
		if (uCharString[i] < 0x80)
		{
			unicodeBuffer[j++] = (tlUNICODE) uCharString[i];
			i += 1;

		} else if ((uCharString[i] >= 0xC0) && (uCharString[i] < 0xF0))
		{
			/** otherwise we have a longer character sequence */

			/** handle 1:2 data */
			if (uCharString[i] < 0xE0)
			{
				if ((uCharString[i+1] >= 0x80) && (uCharString[i+1] < 0xC0))
				{
					unicodeBuffer[j++] = (tlUNICODE)
								((uCharString[i] - 0xC0) * 0x40
										+ (uCharString[i+1] - 0x80));
					i += 2;
				} else
				{
					/** missing second octet */
					unicodeBuffer[j++] = (tlUNICODE) uCharString[i];
					for (k = 0; k < strlen(BAD_UTF); k++)
					{
						unicodeBuffer[j++] = (tlUNICODE) BAD_UTF[k];
					}
					i += 1;
				}

			} else
			{
				/** handle 1:3 data */
				if (uCharString[i+1] >= 0x80 && uCharString[i+i] < 0xC0)
				{
					if ((uCharString[i+2] >= 0x80) && (uCharString[i+2] < 0xC0))
					{
						unicodeBuffer[j++] = (tlUNICODE)
								((uCharString[i] - 0xE0) * 0x1000
									+ (uCharString[i+1] - 0x80) * 0x40
									+ (uCharString[i+2] - 0x80));
						i += 3;
					} else
					{
						/** missing third octet */
						unicodeBuffer[j++] = (tlUNICODE) uCharString[i];
						unicodeBuffer[j++] = (tlUNICODE) uCharString[i+1];
						for (k = 0; k < strlen(BAD_UTF); k++)
						{
							unicodeBuffer[j++] = (tlUNICODE) BAD_UTF[k];
						}
						i += 2;
					}
				} else
				{
					/** second octet was bad -- restart with next char */
					unicodeBuffer[j++] = (tlUNICODE) uCharString[i];
					for (k = 0; k < strlen(BAD_UTF); k++)
					{
						unicodeBuffer[j++] = (tlUNICODE) BAD_UTF[k];
					}
					i += 1;
				}
			}
		} else
		{
			unicodeBuffer[j++] = (tlUNICODE) uCharString[i];
			for (k = 0; k < strlen(BAD_UTF); k++)
			{
				unicodeBuffer[j++] = (tlUNICODE) BAD_UTF[k];
			}
			i += 1;
		}
	}
	unicodeBuffer[j++] = 0;

	return unicodeBuffer;
}

