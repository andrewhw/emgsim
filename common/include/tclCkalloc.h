/** ------------------------------------------------------------
 **
 ** TCL-extracted malloc/free debugging tools
 **
 ** ------------------------------------------------------------
 ** $Id: tclCkalloc.h 10 2008-04-24 18:37:51Z andrew $
 **
 ** This code comes from the University of California, and
 ** was originally part of the TCL project (see note below).
 **
 ** THIS CODE HAS BEEN MODIFIED FROM THE ORIGINAL.
 **
 ** -- original University of California header below --
 **
 ** tclCkalloc.h --
 ** 
 **     Interface to malloc and free that provides support for
 **     debugging problems involving overwritten, double freeing
 **     memory and loss of memory.
 ** 
 ** Copyright (c) 1987-1993 The Regents of the University of
 ** California.  All rights reserved.
 ** 
 ** Permission is hereby granted, without written agreement
 ** and without license or royalty fees, to use, copy, modify,
 ** and distribute this software and its documentation for any
 ** purpose, provided that the above copyright notice and the
 ** following two paragraphs appear in all copies of this software.
 ** 
 ** IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE
 ** TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 ** CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
 ** SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 ** CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ** 
 ** THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
 ** WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 ** PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 ** BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 ** PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 ** MODIFICATIONS.
 ** 
 ** This code contributed by Karl Lehenbauer and Mark Diekhans
 **
 ** ------------------------------------------------------------
 ** This code HAS BEEN MODIFIED from the original by
 ** Andrew Hamilton-Wright
 ** 
 **/


#ifndef _TCL_HEADER__
#define _TCL_HEADER__

#ifndef MAKEDEPEND
#include	<stdio.h>
#include	<stdlib.h>
#endif

#include	"localstrings.h"

#define TCL_VERSION		"7.3"
#define TCL_MAJOR_VERSION	7
#define TCL_MINOR_VERSION	3


/*
 * Miscellaneous declarations (to allow Tcl to be used stand-alone,
 * without the rest of Sprite).
 */

#define TCL_OK		0
#define TCL_ERROR	1
#define TCL_RETURN	2
#define TCL_BREAK	3
#define TCL_CONTINUE	4

/*
 * Argument descriptors for math function callbacks in expressions:
 */

/*
 * Flag values passed to variable-related procedures.
 */

#define TCL_GLOBAL_ONLY	 	1
#define TCL_APPEND_VALUE	2
#define TCL_LIST_ELEMENT	4
#define TCL_TRACE_READS		0x10
#define TCL_TRACE_WRITES	0x20
#define TCL_TRACE_UNSETS	0x40
#define TCL_TRACE_DESTROYED	0x80
#define TCL_INTERP_DESTROYED	0x100
#define TCL_LEAVE_ERR_MSG	0x200

/*
 * Types for linked variables:
 */

#define TCL_LINK_INT		1
#define TCL_LINK_DOUBLE		2
#define TCL_LINK_BOOLEAN	3
#define TCL_LINK_STRING		4
#define TCL_LINK_READ_ONLY	0x80

/*
 * Permission flags for files:
 */

#define TCL_FILE_READABLE	1
#define TCL_FILE_WRITABLE	2


/*
 * Add in protection against being compiled by a C++ compiler
 */
# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

/*
 * The following declarations either map ckalloc and ckfree to
 * malloc and free, or they map them to procedures with all sorts
 * of debugging hooks defined in tclCkalloc.c.
 */

#define DEFAULT_DUMP_FILE	"ckalloc.log"

#ifdef TCL_MEM_DEBUG

#define	VALIDATE_MEMORY	Tcl_ValidateAllMemory(__FILE__, __LINE__)

#if 0
#define	 DUMP_MEMORY	{ \
		Tcl_DumpActiveMemory(DEFAULT_DUMP_FILE);	\
		printf("Memory dumped to '%s'\n",		\
			DEFAULT_DUMP_FILE);			\
	} while (0)
#endif

#define	 DUMP_MEMORY	Tcl_DumpActiveMemory(DEFAULT_DUMP_FILE)

OS_EXPORT void*	Tcl_DbCkalloc(unsigned int size,
			const char *file, int line);
OS_EXPORT int	Tcl_DbCkfree(void *ptr,
			const char *file, int line);
OS_EXPORT void*	Tcl_DbCkrealloc(void *ptr,
			unsigned int size, const char *file, int line);
OS_EXPORT char*	Tcl_DbCkstrdup(const char *string,
			const char *file, int line);
OS_EXPORT int   Tcl_DumpActiveMemoryToFP(FILE *);
OS_EXPORT int	Tcl_DumpActiveMemory(const char *fileName);
OS_EXPORT void	Tcl_ValidateAllMemory(const char *file,
			int line);

   /** ckalloc is used in place of malloc */
#  define ckalloc(x) Tcl_DbCkalloc(x, __FILE__, __LINE__)
   /** ckallocmk is used when a parent __FILE__,__LINE__ is passed in */
#  define ckmkalloc(x,f,l) Tcl_DbCkalloc(x, f, l)
#  define ckfree(x)  Tcl_DbCkfree(x, __FILE__, __LINE__)
#  define ckstrdup(x) Tcl_DbCkstrdup(x, __FILE__, __LINE__)
#  define ckrealloc(x,y) Tcl_DbCkrealloc((x), (y),__FILE__, __LINE__)

#else

#define	 VALIDATE_MEMORY	(void) 0
#define	 DUMP_MEMORY		(void) 0

#  define ckalloc(x)		malloc(x)
#  define ckmkalloc(x,f,l)	malloc(x)
#  define ckfree(x)		free(x)
#  define ckstrdup(x)		strdup(x)
#  define ckrealloc(x,y)	realloc(x,y)
#  define Tcl_DumpActiveMemory(x)
#  define Tcl_ValidateAllMemory(x,y)

#endif /* TCL_MEM_DEBUG */

void panic(const char *message, ...);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif /* _TCL_HEADER__ */


