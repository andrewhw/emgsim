/** ------------------------------------------------------------
 ** OS-dependent flag and detection code.
 **
 ** These routines will define a set of macros to do various
 ** tasks on all known architectures, so that we can write more
 ** portable code.
 **
 ** Tasks of note:
 ** - preprocessor string concatenation and quoting:
 ** OS_CONCAT
 ** OS_QUOTE
 ** - pragma once definition (only include a given file once,
 **   if the architecture supports this)
 ** OS_PRAGMA_ONCE
 ** - M_PI if not available
 ** - Platform path delimiters
 ** ------------------------------------------------------------
 ** $Id: os_defs.h 93 2012-04-30 20:55:38Z andrew $
 **/

#ifndef __OS_DEF_TYPE_HEADER__
#define __OS_DEF_TYPE_HEADER__

/* $Id: os_defs.h 93 2012-04-30 20:55:38Z andrew $ */


/* System include files. */
#ifndef MAKEDEPEND
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#endif


/* OS_QUOTE OS_CONCAT and OS_CONCAT3 macros */
#if defined(__STDC__) || defined(__ANSI_CPP__) || defined(_WINDOWS) || defined(_WIN32) /* ANSI C */
#   define OS_QUOTE(str)	#str
#   define OS_CONCAT(str1,str2)	str1##str2
#   define OS_CONCAT3(str1,str2,str3)   str1##str2##str3
#else					/* Non-ANSI C */
#   define OS_QUOTE(str)	"str"
#   define OS_CONCAT(str1,str2)	str1/**/str2
#   define OS_CONCAT3(str1,str2,str3)   str1/**/str2/**/str3
#endif
#define OS_QUOTE_QUOTE(x)       OS_QUOTE(x)

/*
 * Compilation Environment Identification
 */
#define OS_PRAGMA_ONCE
#if defined( sgi ) && defined( unix ) && !defined( cplusplus_2_1 )
#   define OS_IRIX
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on SGI
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( __SunOS_5_8 ) || defined( __SunOS_5_6) || defined( __SunOS_5_7 ) 
#   define OS_SOLARIS
#   undef OS_PRAGMA_ONCE
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on SunOS
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( __SunOS_5_5_1 ) || defined( __SunOS_5_5 )
#   define OS_SOLARIS
#   undef OS_PRAGMA_ONCE
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on SunOS
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( __sun ) && defined( __SVR4 )
#   define OS_SOLARIS
#   undef OS_PRAGMA_ONCE
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on Solaris
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( hpux ) && defined( __hp9000s700 )
#   define OS_HPUX
#   undef OS_PRAGMA_ONCE
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on HP-UX
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( _AIX41 )
#   define OS_AIX
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on AIX
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( __NetBSD__ ) || defined( __OpenBSD__) || defined( __FreeBSD__ )
#   define OS_BSD
#   define OS_HAS_SNPRINTF
#   undef OS_PRAGMA_ONCE
#   if defined( __NetBSD__ )
#      define OS_NET_BSD
#   elif defined( __OpenBSD__)
#      define OS_OPEN_BSD
#   elif defined( __FreeBSD__ )
#      define OS_FREE_BSD
#   endif
#   if defined( __x86_64__ )
#     define OS_LITTLE_ENDIAN
#   elif defined( _BIG_ENDIAN )
#     define OS_BIG_ENDIAN
#   elif defined( _LITTLE_ENDIAN )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on BSD
# endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( linux )
#   define OS_LINUX
#   define OS_HAS_SNPRINTF
#   define OS_NEEDS_STRLCPY
#   undef OS_PRAGMA_ONCE
#   if defined( __BIG_ENDIAN__ )
#     define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#     define OS_LITTLE_ENDIAN
#   else
#     if defined( __i486__ )
#       define OS_LITTLE_ENDIAN
#     elif defined( __i386__ )
#       define OS_LITTLE_ENDIAN
#     elif defined( __x86_64 )
#       define OS_LITTLE_ENDIAN
#     else
#       error Unknown endian on Linux
#     endif
#   endif
/* #   undef OS_PRAGMA_ONCE */

#elif defined( __CYGWIN__ )
#   define OS_CYGWIN
#   undef OS_PRAGMA_ONCE
#   define OS_LITTLE_ENDIAN

#elif defined( _WIN32 )
#	if ! defined( OS_WINDOWS )
#     define OS_WINDOWS
#   endif
#   define OS_WINDOWS_NT
#   define OS_NEEDS_STRLCPY
#   define SIMPLE_RANDOM_ONLY
#   define OS_LITTLE_ENDIAN
#   if defined __GNUC__
#     undef OS_PRAGMA_ONCE
#   endif

#elif defined( __APPLE__ )
#   define OS_DARWIN
#   define OS_HAS_SNPRINTF
#   if defined( __BIG_ENDIAN__ )
#      define OS_BIG_ENDIAN
#   elif defined( __LITTLE_ENDIAN__ )
#      define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on Apple
#   endif
/* #   undef OS_PRAGMA_ONCE */

#else
#   define OS_UNIX_UNKNOWN
#   undef OS_PRAGMA_ONCE
#endif

#ifndef MAKEDEPEND
#include <limits.h>
#ifdef OS_WINDOWS_NT
#include <stddef.h>
#endif
#endif

/*
 * Limits and Constants
 */
#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif
#ifndef NULL
#define NULL    0
#endif

#if defined( _WIN32 )
/* _POSIX_ARG_MAX smaller than ARG_MAX */
#define OS_ARG_MAX      _POSIX_ARG_MAX
#else
#define OS_ARG_MAX      ARG_MAX
#endif

#define OS_PATH_MAX     PATH_MAX

#if defined( _WIN32 )
#define OS_PIPE_MAX     PIPE_BUF
#else
#define OS_PIPE_MAX     PIPE_MAX
#endif

#ifdef sgi
#   define OS_NAME_MAX  NAME_MAX+1      /* Must include null */
#elif defined( __sun )
#ifndef MAKEDEPEND
#   include <sys/param.h>
#endif
#   define OS_NAME_MAX  MAXNAMELEN
#else
#define OS_NAME_MAX     NAME_MAX
#endif


#if defined( OS_IRIX )
# define OS_EXPORT
#elif defined( OS_SOLARIS )
# define OS_EXPORT
#elif defined( OS_HPUX )
# define OS_EXPORT
#elif defined( OS_AIX ) 
# define OS_EXPORT
#elif defined( OS_BSD )
# define OS_EXPORT
#elif defined( OS_LINUX )
# define OS_EXPORT
#elif defined( OS_DARWIN )
# define OS_EXPORT
#elif defined( OS_UNIX_UNKNOWN )
# define OS_EXPORT
#elif defined( OS_WINDOWS_NT )
# if defined( OS_DYNAMIC )
#  define OS_EXPORT __declspec(dllexport)
# elif defined( OS_USE_DYNAMIC )
#  define OS_EXPORT __declspec(dllimport)
# elif defined( OS_STATIC )
#  define OS_EXPORT
# else
#  define OS_EXPORT
# endif
#endif

/**
 ** This ensures that when we include string.h we will get the 
 ** reentrant version of strtok.
 **/
#ifndef MAKEDEPEND
#ifdef OS_IRIX
#define _SGI_REENTRANT_FUNCTIONS
#endif
#endif /* !MAKEDEPEND */


/**
 ** Not Reached complaint in switch statements -- use OS_BREAK instead
 ** of break after returns from inner switch clauses
 **/
#ifdef  OS_IRIX
# define	OS_BREAK
#else
# define	OS_BREAK	break
#endif

/**
 * Can we/should we leave semicolons bare when deflating
 * macros -- old GCC did not like this . . .
 */
#if !defined __GNUC__ || __GNUC__ >= 2
#define	OS_BARE_SEMI_OK
#endif

/**
 ** Win32 hacks . . . 
 **/
#ifdef  OS_WINDOWS_NT

# include	<math.h>
# ifndef	M_PI
   /**
    * these values are taken from the GCC header file /usr/include/math.h
    * and are reproduced here as these values are not available under Windows
    */
#  define M_E		2.7182818284590452354	/* e */
#  define M_LOG2E	1.4426950408889634074	/* log_2 e */
#  define M_LOG10E	0.43429448190325182765	/* log_10 e */
#  define M_LN2		0.69314718055994530942	/* log_e 2 */
#  define M_LN10	2.30258509299404568402	/* log_e 10 */
#  define M_PI		3.14159265358979323846	/* pi */
#  define M_PI_2	1.57079632679489661923	/* pi/2 */
#  define M_PI_4	0.78539816339744830962	/* pi/4 */
#  define M_1_PI	0.31830988618379067154	/* 1/pi */
#  define M_2_PI	0.63661977236758134308	/* 2/pi */
#  define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#  define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
#  define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
# endif


# define random()	rand()
# define srandom(s)	srand(s)

#if !defined __GNUC__
typedef int		mode_t;
typedef int		pid_t;
#endif

# define	OS_PATH_DELIM			'\\'
# define	OS_PATH_DELIM_STRING		"\\"
# define	OS_PATH_INVALID_DELIM		'/'
# define	OS_PATH_INVALID_DELIM_STRING	"/"
#else
# define	OS_PATH_DELIM			'/'
# define	OS_PATH_DELIM_STRING		"/"
# define	OS_PATH_INVALID_DELIM		'\\'
# define	OS_PATH_INVALID_DELIM_STRING	"\\"
#endif

#ifndef		lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT char *osIndependentPath(const char *localPath);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

/**
 ** On Win32 systems, defining OS_DEBUG
 ** will enable extra memory debug smarts that can help track down
 ** memory leaks.
 **/
#ifdef OS_DEBUG
# ifdef OS_WINDOWS_NT
 /*  derfix: #   include <afx.h> */
#  include <windows.h>
#  include <tchar.h>
# endif /* !OS_WINDOWS_NT */
#endif /* !OS_DEBUG */

/**
 ** if not on a Windows system, ensure that we have O_BINARY
 ** defined to a safe token
 **/
#ifndef	OS_WINDOWS_NT
# ifndef	O_BINARY
# define	O_BINARY	0
# endif
#endif



/*
 * Types specific to certain OS's are mapped to an os-independent
 * type here, so that we can do things like 64-bit math in a sensible
 * and portable way
 */
#if defined( OS_SGI )

#ifndef MAKEDEPEND
#include <float.h>
#endif

typedef __int16_t		osInt16;
typedef __uint16_t		osUint16;
typedef __int32_t		osInt32;
typedef __uint32_t		osUint32;

#if (_MIPS_SZLONG != 64) && !defined( _LONGLONG)
#error No 64-bit integer defined
#else
typedef __int64_t		osInt64;
typedef __uint64_t		osUint64;
#endif




#elif defined( OS_WINDOWS_NT )	/* WINDOWS */

#if !defined __GNUC__
#include <fpieee.h>
#include <float.h>
typedef _I16			osInt16;
typedef _U16			osUint16;
typedef _I32			osInt32;
typedef _U32			osUint32;
typedef __int64			osInt64;

#else
typedef short			osInt16;
typedef unsigned short		osUint16;
typedef int			osInt32;
typedef unsigned int		osUint32;
typedef long			osInt64;
typedef unsigned long		osUint64;
#warning	no true 64-bit type defined
#endif



#elif defined( OS_SOLARIS ) || defined( OS_AIX ) || defined( OS_HPUX )

typedef short			osInt16;
typedef unsigned short		osUint16;
typedef int			osInt32;
typedef unsigned int		osUint32;
typedef long long		osInt64;
typedef unsigned long long      osUint64;




#elif defined( OS_BSD )

#ifndef MAKEDEPEND
#include <sys/types.h>
#endif

typedef short			osInt16;
typedef unsigned short		osUint16;
typedef int			osInt32;
typedef unsigned int		osUint32;
typedef quad_t			osInt64;
typedef u_quad_t		osUint64;



#elif defined( OS_LINUX )

#ifndef MAKEDEPEND
#include <stdint.h>
#endif

typedef short			osInt16;
typedef unsigned short		osUint16;
typedef int			osInt32;
typedef unsigned int		osUint32;
typedef int64_t			osInt64;
typedef uint64_t		osUint64;


#elif defined( OS_DARWIN )

#ifndef MAKEDEPEND
#include <sys/types.h>
#endif

typedef short			osInt16;
typedef unsigned short		osUint16;
typedef int			osInt32;
typedef unsigned int		osUint32;
typedef quad_t			osInt64;
typedef u_quad_t		osUint64;

#endif


/* Include this file once per compile */
#ifdef OS_PRAGMA_ONCE
#pragma once
#endif
#endif /* !OS_DEF_TYPE_HEADER */


