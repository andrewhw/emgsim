/** ------------------------------------------------------------
 **
 ** TCL-extracted malloc/free debugging tools
 **
 ** ------------------------------------------------------------
 ** $Id: tclCkalloc.c 107 2013-06-22 20:22:05Z andrew $
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

/**
 ** we must define this here or the macros will blow
 ** us up.
 **/
#ifndef TCL_MEM_DEBUG
#define TCL_MEM_DEBUG
#endif

#ifndef                         MAKEDEPEND
#include                <sys/types.h>
#include                <sys/stat.h>
#include                <fcntl.h>
#include                <ctype.h>
#include                <string.h>
#endif

#include                "msgir.h"
#include                "tclCkalloc.h"
#include                "stringtools.h"
#include                "error.h"


#define UCHAR(c) ((unsigned char) (c))

#define GUARD_SIZE 8

struct mem_header
{
    unsigned long   length;
    const char     *file;
    int             line;
    struct mem_header *flink;
    struct mem_header *blink;
    int             dummy;
    /**
     ** Aligns body on 8-byte boundary.
     **/

    unsigned char   low_guard[GUARD_SIZE];
    char            body[1];
};

static struct mem_header *allocHead = NULL;
/** List of allocated structures                **/

#define GUARD_VALUE  '~'

/* static char high_guard[] = {0x89, 0xab, 0xcd, 0xef}; */

static int      total_mallocs = 0;
static int      total_frees = 0;
static int      current_bytes_malloced = 0;
static int      maximum_bytes_malloced = 0;
static int      current_malloc_packets = 0;
static int      maximum_malloc_packets = 0;
static int      break_on_malloc = 0;
static int      trace_on_at_malloc = 0;
static int      alloc_tracing = 0;
static int      init_malloced_bodies = 1;
#ifdef MEM_VALIDATE
static int      validate_memory = 1;
#else
static int      validate_memory = 0;
#endif


/*
 *----------------------------------------------------------------------
 *
 * dump_memory_info --
 *  Display the global memory management statistics.
 *
 *----------------------------------------------------------------------
 */
static void
dump_memory_info(FILE * outFile)
{
    fprintf(outFile, "total mallocs             %10d\n",
            total_mallocs);
    fprintf(outFile, "total frees               %10d\n",
            total_frees);
    fprintf(outFile, "current packets allocated %10d\n",
            current_malloc_packets);
    fprintf(outFile, "current bytes allocated   %10d\n",
            current_bytes_malloced);
    fprintf(outFile, "maximum packets allocated %10d\n",
            maximum_malloc_packets);
    fprintf(outFile, "maximum bytes allocated   %10d\n",
            maximum_bytes_malloced);
}


/*
 *----------------------------------------------------------------------
 *
 * ValidateMemory --
 *  Procedure to validate allocted memory guard zones.
 *
 *----------------------------------------------------------------------
 */
static void
ValidateMemory(
               struct mem_header * memHeaderP,
               const char *file,
               int line,
               int nukeGuards
	)
{
    unsigned char  *hiPtr;
    int             idx;
    int             guard_failed = 0;
    int             byte;

    for (idx = 0; idx < GUARD_SIZE; idx++)
    {
        byte = *(memHeaderP->low_guard + idx);
        if (byte != GUARD_VALUE)
        {
            guard_failed = 1;
            fflush(stdout);
            byte &= 0xff;
            fprintf(stderr, "low guard byte %d is 0x%x  \t%c\n",
                    idx, byte,
                    (isprint(UCHAR(byte)) ? byte : ' '));
        }
    }

    /** look at the body to ensure the pointer is valid */
#ifdef	OLDDEBUG
    {
        char           *address, c;

        address = &memHeaderP->body[0];
        c = *address;
    }
#endif

    if (guard_failed)
    {
        dump_memory_info(stderr);
        fprintf(stderr, "low guard failed at %p, %s %d\n",
                memHeaderP->body, file, line);
        fflush(stderr);         /* In case name pointer is bad. */
        fprintf(stderr, "%ld bytes allocated at (%s %d)\n",
                memHeaderP->length,
                memHeaderP->file, memHeaderP->line);
        panic("Memory validation failure");
    }
    hiPtr = (unsigned char *) memHeaderP->body + memHeaderP->length;
    for (idx = 0; idx < GUARD_SIZE; idx++)
    {
        byte = *(hiPtr + idx);
        if (byte != GUARD_VALUE)
        {
            guard_failed = 1;
            fflush(stdout);
            byte &= 0xff;
            fprintf(stderr, "hi guard byte %d is 0x%x  \t%c\n",
                    idx, byte,
                    (isprint(UCHAR(byte)) ? byte : ' '));
        }
    }

    if (guard_failed)
    {
        dump_memory_info(stderr);
        fprintf(stderr, "high guard failed at %p, %s %d\n",
                memHeaderP->body, file, line);
        fflush(stderr);         /* In case name pointer is bad. */
        fprintf(stderr, "%ld bytes allocated at (%s %d)\n",
                memHeaderP->length,
                memHeaderP->file, memHeaderP->line);
        panic("Memory validation failure");
    }
    if (nukeGuards)
    {
        memset((char *) memHeaderP->low_guard, 0, GUARD_SIZE);
        memset((char *) hiPtr, 0, GUARD_SIZE);
    }
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_ValidateAllMemory --
 *  Validates guard regions for all allocated memory.
 *
 *----------------------------------------------------------------------
 */
OS_EXPORT void
Tcl_ValidateAllMemory(const char *file, int line)
{
    struct mem_header *memScanP;

    /*
     LogInfo("Validating Memory\n");
     */
    for (memScanP = allocHead; memScanP != NULL;
         memScanP = memScanP->flink)
        ValidateMemory(memScanP, file, line, 0);
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_DumpActiveMemory --
 *  Displays all allocated memory to stderr.
 *
 * Results:
 *  Return TCL_ERROR if an error accessing the file occures, `errno'
 *  will have the file error number left in it.
 *----------------------------------------------------------------------
 */
OS_EXPORT int
Tcl_DumpActiveMemoryToFP(fileP)
    FILE *fileP;
{
    struct mem_header *memScanP;
    char           *address;
    long            numExamined = 0;

    for (memScanP = allocHead; memScanP != NULL;
         memScanP = memScanP->flink)
    {

        address = &memScanP->body[0];
        numExamined++;

        fprintf(fileP, "%8p - %8p  %7ld bytes @ %s (%d)", address,
                address + memScanP->length - 1, memScanP->length,
                memScanP->file, memScanP->line);
        (void) fputc('\n', fileP);

        if (memScanP->length < 100)
        {
            fprintf(fileP, "%*s \"%s\"", 30, "",
                    strunctrl((char *) address, memScanP->length));
            (void) fputc('\n', fileP);
        }
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_DumpActiveMemory --
 *  Displays all allocated memory to stderr.
 *
 * Results:
 *  Return TCL_ERROR if an error accessing the file occures, `errno'
 *  will have the file error number left in it.
 *----------------------------------------------------------------------
 */
OS_EXPORT int
Tcl_DumpActiveMemory(fileName)
    const char     *fileName;
{
    FILE           *fileP;
    int             fd, status;

    if (fileName != NULL)
    {
        fd = irOpen(fileName, O_CREAT | O_TRUNC | O_WRONLY, 0666);

#ifndef	OS_WINDOWS_NT
        fileP = fdopen(fd, "w");
#else
        fileP = _fdopen(fd, "w");
#endif
        if (fileP == NULL)
            return TCL_ERROR;
    } else
    {
        fileP = stdout;
    }

    status = Tcl_DumpActiveMemoryToFP(fileP);

    if (fileName != NULL)
    {
        fclose(fileP);
    }
    return status;
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_DbCkalloc - debugging ckalloc
 *
 * Allocate the requested amount of space plus some extra for
 * guard bands at both ends of the request, plus a size,
 * panicing if there isn't enough space, then write in the
 * guard bands and return the address of the space in the
 * middle that the user asked for.
 *
 * The second and third arguments are file and line, these
 * contain the filename and line number corresponding to the
 * caller.
 * These are sent by the ckalloc macro; it uses the preprocessor
 * autodefines __FILE__ and __LINE__.
 *
 *----------------------------------------------------------------------
 */
OS_EXPORT void *
Tcl_DbCkalloc(size, file, line)
    unsigned int    size;
    const char     *file;
    int             line;
{
    struct mem_header *result;

    if (validate_memory)
        Tcl_ValidateAllMemory(file, line);

    result = (struct mem_header *) malloc((unsigned) size +
                                    sizeof(struct mem_header) + GUARD_SIZE);

    if (result == NULL)
    {
        fflush(stdout);
        dump_memory_info(stderr);
        panic("unable to alloc %d bytes, %s line %d", size, file, line);
    }
    /*
     * Fill in guard zones and size.  Also initialize the contents of
     * the block with bogus bytes to detect uses of initialized data.
     * Link into allocated list.
     */
    if (init_malloced_bodies)
    {
        memset((void *) result, GUARD_VALUE,
               size + sizeof(struct mem_header) + GUARD_SIZE);
    } else
    {
        memset((char *) result->low_guard, GUARD_VALUE, GUARD_SIZE);
        memset(result->body + size, GUARD_VALUE, GUARD_SIZE);
    }
    result->length = size;
    result->file = file;
    result->line = line;
    result->flink = allocHead;
    result->blink = NULL;
    if (allocHead != NULL)
        allocHead->blink = result;
    allocHead = result;

    total_mallocs++;
    if (trace_on_at_malloc && (total_mallocs >= trace_on_at_malloc))
    {
        (void) fflush(stdout);
        fprintf(stderr,
                "reached malloc trace enable point (%d)\n",
                total_mallocs);
        fflush(stderr);
        alloc_tracing = 1;
        trace_on_at_malloc = 0;
    }
    if (alloc_tracing)
        fprintf(stderr,
                "ckalloc %p %d %s %d\n", result->body, size,
                file, line);

    if (break_on_malloc && (total_mallocs >= break_on_malloc))
    {
        break_on_malloc = 0;
        (void) fflush(stdout);
        fprintf(stderr,
                "reached malloc break limit (%d)\n", total_mallocs);
        fprintf(stderr, "program will now enter C debugger\n");
        (void) fflush(stderr);
        abort();
    }
    current_malloc_packets++;
    if (current_malloc_packets > maximum_malloc_packets)
        maximum_malloc_packets = current_malloc_packets;
    current_bytes_malloced += size;
    if (current_bytes_malloced > maximum_bytes_malloced)
        maximum_bytes_malloced = current_bytes_malloced;

    return result->body;
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_DbCkfree - debugging ckfree
 *
 * Verify that the low and high guards are intact, and if so
 * then free the buffer else panic.
 *
 * The guards are erased after being checked to catch duplicate
 * frees.
 *
 * The second and third arguments are file and line, these
 * contain the filename and line number corresponding to the
 * caller.
 * These are sent by the ckfree macro; it uses the preprocessor
 * autodefines __FILE__ and __LINE__.
 *
 *----------------------------------------------------------------------
 */

OS_EXPORT int
Tcl_DbCkfree(ptr, file, line)
    void           *ptr;
    const char     *file;
    int             line;
{
    struct mem_header *memp = 0;/* Must be zero for size calc */

    /*
     * Since header ptr is zero, body offset will be size
     */
#ifdef _CRAYCOM
    memp = (struct mem_header *) ((char *) ptr
                              - (sizeof(int) * ((unsigned) &(memp->body))));
#else
    memp = (struct mem_header *) (((char *) ptr) - (long) memp->body);
#endif

    if (alloc_tracing)
        fprintf(stderr, "ckfree %p %ld %s %d\n", memp->body,
                memp->length, file, line);

    if (validate_memory)
        Tcl_ValidateAllMemory(file, line);

    ValidateMemory(memp, file, line, 1);
    if (init_malloced_bodies)
    {
        memset((void *) ptr, GUARD_VALUE, memp->length);
    }
    total_frees++;
    current_malloc_packets--;
    current_bytes_malloced -= memp->length;

    /*
     * Delink from allocated list
     */
    if (memp->flink != NULL)
        memp->flink->blink = memp->blink;
    if (memp->blink != NULL)
        memp->blink->flink = memp->flink;
    if (allocHead == memp)
        allocHead = memp->flink;
    free((char *) memp);
    return 0;
}


/*
 *--------------------------------------------------------------------
 *
 * Tcl_DbCkrealloc - debugging ckrealloc
 *
 * Reallocate a chunk of memory by allocating a new one of the
 * right size, copying the old data to the new location, and then
 * freeing the old memory space, using all the memory checking
 * features of this package.
 *
 *--------------------------------------------------------------------
 */
OS_EXPORT void *
Tcl_DbCkrealloc(ptr, size, file, line)
    void           *ptr;
    unsigned int    size;
    const char     *file;
    int             line;
{
    char           *new;
    unsigned int    copySize;
    struct mem_header *memp = 0;/* Must be zero for size calc */

#ifdef _CRAYCOM
    memp = (struct mem_header *) ((char *) ptr
                              - (sizeof(int) * ((unsigned) &(memp->body))));
#else
    memp = (struct mem_header *) (((char *) ptr) - (long) memp->body);
#endif
    copySize = size;
    if (copySize > memp->length)
    {
        copySize = memp->length;
    }
    new = Tcl_DbCkalloc(size, file, line);
    memcpy((void *) new, (void *) ptr, (int) copySize);
    Tcl_DbCkfree(ptr, file, line);
    return (new);
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_Ckstrdup --
 *  Interface to strdup when TCL_MEM_DEBUG is disabled.  It does
 *  check that memory was actually allocated.
 *
 *----------------------------------------------------------------------
 */
OS_EXPORT char *
Tcl_DbCkstrdup(string, file, line)
    const char     *string;
    const char     *file;
    int             line;
{
    char *result;
    int len;

    len = strlen(string) + 1;
    result = (char *) Tcl_DbCkalloc(len, file, line);
    if (result == NULL)
        panic("unable to alloc %d bytes", len);

    strlcpy(result, string, len);
    return result;
}

