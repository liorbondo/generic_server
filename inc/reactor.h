/***************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 10/12/2014
Description:        Reactor API (Bridge)
****************************************/
#ifndef __REACTOR__
#define __REACTOR__

#include "uncopyable.h"   // UnCopyable
#include "reactor_mask.h" // Mask
#include "timer_queue.h"  // TimerId


namespace adv_cpp
{

class ReactorImplementation;
class EventHandler;

class Reactor : private UnCopyable
{
public:
  explicit Reactor(ReactorImplementation*);
  int RegisterIO(EventHandler*, Mask);
  int UnRegisterIO(EventHandler*, Mask);
  TimerId SchedualTimer(EventHandler*, unsigned int mSec, void* arg);
  int CancelTimer(TimerId);
  int CancelAllTimers(EventHandler*);
  int RegisterSignal(EventHandler*, int signalNum);
  int UnRegisterSignal(EventHandler* _eh, int signalNum);
  int HandleEvents();

private:
  ReactorImplementation* m_impl;  
};

} // namespace adv_cpp

#endif // ifndef __REACTOR__
