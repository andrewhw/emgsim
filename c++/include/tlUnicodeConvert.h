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
 * $Id: tlUnicodeConvert.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_UNICODE_CONVERSION_HEADER__
#define		__TOOL_UNICODE_CONVERSION_HEADER__

#include "os_defs.h"

////////////////////////////////////////////////////////////////
// A single 16-bit <a href="http://www.unicode.org">Unicode</a>
// character for use in tlSrString or other Unicode functions.
typedef unsigned short tlUNICODE;


////////////////////////////////////////////////////////////////
// return the length in 16-bit <a href="http://www.unicode.org">Unicode</a>
// characters of the given string
OS_EXPORT int
tlUnicodeStrlen(const tlUNICODE *unicodeString);

////////////////////////////////////////////////////////////////
// Convert the supplised
// <a href="http://www.unicode.org">Unicode</a>
// string to the standard Universal Translation Format (UTF-8)
// encoding for ASCII-printable encoding
OS_EXPORT char *
tlUnicodeToUTF(const tlUNICODE *unicodeString);


////////////////////////////////////////////////////////////////
// Convert the supplised UTF-8 (ASCII-superset) encoded string
// to 16-bit
// <a href="http://www.unicode.org">Unicode</a>
OS_EXPORT tlUNICODE *
tlUTFToUnicode(const char *userCharString);

#endif /* __TOOL_UNICODE_CONVERSION_HEADER__ */

