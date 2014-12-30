/*******************************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 14/12/2014
Description:        Select Reactor Implementation 
*******************************************************/
#include <sys/select.h> // select, fd_set
#include <stdexcept>    // std::runtime_error
#include <functional>   // mem_fun 
#include <unistd.h>     // close, usleep
#include <signal.h>     // siaction
#include <cassert>      // assert
#include <cstdio>       // perror
#include <map>          // map
#include <functional>   // mem_fun
#include <iostream>     // cerr, endl
#include <algorithm>    // for_each

#include "selector.h" 
#include "event_handler.h"

extern "C"
{
  #include "timer_queue.h" 
}

#define OK       0
#define SYS_ERR -1

namespace adv_cpp
{

bool Selector::m_isRunning = true;

std::multimap<int,EventHandler*> Selector::m_signalTable;

Selector::Selector()
{
  // Clear all sets
  FD_ZERO(&m_readMasterSet);
  FD_ZERO(&m_readCurrentSet);
  FD_ZERO(&m_writeMasterSet);
  FD_ZERO(&m_writeCurrentSet);
  FD_ZERO(&m_exceptMasterSet);
  FD_ZERO(&m_exceptCurrentSet);
  
  // Create Server's TimerQueue
  m_timerQueue = TQCreate();
  if(!m_timerQueue)
  {
    throw std::runtime_error("TQCreate failed");
  }
  
  // Establish SIGINT
  if(SYS_ERR == SignalEstablishment(SIGINT))
  {
    throw std::runtime_error("SignalEstablishment failed");
  }
}


Selector::~Selector()
{
  // Destroy server TimerQueue
  TQDestroy(m_timerQueue);
}


void Selector::SignalHandler(int _signal)
{
  std::pair<std::multimap<int,EventHandler*>::iterator, std::multimap<int,EventHandler*>::iterator> signalRange;
  signalRange = m_signalTable.equal_range(_signal);
  
  std::multimap<int,EventHandler*>::iterator nextItr = signalRange.first;
  
  // Activate HandleSignal for all registered EventHandlers
  // Allows current iterator erase during HandleSignal
  while(nextItr != signalRange.second)
  {
    std::multimap<int,EventHandler*>::iterator currItr = nextItr;
    nextItr++;
    currItr->second->HandleSignal(_signal);
  }
  
  m_isRunning = false;
  
  std::cout << std::endl << std::endl;
  std::cout << "SERVER CLOSED !" << std::endl;
  std::cout << "bye bye..." << std::endl << std::endl; 
}


int Selector::SignalEstablishment(int _signal)
{
  struct sigaction sa;
	sa.sa_handler = SignalHandler;
	sa.sa_flags 	= 0;
	sigemptyset(&sa.sa_mask);

  if(SYS_ERR == sigaction(_signal, &sa, NULL))
	{
		perror("sigaction");
		return SYS_ERR;
	}

  return OK;
}


int Selector::RegisterIO(EventHandler* _eh, Mask _mask)
{
  assert(_eh);
  
  int socket = _eh->GetFd();
  
  switch(_mask)
  {
    case READ:
      FD_SET(socket, &m_readMasterSet);
    break;
    
    case WRITE:
      FD_SET(socket, &m_writeMasterSet);
    break;
    
    case OUT_OF_BAND:
      FD_SET(socket, &m_exceptMasterSet);
    break;
    
    default:
      return SYS_ERR;
  }
  
  // Add to table
  m_eventHandlerTable[socket] = _eh;
  
  return OK;
}


int Selector::UnRegisterIO(EventHandler* _eh, Mask _mask)
{
  assert(_eh);
  
  int socket = _eh->GetFd();
  
  switch(_mask)
  {
    case READ:
      FD_CLR(socket, &m_readMasterSet);
    break;
    
    case WRITE:
      FD_CLR(socket, &m_writeMasterSet);
    break;
    
    case OUT_OF_BAND:
      FD_CLR(socket, &m_exceptMasterSet);
    break;
    
    default:
      return SYS_ERR;
  }
  
  // Remove from table --> FIXME currently net removing if wanted erase only when ref count of masks is 0
  // m_eventHandlerTable.erase(socket); 
  
  return OK;
}


int Selector::TimerCallbackAdapter(EventHandler* _eh)
{
  return _eh->HandleTimer(NULL);
}


TimerId Selector::SchedualTimer(EventHandler* _eh, unsigned int _mSec, void* _arg)
{
  assert(_eh);
  
  return TQScheduleTimer(m_timerQueue, _mSec, (TimerCB)TimerCallbackAdapter, _eh);
}


int Selector::CancelTimer(TimerId _tid)
{
  assert(_tid != -1);
  
  // Cancel timeout timer by TimerId
  TQCancelTimer(m_timerQueue, _tid);
  
  return OK;
}


int Selector::CancelAllTimers(EventHandler* _eh)
{
  assert(_eh);
  
  // Cancel timeout timer by the EventHandler TimerId (for now only one timer per EH)
  TQCancelTimer(m_timerQueue, _eh->GetTimer());
  
  return OK;
}



int Selector::RegisterSignal(EventHandler* _eh, int _signal) 
{
  assert(_eh);
  
  m_signalTable.insert(std::pair<int, EventHandler*>(_signal, _eh));
  
  return OK;
}


int Selector::UnRegisterSignal(EventHandler* _eh, int _signal)
{
  assert(_eh);
  
  std::pair<std::multimap<int,EventHandler*>::iterator, std::multimap<int,EventHandler*>::iterator> signalRange;
  signalRange = m_signalTable.equal_range(_signal);
  
  // Remove the eventHandler from SignalTanble (@ _signal key) 
  for(std::multimap<int,EventHandler*>::iterator it = signalRange.first; it != signalRange.second; ++it)
  {
    if(it->second == _eh)
    {
      m_signalTable.erase(it); 
      break;
    }  
  }
  
  return OK;
}


inline Selector::SelectFunc::SelectFunc(fd_set& _readSet, fd_set& _writeSet)
: m_readSet(_readSet),
  m_writeSet(_writeSet)  {}


inline void Selector::SelectFunc::operator()(std::pair<const int, EventHandler*>& _pair)
{
  int socket = _pair.first;
  EventHandler* eventHandler = _pair.second;
  
  // If the socket is readable --> HandleInput
  if(FD_ISSET(socket, &m_readSet)) 
  {
    eventHandler->HandleInput(socket);
  }
  
  // If the socket is writable --> HandleOutput
  if(FD_ISSET(socket, &m_writeSet)) 
  {
    eventHandler->HandleOutput(socket);
  }
}


namespace Details
{
  typedef std::map<int, EventHandler*>::iterator EventHandlerTableItr;
  
  template <typename FUNC>
  static void ForEachSafe(EventHandlerTableItr _begin, EventHandlerTableItr _end, FUNC _doFunc)
  {
    // "Safe" for_each (for std::map) --> Allows current iterator erase during _doFunc 
    EventHandlerTableItr currItr;
    while(_begin != _end)
    {
      currItr = _begin;
      ++_begin;
      _doFunc(*currItr);
    }
  }
}


int Selector::HandleEvents()
{
  while(m_isRunning)
  {
    // Update all Sets
    m_readCurrentSet   = m_readMasterSet;
    m_writeCurrentSet  = m_writeMasterSet;
    m_exceptCurrentSet = m_exceptMasterSet;
      
    // if no timers selectTimeout = NO_TIMERS --> very large timeout
    TimeVal selectTimeout = TQTimeTillTheNearestTime(m_timerQueue);
    
    // Max socket --> last item of sorted map eventHandlerTable
    int maxSocket = m_eventHandlerTable.rbegin()->first;
    
    // Wait until some socket readable/writeable (selectTimeout is the nearest client timeout)
    int status = pselect(maxSocket + 1, &m_readCurrentSet, &m_writeCurrentSet, &m_exceptCurrentSet, &selectTimeout, NULL);
    if(SYS_ERR == status)
    {
      perror("pselect");
      continue;
    } 
    
    // Dispatch expired Clients callback function (release from server)
    TQDispatchExpiredTimers(m_timerQueue);
  
    Details::ForEachSafe(m_eventHandlerTable.begin(), m_eventHandlerTable.end(), SelectFunc(m_readCurrentSet, m_writeCurrentSet));
    
    // Delay
    usleep(SLEEP_TIME);  
  } // while (isRunning)
  
  return OK;
}


} // namespace adv_cpp

