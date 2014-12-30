/***************************************************
Author:             Lior Bondorevsky
Creation date:      11/12/2014
Last modofied date: 11/12/2014
Description:        Reactor Implementation (Bridge)
***************************************************/
#include <cassert> // assert

#include "reactor.h"  // Mask
#include "reactor_impl.h" // m_impl->

#define SYS_ERR -1
#define OK       0

namespace adv_cpp
{


Reactor::Reactor(ReactorImplementation* _impl)
: m_impl(_impl) 
{
  assert(_impl);
}


int Reactor::RegisterIO(EventHandler* _eh, Mask _mask)
{
  assert(_eh);
  
  return m_impl->RegisterIO(_eh, _mask);
}


int Reactor::UnRegisterIO(EventHandler* _eh, Mask _mask)
{
  assert(_eh);
  
  return m_impl->UnRegisterIO(_eh, _mask);
}


TimerId Reactor::SchedualTimer(EventHandler* _eh, unsigned int _mSec, void* _arg)
{
  assert(_eh);
  
  return m_impl->SchedualTimer(_eh, _mSec, _arg);
}


int Reactor::CancelTimer(TimerId _tid)
{
  return m_impl->CancelTimer(_tid);
}


int Reactor::CancelAllTimers(EventHandler* _eh)
{
  assert(_eh);
  
  return m_impl->CancelAllTimers(_eh);
}


int Reactor::RegisterSignal(EventHandler* _eh, int _signalNum)
{
  return m_impl->RegisterSignal(_eh, _signalNum);
}


int Reactor::UnRegisterSignal(EventHandler* _eh, int _signalNum)
{
  return m_impl->UnRegisterSignal(_eh, _signalNum);
}


int Reactor::HandleEvents()
{
  return m_impl->HandleEvents();
}

} // namespace adv_cpp
