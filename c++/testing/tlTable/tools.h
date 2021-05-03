
#ifndef	__TOOL_HEADER__
#define	__TOOL_HEADER__

#include <stdio.h>
#include "tlBinTable.h"

int testDiscretize1to16(tlBinTable::BinStrategy);
int testDiscretizeFloatSeries(tlBinTable::BinStrategy);
int testDiscretizeXorFloatSeries(tlBinTable::BinStrategy strategy);

#define	FILENAME	"tmpData.txt"
#define	DELIM_SET	",;"

#endif // __TOOL_HEADER__
