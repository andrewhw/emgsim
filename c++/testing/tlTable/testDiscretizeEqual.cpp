#include <stdio.h>

#include "tools.h"

int
testDiscretizeEqual()
{
	int status, s;

	s = testDiscretize1to16(tlBinTable::EQUAL_BIN_RANGE);
	status = s;

	s = testDiscretizeFloatSeries(tlBinTable::EQUAL_BIN_RANGE);
	status = s && status;

	return status;
}
