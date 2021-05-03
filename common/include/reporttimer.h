/** ------------------------------------------------------------
 ** Time-to-completion guessing tools
 ** ------------------------------------------------------------
 ** $Id: reporttimer.h 78 2010-04-24 15:53:40Z andrew $
 **/

#ifndef __REPORT_TIMERS_HEADER__
#define __REPORT_TIMERS_HEADER__

#ifndef MAKEDEPEND
# include       <time.h>
#endif

# include       "os_defs.h"
# include       "os_types.h"

#define	REPORT_BUFSIZ	128

struct report_timer
{
	time_t	start_time_;
	osInt64	num_events_;
	char obuf[REPORT_BUFSIZ];
};


#ifndef         lint
/** 
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

/** begin the timer sequence */
OS_EXPORT struct report_timer *startReportTimer(osInt64 numEvents);

/** iterate the timer */
OS_EXPORT const char *reportTime(
                        osInt64 currentIteration,
                        const struct report_timer *timer
                );

/** destroy a timer */
OS_EXPORT void deleteReportTimer(struct report_timer *timer);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif /* __REPORT_TIMERS_HEADER__ */


