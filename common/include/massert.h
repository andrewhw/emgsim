/** ------------------------------------------------------------
 ** Assert message handling
 ** ------------------------------------------------------------
 ** $Id: massert.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         MESSAGE_ASSERT_HEADER__
#define         MESSAGE_ASSERT_HEADER__

#ifndef MAKEDEPEND
#include        <assert.h>
#endif

#include        "os_defs.h"


/**
 ** MACRO DEFINITIONS
 **/

/** ASSERT */
#if defined(DEBUG) || defined(_DEBUG)

# define       MSG_ASSERT(x, m)        \
            do { \
                if (!(x)) { \
                    fprintf(stderr, "Assertion Failed -- %s\n", (m)); \
                    fflush(stderr); \
                    assert(x); \
                } \
            } while ( 0 )
#else

#define         MSG_ASSERT(x, m)        assert(x)

#endif



#if defined(DEBUG) || defined(_DEBUG)

# ifndef        OS_WINDOWS_NT
#  define       MSG_FAIL(m)        \
            do { \
		fprintf(stderr, "Failure -- %s\n", (m)); \
		fflush(stderr); \
		assert(0); \
            } while ( 0 )
# else
        /* in windows -- see if we are in an AFXDLL setting */
#  if defined(AFXDLL)
#    define     MSG_FAIL(m)        \
            do { \
		char buffer[1024]; \
		sprintf(buffer, "Failure -- %s\n", (m)); \
		TRACE(out); \
		assert(0); \
            } while ( 0 )
#  else
#    define     MSG_FAIL(m)        \
            do { \
		fprintf(stderr, "Failure -- %s\n", (m)); \
		fflush(stderr); \
		assert(0); \
            } while ( 0 )
#  endif
# endif
#else

#define         MSG_FAIL(m)        assert(0)

#endif


#endif  /* MESSAGE_ASSERT_HEADER__      */


