/** ------------------------------------------------------------
 ** Insertion sorting routines
 ** ------------------------------------------------------------
 ** $Id: isort.c 88 2011-10-21 19:06:51Z andrew $
 **/

#ifndef MAKEDEPEND
#include	<stdio.h>
#include	<string.h>
#endif

#include	"tclCkalloc.h"
#include	"msgisort.h"
#include	"msgdbids.h"
#include	"listalloc.h"
#include	"os_defs.h"


/*
 * ---------------------------------------------
 * Insert a block into the given list
 * We pass in a function to deal with the
 * insert.  This function takes the insertion
 * point, the new data, and a flag to determine
 * whether it is overwriting (ISORT_OVERWRITE),
 * or inserting into a new tile (ISORT_INSERT).
 *
 * Returns :    -1 on fail
 *  0 on insert
 *  1 if object is found (no insert performed)
 * ---------------------------------------------
 */
OS_EXPORT int
listInsertSort(
		void **list,
		void *newData,
		int *list_size,
		int *cur_blocks,
		int blksize,
		int tile_size,
		int (*compFunc)(void *listData, void *newData),
		int (*insertFunc)(void *targ, void *src, int flag)
	)
{
	int i, j;
	int returnIndex = (-1);
	int compResult;
	char *ilist;


	ilist = *list;
	for (i = 0; i < *list_size; i++)
	{
		compResult = (*compFunc)(
					(void *) &ilist[i * tile_size],
					newData
				);


		/** if we actually find the point, then call with overwrite **/
		if (compResult == 0)
		{
			(*insertFunc) ((void **) &ilist[i * tile_size],
							newData, ISORT_OVERWRITE);
			returnIndex = i;
			goto SUCCESS;

		/** if we need to insert in the middle, clear space **/
		} else if (compResult > 0)
		{
			/** check for failure from grow **/
			if ( ! listCheckSize((*list_size) + 1, list,
						cur_blocks, blksize, tile_size))
				return (-1);

			ilist = *list;

			for (j = *list_size - 1; j >= i; j--)
			{
				memcpy( &ilist[tile_size * (j + 1)],
						&ilist[tile_size * j],
						tile_size);
			}

			(*insertFunc) ((void *) &ilist[i * tile_size],
					newData, ISORT_INSERT);
			returnIndex = i;
			(*list_size)++;
			goto SUCCESS;
		}
	}

	/** insert at end **/
	if (!listCheckSize((*list_size) + 1, list,
				cur_blocks, blksize, tile_size))
		return (-1);

	ilist = *list;
	(*insertFunc) ((void **) &ilist[(*list_size) * tile_size],
				newData, ISORT_INSERT);
	returnIndex = (*list_size);
	(*list_size)++;

	SUCCESS:
	return (returnIndex);
}


