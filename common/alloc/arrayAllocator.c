/*
 * "Array" allocation utility.  Returns two- and three-dimensional
 * array-like constructs to provide runtime access to scalable
 * array structures in C.
 */

#include	<stdio.h>
#include	<string.h>

			/* for OS_EXPORT */
#include	"os_defs.h"

			/* for our local definitions */
#include	"arrayAllocator.h"
#include	"tclCkalloc.h"

/*
 * Allocate an array-like storage for a given tile size.
 *
 * Note that the result is indexed in row-major order, meaning
 * that we really have a list of rows that we are returning.
 * This also implies that applications assuming a contiguous
 * storage block may expect that the first element of row N
 * will be stored immediately after the last element of row N-l
 */
OS_EXPORT void **
alloc2darray(
		size_t nRows,
		size_t nCols,
		size_t elementSize
	)
{
	void **result;
	unsigned char *buffer;
	size_t indexSize, rowSize, dataSize;
	size_t i;

	indexSize = nRows * sizeof(void *);
	rowSize = nCols * elementSize;
	dataSize = nRows * rowSize;

	/**
	 * allocate, in a single block, enough memory for
	 * a list of pointers to use to point at the rows, 
	 * plus a block of memory for storing the data
	 */
	result = (void **) ckalloc(indexSize + dataSize);
	if (result == NULL)
		return NULL;


	/** set up the pointer to the block of memory to use for storing the data */
	buffer = (unsigned char *) &((char *) result)[indexSize];

	/** paint the buffer to be full of zeros */
	memset(buffer, 0x00, dataSize);

	/**
	 * stitch the rows into the index list, allowing the memory
	 * to look like an array
	 *
	 * this means that each cell in result has to point at the
	 * beginning of the correct row, and there are nCols entries
	 * accessed in that row at adjacent memory locations
	 */
	for (i = 0; i < nRows; i++)
	{
		result[i] = &buffer[i * rowSize];
	}

	return result;
}


/**
 * Three dimensional array allocation.
 *
 * Similar ideas to 2d, but row-major, has now become depth-major,
 * then row (semi-major), then column, so as we walk across the
 * data block we expect to traverse a row from column to column,
 * appearing one row down at the end (as before), however if we
 * keep going, we will eventually walk from depth 0 to depth 1,
 * reappearing at the first column and row at the next depth
 * every time we do this.
 *
 * This in turn implies that while the block of data values needs
 * to have enough storage for all the elements, there also needs
 * to be a second level storage with nDepth x nRows pointers to
 * point at the data block, as well as a primary index level with nDepth
 * pointers to point at "sets of rows" in the level 2 index.
 *
 * The arithmetic gets a little tricky here, so the best thing
 * is to draw a picture.  Here are the bits of storage for a 2x3x4
 * array
 *
 * pointer returned:
 *     +----+
 *     |    |
 *     +----+
 *
 * "primary index" - handles depth (pointed to by return value):
 *     +----+----+
 *     | P0 | P1 |
 *     +----+----+
 *
 * 
 * "secondary index" - each "row" in this is pointed to by one of
 * the above primary index values.  Each value here points at
 * a row of data in the data block.  The equal signs delimit the
 * parts relating to depth 0 from the parts relating to depth 1.
 *     +----+----+----+
 *  P0 | S0 | S1 | S2 |
 *     +====+====+====+
 *  P1 | S3 | S4 | S5 |
 *     +----+----+----+
 *
 * "data" - each row in this is a data "row", and is pointed to
 * by the indicated above secondary index value.  The values
 * indicated in the blocks refer to the final data index number.
 *     +----+----+----+----+
 *  S0 |  0 |  1 |  2 |  3 |
 *     +----+----+----+----+
 *  S1 |  4 |  5 |  6 |  7 |
 *     +----+----+----+----+
 *  S2 |  8 |  9 |  10|  11|
 *     +====+====+====+====+
 *  S3 |  12|  13|  14|  15|
 *     +----+----+----+----+
 *  S4 |  16|  17|  18|  19|
 *     +----+----+----+----+
 *  S5 |  20|  20|  22|  23|
 *     +----+----+----+----+
 */
OS_EXPORT void ***
alloc3darray(
		size_t nDepths,
		size_t nRows,
		size_t nCols,
		size_t elementSize
	)
{
	unsigned char *buffer, *ucharResult;
	void ***result;
	void **index;
	size_t outerIndexSize, innerIndexSize, rowSize, dataSize;
	size_t indexOffset, bufferOffset;
	size_t i, j;

	/* calculate the sizes of the various portions */
	outerIndexSize = nDepths * sizeof(void **);
	innerIndexSize = nDepths * nRows * sizeof(void *);
	rowSize = nCols * elementSize;
	dataSize = nDepths * nRows * rowSize;


	/** allocate a list of pointers to use to point at the rows */
	result = (void ***)
			ckalloc(outerIndexSize + innerIndexSize + dataSize);

	if (result == NULL)
		return result;


	ucharResult = (unsigned char *) result;
	index = (void **) &ucharResult[outerIndexSize];
	buffer = (unsigned char *) &ucharResult[
				outerIndexSize + innerIndexSize
			];

	/** paint the buffer to be full of zeros */
	memset(buffer, 0x00, dataSize);


	/**
	 * make the result (at a given offset) point at the
	 * right part of the index.  There are nDepths entries
	 * in the result, and (nDepths x nRows) entries in the
	 * index.
	 */
	for (i = 0; i < nDepths; i++)
	{
		result[i] = &index[i * nRows];


		/**
		 * For each depth, stitch together the index->rows
		 * portion
		 */
		for (j = 0; j < nRows; j++)
		{
			indexOffset = i * nRows + j;
			bufferOffset = ((i * nRows * nCols) + (j * nCols))
					* elementSize;

			index[indexOffset] = &buffer[bufferOffset];
		}
	}

	return result;
}

