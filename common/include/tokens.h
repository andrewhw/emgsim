/** ------------------------------------------------------------
 ** Read in tokenized values
 ** ------------------------------------------------------------
 ** $Id: tokens.h 89 2011-11-17 22:55:45Z andrew $
 **/

#ifndef TOKENIZER_HEADER__
#define TOKENIZER_HEADER__

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#endif

#include "os_defs.h"

#define	TKN_MAX_STR_SIZE		4096
#define	TOK_NAME_PROTECT_CHARS		" \t+-=()'\"?,;"

#define TKNIZER_TYPE_FILE		0x01
#define TKNIZER_TYPE_STRING		0x02
#define TKNIZER_TYPE_STRLIST		0x03

typedef struct token
{
    int type_;
    struct {
		const char *strptr_;
		int ival_;
		double dval_;
    } data_;
} token;

struct tokenizerString
{
    char *data_;
    size_t offset_;
};

struct tokenizerStrList
{
    const char ** data_;
    size_t lineOffset_;
    size_t charOffset_;
};

struct tokenizerFile
{
    FILE *ifp_;
};

typedef struct tokenizer
{
    int type_;
    union {
	struct tokenizerFile file_;
	struct tokenizerString string_;
	struct tokenizerStrList strList_;
    } typeData_;
    token token_;
    char tokenString_[TKN_MAX_STR_SIZE];
    int tokenStringLen_;
    int saveCh_;
    int lineNo_;
    int options_;
    int	 pushedToken_;
    struct {
	char echoFlush_;
	char sawNL_;
    } verbose_;
} tokenizer;

#define	TT_MAX_LITERAL	(0xFF)
#define	TT_STRING	(TT_MAX_LITERAL + 1)
#define	TT_CHARACTER	(TT_MAX_LITERAL + 2)
#define	TT_INTEGER	(TT_MAX_LITERAL + 3)
#define	TT_REAL		(TT_MAX_LITERAL + 4)
#define	TT_IDENTIFIER	(TT_MAX_LITERAL + 5)

#define	TT_UNKNOWN	(TT_MAX_LITERAL + 6)
#define	TT_EOF		(TT_MAX_LITERAL + 7)
#define	TT_MAX__	(TT_MAX_LITERAL + 8)


#define		TTOPT_RETURN_CR		0x01
#define		TTOPT_RETURN_COMMENTS	0x02
#define		TTOPT_VERBOSE_PARSE	0x04

#ifndef		lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT tokenizer *tknGetTokenizer(FILE *ifp);
OS_EXPORT tokenizer *tknGetFileTokenizer(FILE *ifp);
OS_EXPORT tokenizer *tknGetStringTokenizer(const char *string);
OS_EXPORT tokenizer *tknGetStrListTokenizer(const char **string);
OS_EXPORT void tknDeleteTokenizer(tokenizer *t);

OS_EXPORT void tknPushToken(tokenizer *t);
OS_EXPORT int  tknCompare(token *tok, const char *string);
OS_EXPORT int  tknCompareNoCase(token *tok, const char *string);
OS_EXPORT int  tknIsDelim(token *tok, const char *delimset);
OS_EXPORT int  tknIsIdChar(int ch);

OS_EXPORT void tknReset(tokenizer *t);
OS_EXPORT void tknFileReset(tokenizer *t, FILE *ifp);
OS_EXPORT void tknStringReset(tokenizer *t, const char *data);

OS_EXPORT void tknSetOptions(tokenizer *t, int flags);
OS_EXPORT int tknGetOptions(tokenizer *t);

OS_EXPORT token *tknGetToken(tokenizer *t);
OS_EXPORT int tknExpect(tokenizer *t, const char *s);
OS_EXPORT int tknGetLineNo(tokenizer *t);

OS_EXPORT int tknExpandToken(char *buffer, int size, tokenizer *t);
OS_EXPORT int tknPrintToken(FILE *fp, tokenizer *t);
OS_EXPORT int tknDumpToken(tokenizer *t);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif

#endif  /* TOKENIZER_HEADER__ */

