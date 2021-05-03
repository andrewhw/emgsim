#include <stdio.h>

#include "tools.h"

int
testDiscretizeMMEIgnoreUnique_Float()
{
	return testDiscretizeFloatSeries(tlBinTable::MME_IGNORE_UNIQUE);
}
