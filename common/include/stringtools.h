/** ------------------------------------------------------------
 ** Tools to perform many handy string functions.
 ** ------------------------------------------------------------
 ** $Id: stringtools.h 81 2010-07-12 19:38:05Z andrew $
 **/

#ifndef         STRINGTOOLS_HEADER__
#define         STRINGTOOLS_HEADER__

#include        <os_defs.h>

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <sys/types.h>
# ifndef                OS_WINDOWS_NT
# include       <netinet/in.h>
# endif
#endif

#include        "tclCkalloc.h"

#include        "os_defs.h"
#include        "log.h"


#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT int slnprintf(char *buffer, int buflen,
			const char *format, ...);

OS_EXPORT char **strExecParse(const char *src,
			const char *quoteString,
			const char *breakString);
OS_EXPORT char *strExecJoin(const char **execString);
OS_EXPORT char *strTruncBlank(char *string);
OS_EXPORT char *strTrim(char *string);
OS_EXPORT char *strconcat(const char *leadptr, ...);
OS_EXPORT char *strqconcat(const char *leadptr, ...);
OS_EXPORT char *strunctrl(const char *str, int size);
OS_EXPORT char *chunctrl(int character);
OS_EXPORT char *ckstrndup(const char *str, int size);
OS_EXPORT char *ckstrdup_toupper(const char *str);
OS_EXPORT char *ckstrdup_tolower(const char *str);
OS_EXPORT char *strPid(void);
OS_EXPORT char *strUnique(void);
OS_EXPORT char *strIPaddr(struct in_addr *address);
OS_EXPORT char *strtolower(char *source);
OS_EXPORT char *strtoupper(char *source);
OS_EXPORT int strTimeToDelta(const char *timestring);
OS_EXPORT char *strTimeToString(time_t time);

OS_EXPORT const char *strbasename(const char *string);
OS_EXPORT int split(
			char **fill_array,
			const char *delims,
			const char *source,
			int max
		);

		/** return a string which will make a nice filename  */
OS_EXPORT char *niceFilename(const char *buffer);


		/** trim a number in a buffer to max one zero after decimal */
OS_EXPORT void trimFloatBuffer(char *buffer);

		/** return a pre-trimmed double value */
OS_EXPORT char *niceDouble(double value);

		/** trim a double to int if possible */
OS_EXPORT char *fullyTrimmedDouble(double value);


typedef struct strFormatData {
    int nBlocks_;
    char *data_;
    size_t indentLen_;
    char *indentStr_;

    size_t maxLineLen_;
    size_t curLineLen_;
    size_t loadOffset_;
    int continueLine_;
} strFormatData;

		/** format a block of text to fit in a given width */
OS_EXPORT struct strFormatData *createFormatter(
			const char *indentString, int maxWidth
		);

OS_EXPORT void deleteFormatter(struct strFormatData *config);

OS_EXPORT char *formatParagraph(struct strFormatData *config,
			const char *firstIndent,
			const char *data);

#ifdef OS_NEEDS_STRLCPY
OS_EXPORT size_t strlcpy(char *dst, const char *src, size_t size);
OS_EXPORT size_t strlcat(char *dst, const char *src, size_t size);
#endif

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif  /* STRINGTOOLS_HEADER__ */

