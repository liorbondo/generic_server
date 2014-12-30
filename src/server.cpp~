/*******************************************************
Author:             Lior Bondorevsky
Creation date:      11/12/2014
Last modofied date: 24/12/2014
Description:        Select Reactor Implementation 
*******************************************************/
#include <cstdlib> // system

#include "acceptor.h"
#include "log_handler.h"
#include "repo_handler.h"
#include "service_handler.h"
#include "reactor.h"
#include "singleton.h"
#include "selector.h"


#define SYS_ERR        -1
#define OK              0
#define ERR             1

#define LOGGER_PORT     5060
#define REPO_PORT       5061
#define SERVICE_PORT1   5062
#define SERVICE_PORT2   5063

#define RETURN_ON_ERR(FUNC)  if(SYS_ERR == (FUNC)) { return ERR; }                   
  
using namespace adv_cpp;

typedef Acceptor<LogHandler>       LoggingAcceptor;
typedef Acceptor<FsRepoHandler>    FsRepoAcceptor;
typedef Acceptor<FsServiceHandler> FsServiceAcceptor;


int main()
{
  // Singleton destroyed automaticly @ exit
  Selector& selector = Singleton<Selector>::Instance();
  Reactor*  reactor  = new Reactor(&selector);
  
  LoggingAcceptor* logAcc         = new LoggingAcceptor();
  FsRepoAcceptor*  repoAcc        = new FsRepoAcceptor();
  FsServiceAcceptor* serviceAcc1  = new FsServiceAcceptor();
  FsServiceAcceptor* serviceAcc2  = new FsServiceAcceptor();
  
  // LOGGING APP
  // NULL -> Any connection interface
  RETURN_ON_ERR(logAcc->Init(NULL, LOGGER_PORT, reactor));
  RETURN_ON_ERR(logAcc->Open());
  
  // FILE SERVER REPOSITORY APP
  // NULL -> Any connection interface
  RETURN_ON_ERR(repoAcc->Init(NULL, REPO_PORT, reactor));
  RETURN_ON_ERR(repoAcc->Open());
  
  // FILE SERVER SERVICE
  // NULL -> Any connection interface
  RETURN_ON_ERR(serviceAcc1->Init(NULL, SERVICE_PORT1, reactor));
  RETURN_ON_ERR(serviceAcc1->Open());
  
  // FILE SERVER SERVICE
  // NULL -> Any connection interface
  RETURN_ON_ERR(serviceAcc2->Init(NULL, SERVICE_PORT2, reactor));
  RETURN_ON_ERR(serviceAcc2->Open());
  
  system("clear");
  std::cout << "Server is up..." << std::endl;
  
  RETURN_ON_ERR(reactor->HandleEvents());
  
  // Delete all allocations
  delete reactor;
  delete logAcc;
  delete repoAcc;
  delete serviceAcc1;
  delete serviceAcc2;

  return OK;
}
