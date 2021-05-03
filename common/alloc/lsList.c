/** ------------------------------------------------------------
 ** list allocation and scaling routines
 ** ------------------------------------------------------------
 ** $Id: lsList.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <string.h>
#endif

#include        "tclCkalloc.h"
#include        "listalloc.h"
#include        "os_defs.h"

/*
 * ---------------------------------------------
 * create a managed listBag
 * ---------------------------------------------
 */
OS_EXPORT lsBag *
lsCreate(id, blockSize)
	const char     *id;
	int             blockSize;
{
	lsBag          *result;

	result = (lsBag *) ckalloc(sizeof(lsBag));
	memset(result, 0, sizeof(lsBag));

	if (blockSize > 0)
		result->blockSize_ = blockSize;
	else
		result->blockSize_ = 16;

	result->name_ = ckstrdup(id);
	return result;
}

/*
 * ---------------------------------------------
 * delete a managed listBag
 * ---------------------------------------------
 */
OS_EXPORT void
lsDelete(bag)
    lsBag          *bag;
{
	lsClear(bag);
	ckfree(bag);
}

/*
 * ---------------------------------------------
 * clear allocations managed by a listBag
 * ---------------------------------------------
 */
OS_EXPORT void
lsClear(bag)
    lsBag          *bag;
{
	if (bag->d_ != NULL)
		ckfree(bag->d_);

	if (bag->name_ != NULL)
		ckfree(bag->name_);

	memset(bag, 0, sizeof(lsBag));
}

/*
 * ---------------------------------------------
 * add something to the (end) of the list
 * ---------------------------------------------
 */
OS_EXPORT int
lsAdd(bag, newElement)
    lsBag          *bag;
    void           *newElement;
{
	if (!listMkCheckSize(
						 bag->nElem_ + 1,
						 (void **) &bag->d_,
						 &bag->nBlocks_,
						 bag->blockSize_,
						 sizeof(void *),
						 __FILE__, __LINE__))
	{
		return 0;
	}
	bag->d_[bag->nElem_++] = newElement;

	return 1;
}



/*
 * ---------------------------------------------
 * get something from the list
 * ---------------------------------------------
 */
OS_EXPORT void *
lsGet(bag, index)
    lsBag          *bag;
    int             index;
{
	if (index < 0 || index >= bag->nElem_)
		return NULL;

	return bag->d_[index];
}

