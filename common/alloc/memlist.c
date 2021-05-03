/** ------------------------------------------------------------
 ** list allocation and scaling routines
 ** ------------------------------------------------------------
 ** $Id: memlist.c 17 2008-07-03 17:24:49Z andrew $
 **/

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#endif

#include        "tclCkalloc.h"
#include        "listalloc.h"
#include        "os_defs.h"

#ifdef TAGS_ONLY
int
listCheckSize()
#endif

/*
 * ---------------------------------------------
 * grow a memory list to the needed size
 * ---------------------------------------------
 */
OS_EXPORT int
listMkCheckSize(size_needed, list,
		cur_blocks, blksize, tilesize,
		filemark, linemark
	)
		int size_needed;
		void **list;
		int *cur_blocks, blksize, tilesize;
		const char *filemark;
		int linemark;
{
	int             calcblocks, calcbytes;
	void           *old_data;

	/** check whether we are just in the simple case **/
	if (size_needed <= (*cur_blocks) * blksize)
	{
		return (1);
	}
	/**
	 ** calculate the number of blocks we will need,
	 ** allowing for truncation of the number
	 **/
	calcblocks = (size_needed / blksize) + 1;

	/** convert to number of bytes      **/
	calcbytes = calcblocks * blksize * tilesize;

	/** allocate and copy old values **/
	old_data = (*list);
	(*list) = (void *) ckmkalloc(calcbytes, filemark, linemark);
	if ((*list) == NULL)
	{
		(*list) = old_data;
		return (0);
	}
	memset((*list), 0, calcbytes);

	if (old_data != NULL)
	{
		/** copy over old size data **/
		(void) memcpy((*list), old_data,
					  (*cur_blocks) * blksize * tilesize);
		ckfree(old_data);
	}
	(*cur_blocks) = calcblocks;

	return (1);
}

