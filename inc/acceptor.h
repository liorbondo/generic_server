/*******************************************************
Author:             Lior Bondorevsky
Creation date:      10/12/2014
Last modofied date: 14/12/2014
Description:        Acceptor API & Implementation
*******************************************************/
#ifndef __ACCEPTOR__
#define __ACCEPTOR__

#include <list>           // std::list
#include <new>            // std::nothrow
#include <sys/socket.h>   // socket
#include <netinet/in.h>   // sockaddr_in 
#include <arpa/inet.h>    // inet_pton
#include <sys/fcntl.h>    // fnctl
#include <errno.h>        // errno
#include <cstring>        // memset
#include <cstdio>         // perror
#include <iostream>       // cerr, endl
#include <unistd.h>       // close
#include <sys/resource.h> // setrlimit
#include <signal.h>       // SIGINT

#include "event_handler.h"


#define OK       0
#define SYS_ERR -1

namespace adv_cpp
{

template <typename EH_APPL>
class Acceptor : public EventHandler
{
public:
  Acceptor();
  virtual ~Acceptor();
  
  int Init(const char* ip, unsigned short port, Reactor*);
  virtual int Open();
  virtual int HandleInput(int fd);
  virtual int HandleClose(int fd);
  virtual int HandleSignal(int signal);
  
private:
  int SetNonBlocking(int fd);
  static const int CLIENT_TIMEOUT   = 240000; // in mSec
  static const int LISTEN_BACKLOG   = 100;    // maximum # of pending connections
  static const int MAX_OPEN_CLIENTS = 4000;   // total opened file descriptors for the process
};


template <typename EH_APPL>
Acceptor<EH_APPL>::Acceptor() {}


template <typename EH_APPL>
Acceptor<EH_APPL>::~Acceptor() {}


template <typename EH_APPL>
int Acceptor<EH_APPL>::SetNonBlocking(int _fd)
{
  int flags = fcntl(_fd, F_GETFL, 0);
  if(SYS_ERR == flags)
	{
		perror("fcntl");
		return SYS_ERR;
	}
	
	if(SYS_ERR == fcntl(_fd, F_SETFL, flags | O_NONBLOCK))
	{
    perror("fcntl");
    return SYS_ERR;
  }
  
  return OK;
}


template <typename EH_APPL>
int Acceptor<EH_APPL>::Init(const char* _ip, unsigned short _port, Reactor* _reactor)
{  
  // listeningSocket Creation
  int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(SYS_ERR == listeningSocket)
  {
    perror("socket");
    return SYS_ERR;
  }
  
  // Set to non-blocking fd
  if(SYS_ERR == SetNonBlocking(listeningSocket)) 
  {
    perror("setNonBlocking");
    return SYS_ERR;
  }
  
  // Set open files limit
  struct rlimit rl;
  rl.rlim_cur = MAX_OPEN_CLIENTS;
  rl.rlim_max = MAX_OPEN_CLIENTS;

  if(SYS_ERR == setrlimit(RLIMIT_NOFILE, &rl))
  {
    perror("setrlimit");
    return errno;
  }

  // Init serverAddr
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port   = htons(_port);
  
  if(!_ip)
  {
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else
  {  
    inet_pton(AF_INET, _ip, &serverAddr.sin_addr);
  }  
  
  // Binding server address (setsockopt prevents "bind: Address already in use")
  int on = 1;
  setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if(SYS_ERR == bind(listeningSocket, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)))
  {
    perror("bind");
    return SYS_ERR;
  }
  
  // Register listeningSocket as passive socket
  if(SYS_ERR == listen(listeningSocket, LISTEN_BACKLOG))
  {
    perror("listen");
    return SYS_ERR;
  }
  
  // Set listeningSocket as Acceptor object file descriptor, and set Reactor 
  this->SetFd(listeningSocket);
  this->SetReactor(_reactor);
  
  return OK;
}


template <typename EH_APPL>
int Acceptor<EH_APPL>::Open()
{
  if(SYS_ERR == this->GetReactor()->RegisterIO(this, READ))
  {
    std::cerr << "RegisterIO failed" << std::endl;
    return SYS_ERR;
  }
  
  if(SYS_ERR == this->GetReactor()->RegisterSignal(this, SIGINT))
  {
    std::cerr << "RegisterSignal failed" << std::endl;
    return SYS_ERR;
  }
  
  return OK;
}


template <typename EH_APPL>
int Acceptor<EH_APPL>::HandleInput(int _fd)
{
  int clientSocket = accept(_fd, NULL, NULL);
  if(SYS_ERR == clientSocket)
  {
    // Non blocking error or interrupted
    if(EINTR == errno || EAGAIN == errno || EWOULDBLOCK == errno)
    {
      return OK; 
    }

    perror("accept");
    return SYS_ERR;     
  }
  
  EH_APPL* appEH = new (std::nothrow) EH_APPL;
  if(!appEH)
  {
    std::cerr << "EH_APPL Allocation failed" << std::endl;
    return SYS_ERR;
  }
  
  // Check that appEH is a type of EventHandler (won't compile otherwise)
  EventHandler* eh = static_cast<EventHandler*>(appEH);
  
  eh->SetFd(clientSocket);
  eh->SetReactor(this->GetReactor());
  
  // Do app specific activation --> in our app's register EH with reactor for read
  if(SYS_ERR == eh->Open())
  {
    std::cerr << "EH_APPL Open failed" << std::endl;
    return SYS_ERR;
  }
  
  // SchedualTimer for new socket
  TimerId tid = this->GetReactor()->SchedualTimer(eh, CLIENT_TIMEOUT, NULL);
  if(SYS_ERR == tid)
  {
    std::cerr << "SchedualTimer failed" << std::endl;
    return SYS_ERR;
  }
  eh->SetTimer(tid);
  
  return OK;
}


template <typename EH_APPL>
int Acceptor<EH_APPL>::HandleSignal(int _signal)
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


template <typename EH_APPL>
int Acceptor<EH_APPL>::HandleClose(int fd)
{
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
  
  // Close socket
  if(SYS_ERR == close(this->GetFd()))
  {
    perror("close");
    return SYS_ERR;
  }
  
  return OK;
}

} // namespace adv_cpp

#endif // ifndef __ACCEPTOR__
