/**
 * $Id: testutils.h 36 2008-09-19 18:46:54Z andrew $
 */

#ifndef __TESTSUIT_UTILS_HEADER__
#define __TESTSUIT_UTILS_HEADER__

void PASS(const char *file, int line, const char *fmt, ...);
void FAIL(const char *file, int line, const char *fmt, ...);
void TEST(const char *file, int line, const char *fmt, ...);
void DBG(const char *file, int line, const char *fmt, ...);

#define MK	__FILE__, __LINE__

#endif /* __TESTSUIT_UTILS_HEADER__ */

