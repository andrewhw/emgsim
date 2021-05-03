/**
 * $Id: testutils.c 4 2008-04-24 21:25:46Z andrew $
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

/**
 * $Log$
 * Revision 1.1  2004/09/23 14:23:03  andrew
 * o Added test case
 *
 */

