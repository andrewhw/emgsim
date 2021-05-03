/* ------------------------------------------------------------
 * Bitstring tools -- allocate and manage bitstrings of
 * arbitrary length
 * ------------------------------------------------------------
 * $Id: bitstring.h 99 2012-05-09 17:30:02Z andrew $
 */

#ifndef         __BITSTRING_HEADER__
#define         __BITSTRING_HEADER__

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#endif

#include        "tclCkalloc.h"

/*
 * the bits will be held in a string of octets (unsigned chars)
 */
typedef unsigned char * BITSTRING;


/*
 * macro definitions
 */

#define	BITSTRING_LEN(nBits)	((nBits + 7) / 8)
#define	ALLOC_BITSTRING(nBits)	\
    		((unsigned char *) ckalloc(BITSTRING_LEN(nBits)))
#define	FREE_BITSTRING(s)	ckfree(s)
#define	ZERO_BITSTRING(s,nBits)	memset(s, 0, BITSTRING_LEN(nBits))


#define GET_BIT(s,i)	((int) (((s[i/8]) >> (i % 8)) & 0x01))
#define CLR_BIT(s,i)	(s[i/8] = (s[i/8] & ~(0x01 << (i % 8)))
#define SET_BIT(s,i,v)	(s[i/8] = \
				((s[i/8] & ~(0x01 << (i % 8))) \
					 | (((v == 0) ? 0x00 : 0x01) << (i % 8))))

#endif  /* __BITSTRING_HEADER__ */

