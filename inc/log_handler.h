/*******************************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 14/12/2014
Description:        Log Handler API
*******************************************************/
#ifndef __LOG_HANDLER__
#define __LOG_HANDLER__

#include "event_handler.h" // Inheritance

namespace adv_cpp
{

class LogHandler : public EventHandler
{
public:
  LogHandler();
  virtual ~LogHandler();
  
  virtual int Open();
  virtual int HandleTimer(void* _arg);
  virtual int HandleInput(int fd);
  virtual int HandleClose(int fd);
  virtual int HandleSignal(int signal);
  
private:
  int m_id;
  FILE* m_logFile;
  static const size_t BUFFER_SIZE   = 256;
  static const size_t FILENAME_SIZE = 64;
  char m_recvBuffer[BUFFER_SIZE];
  char m_sendBuffer[BUFFER_SIZE];
  char m_logFilename[FILENAME_SIZE];
};


} // namespace adv_cpp

#endif // ifndef __LOG_HANDLER__
