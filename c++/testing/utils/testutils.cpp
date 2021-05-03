/**
 * $Id: testutils.cpp 10 2008-04-24 18:37:39Z andrew $
 */

#include <stdio.h>
#include <stdarg.h>


void
PASS(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<PASS> +   ");
    va_start(vargs, fmt);
    (void) vfprintf(stdout, fmt, vargs); 
    va_end(vargs);
}

void
FAIL(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<FAIL> at '%s' (%d):\n", file, line);
    printf("<FAIL> >>> ");
    va_start(vargs, fmt);
    (void) vfprintf(stdout, fmt, vargs); 
    va_end(vargs);
    printf("<FAIL> <<<\n");
}

void
TEST(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<TEST> :   ");
	fflush(stdout);
    va_start(vargs, fmt);
    (void) vfprintf(stdout, fmt, vargs); 
    va_end(vargs);
}

void
DBG(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<DEBUG> ");
    va_start(vargs, fmt);
    (void) vfprintf(stdout, fmt, vargs); 
    va_end(vargs);
}

