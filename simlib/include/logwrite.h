/**
 ** Log and report on file writing activity as an aid to debugging.
 **
 ** $Id: logwrite.h 4 2008-04-24 21:27:41Z andrew $
 **/

#ifndef __LOGGED_FWRITE_HEADER__
#define __LOGGED_FWRITE_HEADER__

#ifndef MAKEDEPEND
# include       <stdio.h>
#endif

extern void logNote(const char *fmt, ...);
extern size_t logfwrite(
		                void *ptr, const char *tag,
		                const char *file, int line,
		                size_t size, size_t nelem, FILE *fp
		        );

#endif  /* __LOGGED_FWRITE_HEADER__     */


