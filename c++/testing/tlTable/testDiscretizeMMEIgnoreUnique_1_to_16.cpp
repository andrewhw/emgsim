#include <stdio.h>

#include "tools.h"

int
testDiscretizeMMEIgnoreUnique_1_to_16()
{
	return testDiscretize1to16(tlBinTable::MME_IGNORE_UNIQUE);
}
