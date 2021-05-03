#include <stdio.h>
#include "mathtools.h"
#include "testutils.h"

static struct {
    long val;
    long result;
} sData[] = {
	{0, 	0},
	{1, 	1},
	{2,		2},
	{3,		6},
	{4,		24},
	{5,		120},
	{6,		720},
	{7,		5040},
	{8,		40320},
	{9,		362880},
	{10,	3628800},
	{11,	39916800},
	{12,	479001600},
	{13,	6227020800},
	{14,	87178291200},
	{15,	1307674368000},
	{16,	20922789888000},
	{17,	355687428096000},
	{18,	6402373705728000},
	{19,	121645100408832000},
	{20,	2432902008176640000},
	{21,	-1},
	{-1,	-1},
    };

int
testFactorial()
{
    int status = 1;
	long result;
    int i;

    for (i = 0; sData[i].val >= 0; i++)
	{
		result = factorial(sData[i].val);

		if (result == sData[i].result)
		{
			PASS(__FILE__, __LINE__,
					"Got expected factorial value %ld\n",
					result);
		}
		else
		{
			FAIL(__FILE__, __LINE__,
					"Expected factorial of %ld to be %ld, got %ld\n",
					sData[i].val, sData[i].result, result);
		}
    }

    return status;
}
