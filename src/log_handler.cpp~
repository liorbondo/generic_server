/*******************************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 14/12/2014
Description:        Log Handler Implementation
*******************************************************/
#include <errno.h>  // errno
#include <iostream> // cerr
#include <unistd.h> // read
#include <cstring>  // memchr
#include <cstdlib>  // atoi
#include <cstdio>   // sprintf
#include <signal.h> // SIGINT

#include "log_handler.h"
#include "reactor_mask.h"  // Mask

#define SYS_ERR -1
#define OK       0

namespace adv_cpp
{

LogHandler::LogHandler() 
: m_id(-1),
  m_logFile(NULL) {}


LogHandler::~LogHandler() {}


int LogHandler::Open()
{
  if(SYS_ERR == this->GetReactor()->RegisterIO((EventHandler*)this, READ))
  {
    std::cerr << "RegisterIO failed" << std::endl;
    return SYS_ERR;
  }
  
  if(SYS_ERR == this->GetReactor()->RegisterSignal((EventHandler*)this, SIGINT))
  {
    std::cerr << "RegisterSignal failed" << std::endl;
    return SYS_ERR;
  }
  
  // Open server logFile for appending (common for all clients)
  sprintf(m_logFilename, "logs/socket_%d.log", this->GetFd());
  m_logFile = fopen(m_logFilename, "a");	
	if((!m_logFile || ferror(m_logFile)))
	{
		return SYS_ERR;
	}

  return OK;
}


int LogHandler::HandleSignal(int _signal)
{
  switch(_signal)
  {
    case SIGINT:
      if(SYS_ERR == this->HandleClose(this->GetFd()))
      {
       std::cerr << "HandleClose failed" << std::endl;
       return SYS_ERR;
      } 
    break; 
  }
  
  return OK;
}


int LogHandler::HandleTimer(void* _arg)
{
  std::cout << "client #" << m_id << " timeout elapsed" << std::endl; 
  // Write to logFile
  fprintf(m_logFile, "TIMEOUT: client #%d timeout elapsed\n", m_id);
  
  if(SYS_ERR == this->HandleClose(this->GetFd()))
  {
    std::cerr << "HandleClose failed" << std::endl;
    return SYS_ERR;
  }
  
  return OK;
}


int LogHandler::HandleInput(int _fd)
{
  int readBytes = read(_fd, m_recvBuffer, sizeof(m_recvBuffer));
        
  /* Client has closed connection or real error occured (not including non readable socket) */
  if((SYS_ERR == readBytes && EAGAIN != errno && EWOULDBLOCK != errno) || 0 == readBytes)
  {
    /* Cancel timeout timer for this client */
    GetReactor()->CancelTimer(GetTimer());
    
    std::cout << "client #" << m_id << " closed connection" << std::endl;
    
    if(SYS_ERR == this->HandleClose(_fd))
    {
      std::cerr << "HandleClose failed" << std::endl;
    }    
    
    return SYS_ERR;
  }
  
  // Read Succeeded
  std::cout << "server recieved: " << m_recvBuffer << std::endl;
  
  // Write to logFile
  fprintf(m_logFile, "RECV: %s\n", m_recvBuffer);
  
  /* Init response message */
  m_id = atoi((char*)(memchr(m_recvBuffer, '#', BUFFER_SIZE)) + 1);
  sprintf(m_sendBuffer, "welcome client #%d", m_id);
  int dataLen = strlen(m_sendBuffer) + 1;
  
  /* Send response */
  int bytesWritten = write(_fd, m_sendBuffer, dataLen);
  if(SYS_ERR == bytesWritten || bytesWritten != dataLen)
  {
    perror("write");
  }
  std::cout << "server sent: " << m_sendBuffer << std::endl;
  
  // Write to logFile
  fprintf(m_logFile, "SEND: %s\n", m_sendBuffer);
  
  return OK;
}


int LogHandler::HandleClose(int fd)
{
  // Close logFile
  fclose(m_logFile);
  
  // UnRegister from Signaling
  if(SYS_ERR == this->GetReactor()->UnRegisterSignal(this, SIGINT))
  {
    std::cerr << "RegisterSignal failed" << std::endl;
    return SYS_ERR;
  }
  
  // UnRegister from Reactor Read set
  if(SYS_ERR == this->GetReactor()->UnRegisterIO(this, READ))
  {
    std::cerr << "UnRegisterIO failed" << std::endl;
    return SYS_ERR;
  }
  
  // Cancel all timers
  if(SYS_ERR == this->GetReactor()->CancelAllTimers(this))
  {
    std::cerr << "CancelAllTimers failed" << std::endl;
    return SYS_ERR;
  }
  
  // Close socket
  if(SYS_ERR == close(this->GetFd()))
  {
    perror("close");
    return SYS_ERR;
  }
  
  this->Destroy();
  
  return OK;
}


} // namespace adv_cpp


