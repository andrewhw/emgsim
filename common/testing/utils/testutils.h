/**
 * $Id: testutils.h 10 2008-04-24 18:37:51Z andrew $
 */

#ifndef __TESTSUIT_UTILS_HEADER__
#define __TESTSUIT_UTILS_HEADER__

void PASS(const char *file, int line, const char *fmt, ...);
void FAIL(const char *file, int line, const char *fmt, ...);
void TEST(const char *file, int line, const char *fmt, ...);
void DBG(const char *file, int line, const char *fmt, ...);

#define MK	__FILE__, __LINE__

#endif /* __TESTSUIT_UTILS_HEADER__ */

