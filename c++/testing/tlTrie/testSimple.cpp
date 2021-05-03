#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tlTrie.h"


int
testSimpleIntProgression()
{
    tlTrie t;
	int query;
	int key, value;
	long result;
    int j, k;

    printf("<TESTCASE> testSimpleIntProgression()\n");

	for (j = 0; j < 32; j++) {
		key = j * j * j * j;
		value = j;

		t.add(&key, sizeof(key), value);

		for (k = 0; k <= j; k++) {
			query = k * k * k * k;

			if (t.query(&result, &query, sizeof(query)) < 0)
			{
				printf("<FAIL> -- nothing values found for %8d\n",
						query);
			} else
			{
				if (result == k) {
					printf("<PASS> -- Value at %8d is : %ld\n",
								query, result);
				} else {
					printf("<FAIL> -- Value at %8d is : %8ld -- expected %d\n",
								query, result, k);
				}
			}
		}
	}

    return 1;
}

int
testSimpleOverwrite()
{
    tlTrie t;
	int values[] = { 2, 127, 1024, 16384 };
	short key;
	long result;
    int i, j, foundIndex, r;

    printf("<TESTCASE> testSimpleIntProgression()\n");

	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 4; i++)
		{
			key = values[i];
			t.add(&key, sizeof(key), values[i]);
		}
	}

	for (i = 0; i < SHRT_MAX; i++)
	{
		key = i;
		r = t.query(&result, &key, sizeof(key));

		foundIndex = -1;
		for (j = 0; j < 4; j++)
		{
			if (key == values[j])
			{
				foundIndex = j;
				break;
			}
		}

		if (r < 0)
		{
			if (foundIndex >= 0)
			{
				printf("<FAIL> -- inserted value %d NOT found for key %d\n",
						values[foundIndex], key);
			} else
			{
				printf("<PASS> -- non-inserted value marked as not found\n");
			}
		} else
		{
			if (foundIndex >= 0)
			{
				printf("<PASS> -- inserted value marked as found\n");
			} else
			{
				printf("<FAIL> -- NON-inserted value FOUND\n");
			}
		}
	}

    return 1;
}


int
testSimple()
{
    int status;

    status = testSimpleIntProgression();
    status = testSimpleOverwrite();

    return status;
}
