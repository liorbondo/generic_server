/******************************************************
Author:             Lior Bondorevsky
Creation date:      22/12/2014
Last modofied date: 25/12/2014
Description:        FsService Handler Implementation
*******************************************************/
#include <errno.h>  // errno
#include <iostream> // cerr
#include <unistd.h> // read
#include <cstring>  // memset, strcpy
#include <cassert>  // assert
#include <cstdio>   // sprintf
#include <signal.h> // SIGINT

#include "service_handler.h"   // API
#include "reactor_mask.h"      // Mask
#include "fileserver_common.h" // MD5ChkSum, FileServerHeader

#define SYS_ERR -1
#define OK       0

namespace adv_cpp
{

// CLASS DEFINES
namespace
{
const int    EOF_STATUS         = 2;
const int    CONN_CLOSED_STATUS = 3;
const char*  FILES_PATH = "/home/lior/test/server";
} // anonymous namespace


FsServiceHandler::FsServiceHandler() 
: m_id(-1),
  m_currFile(NULL),
  m_currHashedFile(0),
  m_fileOffset(0),
  m_sendBufferOffset(0),
  m_currReadSize(0),
  m_isEof(false)
{
  memset(&m_serviceMsg, 0, sizeof(FSServiceMsg));
}


FsServiceHandler::~FsServiceHandler() {}


int FsServiceHandler::Open()
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


int FsServiceHandler::HandleSignal(int _signal)
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


int FsServiceHandler::HandleTimer(void* _arg)
{
  std::cout << "client #" << m_id << " timeout elapsed" << std::endl; 
  
  if(SYS_ERR == this->HandleClose(this->GetFd()))
  {
    std::cerr << "HandleClose failed" << std::endl;
    return SYS_ERR;
  }
  
  return OK;
}


inline int FsServiceHandler::SendHeader(const char* _hashedName, ssize_t _fileSize)
{
  FileServerHeader fileHeader; 
  memset(&fileHeader, 0, sizeof(fileHeader));
  
  // Init Header message: FileSize & MD5 checksum
  fileHeader.m_size = _fileSize;
  
  if(_hashedName)
  {
    strcpy(fileHeader.m_checkSum, MD5ChkSum(_hashedName).c_str());
  }
  
  // Send response
  int bytesWritten = write(GetFd(), &fileHeader, sizeof(fileHeader));
  if(SYS_ERR == bytesWritten || bytesWritten != sizeof(fileHeader))
  {
    perror("write");
    return SYS_ERR;
  }
  
  return OK;
}


inline int FsServiceHandler::HandleDownload()
{
  // Client identification sent with Download
  m_id = m_serviceMsg.m_id;
 
  // Open requested file for read
  char hashedName[128];
  sprintf(hashedName,  "%s/%lu", FILES_PATH, m_serviceMsg.m_hashedfileName);
  
	m_currFile = fopen(hashedName, "r");
	if(!m_currFile)
	{
		perror("fopen");
		SendHeader(NULL, SYS_ERR);
    return SYS_ERR;
	}
  
  if(SYS_ERR == SendHeader(hashedName, FileSize(m_currFile)))
  {
    return SYS_ERR;
  }
  std::cout << "file service: sent header" << std::endl;
  
  // Fill the client write buffer
  std::cout << "file service: sending requested file" << std::endl;
  int status = HandleOutput(GetFd());
  if(EOF_STATUS != status && SYS_ERR == this->GetReactor()->RegisterIO(this, WRITE))
  {
    std::cerr << "RegisterIO failed" << std::endl;
    return SYS_ERR;
  }  
  
  return OK;
}


inline int FsServiceHandler::HandleUpload()
{
  // Client identification sent with Upload
  m_id = m_serviceMsg.m_id;
 
  // Open requested file for read
  char hashedName[128];
  sprintf(hashedName,  "%s/%lu", FILES_PATH, m_serviceMsg.m_hashedfileName);
  
  FileServerHeader fileHeader; 
  memset(&fileHeader, 0, sizeof(fileHeader));
  
  // Recieve Header 
  if(SYS_ERR == read(GetFd(), &fileHeader, sizeof(FileServerHeader)))    
  {
    perror("read");
    return false;
  }
  
  ssize_t fileSize = WriteSocketToFile(hashedName, &fileHeader);
  if(SYS_ERR == fileSize)
  {
    std::cerr << "WriteSocketToFile failed" << std::endl;
    return SYS_ERR;
  }
  
  // Send header with uploded file size for client confirmation
  if(SYS_ERR == SendHeader(hashedName, fileSize))
  {
    std::cerr << "SendHeader failed" << std::endl;
    return SYS_ERR;
  }
  
  return OK;
}


inline int FsServiceHandler::ReadSocketToBuffer()
{
  int fd = GetFd();
  int readBytes = read(fd, &m_serviceMsg, sizeof(FSServiceMsg));
        
  // Client has closed connection or real error occured
  if((SYS_ERR == readBytes && EAGAIN != errno && EWOULDBLOCK != errno) || 0 == readBytes)
  {
    std::cout << "file service: client #" << m_id << " closed connection" << std::endl;
    
    if(SYS_ERR == this->HandleClose(fd))
    {
      std::cerr << "HandleClose failed" << std::endl;
    }    
    
    return SYS_ERR;
  }
  
  return OK;
}


int FsServiceHandler::HandleInput(int _fd)
{
  if(SYS_ERR == ReadSocketToBuffer())
  {
    return SYS_ERR;
  }
  
  // Read Succeeded. Parsing incoming msg and handling by type
  std::cout << "file service server recieved: ";

  switch(m_serviceMsg.m_type)
  {
    case FSServiceMsg::DOWNLOAD:
      std::cout << "DOWNLOAD" << std::endl;
    return HandleDownload();
    
    case FSServiceMsg::UPLOAD:
      std::cout << "UPLOAD" << std::endl;
    return HandleUpload();
    
    default:
      std::cerr << "Unknown message recieved" << std::endl;
    return SYS_ERR;  
  }
  
  return OK;
}


int FsServiceHandler::HandleOutput(int _fd)
{
  // Send file to client socket 
  int status = WriteFileToSocket();
  if(EOF_STATUS == status)
  {
    // File send complete 
    std::cout << "file send complete" << std::endl;
    // UnRegister from Reactor Write set
    if(SYS_ERR == this->GetReactor()->UnRegisterIO(this, WRITE))
    {
      std::cerr << "UnRegisterIO failed" << std::endl;
      return SYS_ERR;
    }
    
    return EOF_STATUS;
  } 
  else if(CONN_CLOSED_STATUS == status)
  {
    std::cout << "client #" << m_id << " closed connection" << std::endl;

    if(SYS_ERR == this->HandleClose(GetFd()))
    {
      std::cerr << "HandleClose failed" << std::endl;
    }   
  }  
  else if(SYS_ERR == status)
  {
    std::cerr << "WriteFileToSocket Failed" << std::endl;
  }
  
  return OK;
}


int FsServiceHandler::HandleClose(int fd)
{
  // Close filesList
  if(m_currFile && fclose(m_currFile))
  {
    perror("fclose");
    return SYS_ERR;
  }

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
  
  // UnRegister from Reactor Write set
  if(SYS_ERR == this->GetReactor()->UnRegisterIO(this, WRITE))
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


int FsServiceHandler::WriteFileToSocket()
{
  // Reading file to the application buffer if needed 
  if(!m_sendBufferOffset)
  {
    if(fseek(m_currFile, m_fileOffset, SEEK_SET))
    {
      perror("fseek");
      return SYS_ERR;
    } 
    
    ssize_t bytesRead = fread(m_sendBuffer, sizeof(char), BUFFER_SIZE, m_currFile);
    m_currReadSize = bytesRead;
    
    if(bytesRead != BUFFER_SIZE)
    {
      if(feof(m_currFile))
      {
        m_isEof = true;
      }
      else if(ferror(m_currFile))
      {
        perror("fread");
        return SYS_ERR;
      }
    }
    
    m_fileOffset += bytesRead;
  }
  
  // Write bytes from application buffer to socket
  int bytesWritten = write(GetFd(), m_sendBuffer + m_sendBufferOffset, m_currReadSize - m_sendBufferOffset);
  if(0 == bytesWritten || (SYS_ERR == bytesWritten && EAGAIN != errno && EWOULDBLOCK != errno))
  {
    return CONN_CLOSED_STATUS;
  }
  
  m_sendBufferOffset += bytesWritten;
  
  if(m_sendBufferOffset == m_currReadSize)
  {
    m_sendBufferOffset = 0;
    
    if(m_isEof)
    {
      // reset clientHandler 
      m_fileOffset = 0;
      m_isEof = false;
      
      return EOF_STATUS;
    } 
  }
  
  return OK;
}


namespace Details
{

inline int SocketRead(int _fd, void* _buffer, size_t _buffSize)
{
  int currBytesRead = read(_fd, _buffer, _buffSize);
  if(SYS_ERR == currBytesRead)    
  {
    perror("read");
    return SYS_ERR;
  }
  if(currBytesRead == 0)
  {
    std::cerr << "error: connection closed" << std::endl;
    return SYS_ERR;
  }
  
  return currBytesRead;
}

} // namespace Details


ssize_t FsServiceHandler::WriteSocketToFile(const char* _fileName, const FileServerHeader* _header)
{
  assert(_fileName);
  assert(_header);
  
  static const size_t BUFFER_SIZE = 65536;
  char buffer[BUFFER_SIZE] = {0};
  ssize_t totBytesWritten  = 0;
  
  // Open filesList file
  FILE* pFile = fopen(_fileName, "wb");
	if(!pFile)
	{
		perror("fopen");
    return false;
	}
  
  // Breaks when fileSent or on read error
  while(true)
  {
    // Recieve file chunk 
    int currBytesRead = Details::SocketRead(GetFd(), buffer, sizeof(buffer));
    if(SYS_ERR == currBytesRead)
    {
      std::cerr << "error: SocketRead failed" << std::endl;
      return SYS_ERR;
    }
    
    int currBytesWritten = fwrite(buffer, sizeof(char), currBytesRead, pFile);
    if(SYS_ERR == currBytesWritten)    
    {
      perror("write");
      return SYS_ERR;
    }
    if(currBytesWritten == 0)
    {
      std::cerr << "error: connection closed" << std::endl;
      return SYS_ERR;
    }
 
    totBytesWritten += currBytesWritten; 
    if(totBytesWritten == _header->m_size)
    {
      break;
    }
  }
  
  // Save the file size before closing
  ssize_t fileSize = FileSize(pFile);
  if(SYS_ERR == fileSize)
  {
    std::cerr << "error: FileSize Failed" << std::endl;
    return SYS_ERR;
  }
  
  // Close file 
  if(fclose(pFile))
  {
  	perror("fclose");
    return SYS_ERR;
  }
  
  if(MD5ChkSum(_fileName) != _header->m_checkSum)
  {
    std::cerr << "error: File MD5 Checksum Failed" << std::endl;
    return SYS_ERR;
  }
  
  return fileSize;
}


} // namespace adv_cpp


