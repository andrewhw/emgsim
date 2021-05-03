/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: attvalfile.h 97 2012-05-09 01:50:07Z andrew $
 **/

#ifndef ATTRIBUTE_VALUE_FILE_HEADER__
#define ATTRIBUTE_VALUE_FILE_HEADER__

#ifndef MAKEDEPEND
#include <string.h>
#endif

#include        "os_defs.h"

typedef struct attVal {
        int                     type_;
        const char      *attribute_;            
        union {
                const char      *strptr_;
                int                     ival_;
                double          dval_;
        } data_;
} attVal;

typedef struct attValList {
        int             size_;
        int             curBlocks_;
        attVal  **list_;                
} attValList;


#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

struct tokenizer;

OS_EXPORT attValList *loadAttValFile(const char *filename);
OS_EXPORT attValList *parseAttValFile(FILE *ifp);
OS_EXPORT attVal *parseAttVal(FILE *ifp);
OS_EXPORT attVal *parseAttValFromTokenizer(struct tokenizer *t);

OS_EXPORT attVal *getAttVal(attValList *list, const char *key);

OS_EXPORT attVal *createStringAttribute(
                        const char *tag, const char *value);
OS_EXPORT attVal *createIntegerAttribute(const char *tag, int value);
OS_EXPORT attVal *createRealAttribute(const char *tag, double value);
OS_EXPORT int addAttVal(attValList *list, attVal *data);
OS_EXPORT int updateAttVal(attValList *list, attVal *data);

OS_EXPORT int writeAttValList(FILE *ofp, attValList *list,
                        const char * comment);

OS_EXPORT void deleteAttVal(attVal *item);
OS_EXPORT void deleteAttValList(attValList *list);
OS_EXPORT attValList * createAttValList();
# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif

#endif  /* ATTRIBUTE_VALUE_FILE_HEADER__ */

