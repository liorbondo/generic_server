/***************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 10/12/2014
Description:        Event Handler API 
****************************************/
#ifndef __EVENT_HANDLER__
#define __EVENT_HANDLER__

#include "uncopyable.h" // UnCopyable
#include "reactor.h" // Mask
#include "timer_queue.h" // TimerId

namespace adv_cpp
{

class EventHandler : private UnCopyable
{
public:
  virtual int Init()  { return 0; }
  virtual int Open()  { return 0; }
  virtual int Close() { return 0; }

  virtual int HandleInput(int fd)      { return -1; }
  virtual int HandleOutput(int fd)     { return -1; }
  virtual int HandleTimer(void* arg)   { return -1; }
  virtual int HandleSignal(int signal) { return -1; }
  
  virtual int HandleClose() { return -1; }
  virtual void Destroy()    { delete this; }
  
  void SetFd(int _fd) { m_fd = _fd; }
  int  GetFd() const  { return m_fd; }
  
  TimerId GetTimer() const { return m_timer; }
  void SetTimer(TimerId _tid) { m_timer = _tid;}
  
  void SetReactor(Reactor* _r) { m_reactor = _r; }
  Reactor* GetReactor() { return m_reactor; }
  
protected:
  EventHandler() : m_fd(-1), m_reactor(NULL), m_timer(-1) {}
  virtual ~EventHandler() {}

private:
  int      m_fd;
  Reactor* m_reactor;
  TimerId  m_timer;  
};

} // namespace adv_cpp

#endif // ifndef __EVENT_HANDLER__
