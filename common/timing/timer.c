/** ------------------------------------------------------------
 ** Guess-how-long-until-done tools
 ** ------------------------------------------------------------
 ** $Id: timer.c 81 2010-07-12 19:38:05Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKDEPEND
#include <time.h>
#include <math.h>
#endif

#include "tclCkalloc.h"
#include "reporttimer.h"
#include "stringtools.h"


/**
 ** Create a new timer
 **/
OS_EXPORT struct report_timer *
startReportTimer(osInt64 numEvents)
{
	struct report_timer *result;

	result = (struct report_timer *)
					ckalloc(sizeof(struct report_timer));
	result->start_time_ = time(NULL);
	result->num_events_ = numEvents;

	return result;
}


/**
 ** report on time's passage, relative to a timer
 **/
OS_EXPORT const char *
reportTime(osInt64 currentIteration, const struct report_timer *timer)
{
	int             width, endSec, endMin, startSec, startMin;
	double          percent;
	time_t          curTime, diffTime, endTime;
	char            buffer[256];


	endMin = startMin = 0;

	width = (int) log10((double) timer->num_events_) + 1;
	percent = (currentIteration / (double) timer->num_events_);
	curTime = time(NULL);

	/** calc diff between start and now */
	diffTime = curTime - timer->start_time_;

	if (diffTime < 15)
	{
		slnprintf((char *)timer->obuf, REPORT_BUFSIZ, "%*ld of %*ld (%5.2f)%%",
				width, (long) currentIteration,
				width, (long) timer->num_events_,
				(float) (percent * 100.0));
		return timer->obuf;
	}
	startSec = (int) diffTime;
	endTime = (int) (diffTime / percent);

	/** calc diff between now and end */
	endSec = (int) endTime - (int) diffTime;
	if (endSec > 60)
	{
		endMin = endSec / 60;
		endSec = endSec - (endMin * 60);
	}
	if (startSec > 60)
	{
		startMin = startSec / 60;
		startSec = startSec - (startMin * 60);
	}
	/*
			slnprintf(buffer, 256,
							"(%d:%02ds elapsed - %d:%02ds remain)",
						startMin, startSec,
						endMin, endSec);
	*/
	if (endMin == 0 && endSec == 0)
	{
		buffer[0] = 0;
	} else
	{
		if (endMin > 0)
		{
			slnprintf(buffer, 256, "(%dm %02ds remain)", endMin, endSec);
		} else
		{
			slnprintf(buffer, 256, "(%02ds remain)", endSec);
		}
	}

	slnprintf((char *)timer->obuf, REPORT_BUFSIZ, "%*ld of %*ld (%5.2f)%% %s",
			width, (long) currentIteration,
			width, (long) timer->num_events_,
			(float) (percent * 100.0),
			buffer
		);
	return timer->obuf;
}

/**
 ** clean up data allocated within a timer
 **/
OS_EXPORT void
deleteReportTimer(struct report_timer * delTimer)
{
	ckfree((void *) delTimer);
}

