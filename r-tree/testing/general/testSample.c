/**
 * $Id: testSample.c 4 2008-04-24 21:25:46Z andrew $
 */

#include <stdio.h>
#include <string.h>

#include "rTreeIndex.h"

#include "testutils.h"


/** xmin, ymin, xmax, ymax (for 2 dimensional RTree) */
static struct Rect rects[] = {
    {0, 0, 2, 2},
    {5, 5, 7, 7},
    {8, 5, 9, 6},
    {7, 1, 9, 2}
};

static int nrects = sizeof(rects) / sizeof(rects[0]);
/** search will find above rects that this one overlaps */
static struct Rect search_rect = {
    {6, 4, 10, 6}
};

static int MySearchCallback(int id, void* arg) 
{
    /* Note: -1 to make up for the +1 when data was inserted */
    /* printf("Hit data rect %d\n", id-1); */
    return 1; /* keep going */
}

int testSample()
{
    struct Node* root = RTreeNewIndex();
    int i, nhits;

    /*
    printf("nrects = %d\n", nrects);
    */

    /*
     * Insert all the data rects.
     * Notes about the arguments:
     * parameter 1 is the rect being inserted,
     * parameter 2 is its ID. NOTE: *** ID MUST NEVER BE ZERO ***, hence the +1,
     * parameter 3 is the root of the tree. Note: its address is passed
     * because it can change as a result of this call, therefore no other parts
     * of this code should stash its address since it could change undernieth.
     * parameter 4 is always zero which means to add from the root.
     */
    for(i=0; i<nrects; i++) {
                /* i+1 is rect ID. Note: root can change */
        RTreeInsertRect(&rects[i], i+1, &root, 0);
    }
    nhits = RTreeSearch(root, &search_rect, MySearchCallback, 0);

    /*
    printf("Search resulted in %d hits\n", nhits);
    */
    if (nhits == 2) {
	PASS(__FILE__, __LINE__, "Found correct number (2) hits\n");
    } else {
	FAIL(__FILE__, __LINE__, "Expected 2 hits, found %d\n", nhits);
    }

    return 1;
}

/**
 * $Log$
 * Revision 1.1  2004/09/23 14:23:03  andrew
 * o Added test case
 *
 */

