/** ------------------------------------------------------------
 ** Insertion sorter
 ** ------------------------------------------------------------
 ** $Id: msgisort.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         WB_MSGISORT_HEADER__
#define         WB_MSGISORT_HEADER__

#ifndef MAKEDEPEND
# include       <stdio.h>
#endif

/** 
 ** MACRO DEFINITIONS
 **/
#define         ISORT_INSERT            1
#define         ISORT_OVERWRITE         2


#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    /** memlist.c **/
OS_EXPORT int listInsertSort(
		void **list, void *newData,
		int *list_size, int *cur_blocks,
		int blocksize, int tile_size,
		int (*compFunc)(void *listElement, void *newData),
		int (*insertFunc)(void *target, void *source, int control) );

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* lint */

#endif  /* WB_MSGISORT_HEADER__ */

