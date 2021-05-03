/**
 * $Id: testutils.h 4 2008-04-24 21:25:46Z andrew $
 */

#ifndef __TESTSUIT_UTILS_HEADER__
#define __TESTSUIT_UTILS_HEADER__

void PASS(const char *file, int line, const char *fmt, ...);
void FAIL(const char *file, int line, const char *fmt, ...);
void TEST(const char *file, int line, const char *fmt, ...);
void DBG(const char *file, int line, const char *fmt, ...);

#define MK	__FILE__, __LINE__

#endif /* __TESTSUIT_UTILS_HEADER__ */

/**
 * $Log$
 * Revision 1.1  2004/09/23 14:23:03  andrew
 * o Added test case
 *
 * Revision 1.1  2004/01/11 19:02:09  andrew
 * o Adding in Type-2 sets to testsuite
 *
 */

