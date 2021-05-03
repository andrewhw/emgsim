/**
 * $Id: testutils.cpp 36 2008-09-19 18:46:54Z andrew $
 */

#include <stdio.h>
#include <stdarg.h>


void
PASS(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<PASS> ");
    va_start(vargs, fmt);
    (void) vfprintf(stdout, fmt, vargs); 
    va_end(vargs);
}

void
FAIL(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<FAIL> at '%s' (%d):\n", file, line);
    printf("<FAIL> ");
    va_start(vargs, fmt);
    (void) vfprintf(stdout, fmt, vargs); 
    va_end(vargs);
}
void
TEST(const char *file, int line, const char *fmt, ...)
{
    va_list vargs;

    printf("<TEST> ");
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
 * Revision 1.1  2004/01/13 17:25:08  andrew
 * o Testsuite updated with correct version
 *
 * Revision 1.1  2004/01/11 19:02:09  andrew
 * o Adding in Type-2 sets to testsuite
 *
 */

