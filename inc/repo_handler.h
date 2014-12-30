/*******************************************************
Author:             Lior Bondorevsky
Creation date:      17/12/2014
Last modofied date: 25/12/2014
Description:        FileServer Repo Handler API
*******************************************************/
#ifndef __FS_REPO_HANDLER__
#define __FS_REPO_HANDLER__

#include <map> // map

#include "event_handler.h"     // Inheritance
#include "fileserver_common.h" // IpPort, FSRepoMsg 

namespace adv_cpp
{

class FsRepoHandler : public EventHandler
{
public:
  FsRepoHandler();
  virtual ~FsRepoHandler();
  
  virtual int Open();
  virtual int HandleTimer(void* _arg);
  virtual int HandleInput(int fd);
  virtual int HandleOutput(int fd);
  virtual int HandleClose(int fd);
  virtual int HandleSignal(int signal);
  
private:
  int       m_id;
  FILE*     m_filesList;
  size_t    m_fileOffset;
  size_t    m_sendBufferOffset;
  size_t    m_currReadSize;
  bool      m_isEof;
  FSRepoMsg m_repoMsg;
  
  static const ssize_t BUFFER_SIZE = 65536;
  char m_sendBuffer[BUFFER_SIZE];
  
  static std::map<unsigned long, std::pair<IpPort, IpPort> > m_downloadServers;
  static std::pair<IpPort, IpPort> m_uploadServers;
  
  int WriteFileToSocket();
  int ReadSocketToBuffer();
  int SendHeader();
  int HandleDiscovery();
  int HandleGSFD();
  int HandleGSFU();
  int HandleUpdateFilesList();
};


} // namespace adv_cpp

#endif // ifndef __FS_REPO_HANDLER__
