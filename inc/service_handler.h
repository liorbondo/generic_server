/*******************************************************
Author:             Lior Bondorevsky
Creation date:      22/12/2014
Last modofied date: 25/12/2014
Description:        FileServer Service Handler API
*******************************************************/
#ifndef __FS_SERVICE_HANDLER__
#define __FS_SERVICE_HANDLER__

#include "event_handler.h"     // Inheritance
#include "fileserver_common.h" // FSServiceMsg

namespace adv_cpp
{

class FsServiceHandler : public EventHandler
{
public:
  FsServiceHandler();
  virtual ~FsServiceHandler();
  
  virtual int Open();
  virtual int HandleTimer(void* _arg);
  virtual int HandleInput(int fd);
  virtual int HandleOutput(int fd);
  virtual int HandleClose(int fd);
  virtual int HandleSignal(int signal);
  
private:
  int           m_id;
  FILE*         m_currFile;
  unsigned long m_currHashedFile;
  size_t        m_fileOffset;
  size_t        m_sendBufferOffset;
  size_t        m_currReadSize;
  bool          m_isEof;
  FSServiceMsg  m_serviceMsg;
  
  static const ssize_t BUFFER_SIZE = 65536;
  char m_sendBuffer[BUFFER_SIZE];

  int     WriteFileToSocket();
  ssize_t WriteSocketToFile(const char*, const FileServerHeader*);
  int ReadSocketToBuffer();
  int SendHeader(const char*, ssize_t);
  int SendErrHeader();
  int HandleDownload();
  int HandleUpload();
};


} // namespace adv_cpp

#endif // ifndef __FS_SERVICE_HANDLER__
