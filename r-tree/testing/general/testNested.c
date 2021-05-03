/**
 * $Id: testNested.c 4 2008-04-24 21:25:46Z andrew $
 */

#include <stdio.h>
#include <string.h>

#include "rTreeIndex.h"

#include "testutils.h"


/** xmin, ymin, xmax, ymax (for 2 dimensional RTree) */
static struct Rect rects[] = {
    {0, 0, 0, 0},
    {-0.25, -0.25, 0.25, 0.25},
    {-0.5, -0.5, 0.5, 0.5},
    {-1, -1, 1, 1},
    {-2, -2, 2, 2},
    {-3, -3, 3, 3},
};

static int nrects = sizeof(rects) / sizeof(rects[0]);

/** search will find above rects that this one overlaps */
static struct Rect search_rectmin = {
    {0, 0, 0, 0}
};
static struct Rect search_rectmax = {
    {-4, -4, 4, 4}
};

static int MySearchCallback(int id, void* arg) 
{
    /* Note: -1 to make up for the +1 when data was inserted */
    /* printf("Hit data rect %d\n", id-1); */
    return 1; /* keep going */
}

int testNested()
{
    struct Node* root = RTreeNewIndex();
    int i, nhits, nExpected;

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


    nExpected = 6;
    nhits = RTreeSearch(root, &search_rectmin, MySearchCallback, 0);
    if (nhits == nExpected) {
	PASS(__FILE__, __LINE__, "Found correct number (%d) hits\n", nExpected);
    } else {
	FAIL(__FILE__, __LINE__, "Expected %d hits, found %d\n",
			nExpected, nhits);
    }


    nExpected = 6;
    nhits = RTreeSearch(root, &search_rectmax, MySearchCallback, 0);
    if (nhits == nExpected) {
	PASS(__FILE__, __LINE__, "Found correct number (%d) hits\n", nExpected);
    } else {
	FAIL(__FILE__, __LINE__, "Expected %d hits, found %d\n",
			nExpected, nhits);
    }

    return 1;
}

/**
 * $Log$
 * Revision 1.1  2004/09/23 14:23:03  andrew
 * o Added test case
 *
 */

