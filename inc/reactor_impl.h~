/*******************************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 10/12/2014
Description:        Reactor Implementation API (Bridge)
*******************************************************/
#ifndef __REACTOR_IMPL__
#define __REACTOR_IMPL__

extern "C"
{
  #include "timer_queue.h" // TimerId
}

#include "uncopyable.h"   // UnCopyable
#include "reactor_mask.h" // Mask

namespace adv_cpp
{

class EventHandler;

class ReactorImplementation : private UnCopyable
{
public:   
  ReactorImplementation() {}
  virtual ~ReactorImplementation() {}
  
  virtual int RegisterIO(EventHandler*, Mask)   = 0;
  virtual int UnRegisterIO(EventHandler*, Mask) = 0;
  virtual TimerId SchedualTimer(EventHandler*, unsigned int mSec, void* arg) = 0;
  virtual int CancelTimer(TimerId) = 0;
  virtual int CancelAllTimers(EventHandler*)  = 0;
  virtual int RegisterSignal(EventHandler*, int signalNum) = 0;
  virtual int UnRegisterSignal(EventHandler*, int signalNum) = 0;
  virtual int HandleEvents() = 0;
};

} // namespace adv_cpp

#endif // ifndef __REACTOR_IMPL__
