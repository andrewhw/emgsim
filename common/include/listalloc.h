/** ------------------------------------------------------------
 ** list allocation tools
 ** ------------------------------------------------------------
 ** $Id: listalloc.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         LIST_ALLOCATION_HEADER__
#define         LIST_ALLOCATION_HEADER__

#ifndef MAKEDEPEND
# include       <stdio.h>
#endif

#include        "os_defs.h"


#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

typedef struct lsBag {
    int         nElem_;
    void        **d_;

    int         nBlocks_;
    int         blockSize_;

    char	*name_;
} lsBag;

    /** memlist.c **/
#define listCheckSize(size_needed, list, cur_blocks, blksize, tilesize) \
		listMkCheckSize(size_needed, list, \
				cur_blocks, blksize, \
				tilesize, __FILE__, __LINE__)
	
OS_EXPORT int listMkCheckSize(int size_needed, void **list,
				int *cur_blocks, int blksize, int tilesize,
				const char *filemark, int linemark);


OS_EXPORT lsBag *lsCreate(const char *name, int blocksize);

OS_EXPORT void lsClear(lsBag *list);

OS_EXPORT void lsDelete(lsBag *list);

OS_EXPORT int lsAdd(lsBag *list, void *newElement);

OS_EXPORT void *lsGet(lsBag *list, int index);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* lint */

#endif  /* LIST_ALLOCATION_HEADER__     */


