/**************************************
Author:             Lior Bondorevsky
Creation date:      26/11/2014
Last modofied date: 26/11/2014
Description:        Time Queue API
**************************************/
#ifndef __TIMER_QUEUE__
#define __TIMER_QUEUE__

#include <sys/time.h> /* struct timespec */

typedef struct TQ TQ;
typedef struct timespec TimeVal;
typedef long TimerId;
typedef int (*TimerCB) (void*);

extern TimeVal NO_TIMERS;

/* Return NULL on failure */
TQ* TQCreate();

void TQDestroy(TQ*);

/* Return -1 on failure */
TimerId TQScheduleTimer(TQ*, unsigned int mSec, TimerCB, void* param);

/* Cancel all timers in TimerQueue */
void TQCancelAllTimers(TQ*);

/* WARNING! Undefined behaviour for invalid TimerID */
void TQCancelTimer(TQ*, TimerId);

/* Return value can used directly with pselect
   if there is no timers NO_TIMERS (very large timeout) is returned 
   for elapsed timers TimeVal of 0 is returned */ 
TimeVal TQTimeTillTheNearestTime(TQ*);

/* Return the # of dispached Timers (callback function)
   and -1 on failure */
int TQDispatchExpiredTimers(TQ*);

#endif /* #ifndef __TIMER_QUEUE__ */
