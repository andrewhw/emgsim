/**
 * Calculate various statistics about the contraction
 *
 * $Id: statistics.h 5 2008-04-24 22:05:30Z andrew $
 */

#ifndef __SIM_STATISTICS_HEADER__
#define __SIM_STATISTICS_HEADER__

#include        "SimulatorControl.h"

class DQEmgData;
class SimulationResult;

int calculateStatistics(
						SimulationControl *controlvalues,
						SimulationResult *simData,
		                DQEmgData *dqemgContraction
		        );

#endif /* __SIM_STATISTICS_HEADER__ */


