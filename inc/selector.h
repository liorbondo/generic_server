/*******************************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 14/12/2014
Description:        Select Reactor API
*******************************************************/
#ifndef __SELECTOR__
#define __SELECTOR__

#include <map> // std::map

#include "reactor_impl.h" // Inheritance
#include "singleton.h"    // friend class

extern "C"
{
  #include "timer_queue.h" // TimerId
}

namespace adv_cpp
{

class Selector : public ReactorImplementation
{

// Can be used only by Singleton<Selector>::Instance();
friend class Singleton<Selector>;

public:

  virtual int RegisterIO(EventHandler*, Mask);
  virtual int UnRegisterIO(EventHandler*, Mask);
  virtual TimerId SchedualTimer(EventHandler*, unsigned int mSec, void* arg);
  virtual int CancelTimer(TimerId);
  virtual int CancelAllTimers(EventHandler*);
  virtual int RegisterSignal(EventHandler*, int signalNum);
  virtual int UnRegisterSignal(EventHandler*, int signalNum);
  virtual int HandleEvents();
  
private:
  Selector();
  virtual ~Selector();
  
  fd_set m_readMasterSet;
  fd_set m_readCurrentSet;
  fd_set m_writeMasterSet;
  fd_set m_writeCurrentSet; 
  fd_set m_exceptMasterSet;
  fd_set m_exceptCurrentSet; 
  TQ*    m_timerQueue;
  std::map<int, EventHandler*> m_eventHandlerTable;
   
  static const size_t SLEEP_TIME = 10; // in uSec (HandleEvents delay)
  static int TimerCallbackAdapter(EventHandler*); 
  
  int SignalEstablishment(int signal);
  static void SignalHandler(int signal);
  
  static bool m_isRunning;
  static std::multimap<int, EventHandler*> m_signalTable;
  
  class SelectFunc
  {
  public:
    SelectFunc(fd_set&, fd_set&);    
    void operator()(std::pair<const int, EventHandler*>&);
    
  private:
    fd_set& m_readSet;
    fd_set& m_writeSet;
  };
};

} // namespace adv_cpp

#endif // ifndef __SELECTOR__
