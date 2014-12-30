/******************************************************
Author:             Lior Bondorevsky
Creation date:      17/12/2014
Last modofied date: 25/12/2014
Description:        FsRepo Handler Implementation
*******************************************************/
#include <errno.h>  // errno
#include <iostream> // cerr
#include <unistd.h> // read
#include <cassert>  // assert
#include <cstring>  // memset, strcpy
#include <cstdio>   // sprintf
#include <signal.h> // SIGINT
#include <map>      // map

#include "repo_handler.h"      // API
#include "reactor_mask.h"      // Mask
#include "fileserver_common.h" // MD5ChkSum, FileServerHeader, HashFileName
#include "file_operations.h"   // IsFileExist

#define SYS_ERR -1
#define OK       0

namespace adv_cpp
{

// CLASS DEFINES
namespace
{
const char*   FILES_LIST_PATH    = "/home/lior/test/server/repo_files";
const int     EOF_STATUS         = 2;
const int     CONN_CLOSED_STATUS = 3;
const char*   UPLOAD_IP1         = "127.0.0.1";
const char*   UPLOAD_IP2         = "127.0.0.1";
const short   UPLOAD_PORT1       = 5062; 
const short   UPLOAD_PORT2       = 5063; 
} // anonymous namespace


// Static objects init
std::map<unsigned long, std::pair<IpPort, IpPort> > FsRepoHandler::m_downloadServers;
std::pair<IpPort, IpPort> FsRepoHandler::m_uploadServers(IpPort(UPLOAD_IP1, UPLOAD_PORT1), IpPort(UPLOAD_IP2, UPLOAD_PORT2));


FsRepoHandler::FsRepoHandler() 
: m_id(-1),
  m_filesList(NULL),
  m_fileOffset(0),
  m_sendBufferOffset(0),
  m_currReadSize(0),
  m_isEof(false)
{
  memset(&m_repoMsg, 0, sizeof(FSRepoMsg));
}


FsRepoHandler::~FsRepoHandler() {}


int FsRepoHandler::Open()
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

  // Open server filesList file
  bool isExists = IsFileExist(FILES_LIST_PATH);
  
	m_filesList = fopen(FILES_LIST_PATH, "a+b");
	if(!m_filesList)
	{
		perror("fopen");
    return SYS_ERR;
	}
	
	if(!isExists)
	{
	  fprintf(m_filesList, "** FILES_LIST **\n\n");
	}

  return OK;
}


int FsRepoHandler::HandleSignal(int _signal)
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


int FsRepoHandler::HandleTimer(void* _arg)
{
  std::cout << "client #" << m_id << " timeout elapsed" << std::endl; 
  
  if(SYS_ERR == this->HandleClose(this->GetFd()))
  {
    std::cerr << "HandleClose failed" << std::endl;
    return SYS_ERR;
  }
  
  return OK;
}


namespace Details
{

int FileAppend(FILE* _fp, const char* _str)
{
  // Assumption: file is already open for writting
  assert(_fp);
  
  // Seek file to end for updating
  if(fseek(_fp, 0, SEEK_END))
	{
    perror("fseek");
    return SYS_ERR;
  }
  
  fprintf(_fp, "(+) %s\n", _str);
  
  return OK; 
}

} // namespace Details


inline int FsRepoHandler::SendHeader()
{
  FileServerHeader fileHeader; 
  memset(&fileHeader, 0, sizeof(fileHeader));
  
  // Init Header message: FileSize & MD5 checksum
  fileHeader.m_size = FileSize(m_filesList);
  strcpy(fileHeader.m_checkSum, MD5ChkSum(FILES_LIST_PATH).c_str());

  // Send response
  int bytesWritten = write(GetFd(), &fileHeader, sizeof(fileHeader));
  if(SYS_ERR == bytesWritten || bytesWritten != sizeof(fileHeader))
  {
    perror("write");
    return SYS_ERR;
  }
  
  return OK;
}


inline int FsRepoHandler::HandleDiscovery()
{
  // Client identification sent with Discovery
  m_id = m_repoMsg.m_id;
  
  if(SYS_ERR == SendHeader())
  {
    return SYS_ERR;
  }
  std::cout << "repository: sent header" << std::endl;
  
  // Seek file to beggining
  if(fseek(m_filesList, 0, SEEK_SET))
	{
    perror("fseek");
    return SYS_ERR;
  }
  
  // Fill the client write buffer
  std::cout << "repository: sending files list" << std::endl;
  int status = HandleOutput(GetFd());
  if(EOF_STATUS != status && SYS_ERR == this->GetReactor()->RegisterIO(this, WRITE))
  {
    std::cerr << "RegisterIO failed" << std::endl;
    return SYS_ERR;
  }  
  
  return OK;
}


inline int FsRepoHandler::HandleGSFD()
{
  std::cout << "searching servers..." << std::endl;
  
  std::map<unsigned long, std::pair<IpPort, IpPort> >::iterator itr;
  itr = m_downloadServers.find(m_repoMsg.m_hashedfileName);
  
  // if file not found in map return -1 at port
  memset(&m_repoMsg, 0, sizeof(FSRepoMsg));
  if(itr == m_downloadServers.end())
  {
    m_repoMsg.m_ipp1.SetPort(SYS_ERR);
    m_repoMsg.m_ipp2.SetPort(SYS_ERR);
    std::cerr << "file not found in servers map" << std::endl;
  }
  else
  {
    m_repoMsg.m_ipp1 = itr->second.first;
    m_repoMsg.m_ipp2 = itr->second.second;
  }
  
  // Write bytes from application buffer to socket
  int bytesWritten = write(GetFd(), &m_repoMsg, sizeof(FSRepoMsg));
  if(0 == bytesWritten || (SYS_ERR == bytesWritten && EAGAIN != errno && EWOULDBLOCK != errno))
  {
    std::cerr << "write failed - connection closed" << std::endl;
    return CONN_CLOSED_STATUS;
  }
  
  return OK;
}


inline int FsRepoHandler::HandleGSFU()
{
  // If file not found in map return -1 at port
  memset(&m_repoMsg, 0, sizeof(FSRepoMsg));
  m_repoMsg.m_ipp1 = m_uploadServers.first;
  m_repoMsg.m_ipp2 = m_uploadServers.second;
  
  // Write bytes from application buffer to socket
  int bytesWritten = write(GetFd(), &m_repoMsg, sizeof(FSRepoMsg));
  if(0 == bytesWritten || (SYS_ERR == bytesWritten && EAGAIN != errno && EWOULDBLOCK != errno))
  {
    std::cerr << "write failed - connection closed" << std::endl;
    return CONN_CLOSED_STATUS;
  }
  
  return OK;
}


inline int FsRepoHandler::HandleUpdateFilesList()
{
  if(SYS_ERR == Details::FileAppend(m_filesList, m_repoMsg.m_fileName))
  {
    std::cerr << "FileAppend failed" << std::endl;
    return SYS_ERR;
  }
  
  // Add to servers List
  unsigned long hasedFilename = HashFileName(m_repoMsg.m_fileName);
  std::pair<IpPort, IpPort> ippPair(m_repoMsg.m_ipp1, m_repoMsg.m_ipp2);
  m_downloadServers.insert(std::pair<unsigned long, std::pair<IpPort, IpPort> > (hasedFilename, ippPair));
  
  return OK;
}


inline int FsRepoHandler::ReadSocketToBuffer()
{
  int fd = GetFd();
  int readBytes = read(fd, &m_repoMsg, sizeof(FSRepoMsg));
        
  // Client has closed connection or real error occured 
  if((SYS_ERR == readBytes && EAGAIN != errno && EWOULDBLOCK != errno) || 0 == readBytes)
  {
    std::cout << "repository: client #" << m_id << " closed connection" << std::endl;
    
    if(SYS_ERR == this->HandleClose(fd))
    {
      std::cerr << "HandleClose failed" << std::endl;
    }    
    
    return SYS_ERR;
  }
  
  return OK;
}


int FsRepoHandler::HandleInput(int _fd)
{
  if(SYS_ERR == ReadSocketToBuffer())
  {
    return SYS_ERR;
  }
  
  // Read Succeeded. Parsing incoming msg and handling by type
  std::cout << "repository server recieved: ";

  switch(m_repoMsg.m_type)
  {
    case FSRepoMsg::DISCOVERY:
      std::cout << "DISCOVERY" << std::endl;
      return HandleDiscovery();
      
    case FSRepoMsg::GET_SERV_DOWNLOAD:
      std::cout << "GET_SERV_DOWNLOAD" << std::endl;
      return HandleGSFD();
      
    case FSRepoMsg::GET_SERV_UPLOAD:
      std::cout << "GET_SERV_UPLOAD" << std::endl;
      return HandleGSFU();
    
    case FSRepoMsg::UPDATE_FILES_LIST:
      std::cout << "UPDATE_FILES_LIST" << std::endl;
      return HandleUpdateFilesList();
    
    default:
      std::cerr << "Unknown msg recieved" << std::endl;
    return SYS_ERR;  
  }
  
  return OK;
}


int FsRepoHandler::HandleOutput(int _fd)
{
  // Note: output is only file list sending for DISCOVERY
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


int FsRepoHandler::HandleClose(int fd)
{
  // Close filesList
  if(m_filesList && fclose(m_filesList))
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


int FsRepoHandler::WriteFileToSocket()
{
  // Reading file to the application buffer if needed 
  if(!m_sendBufferOffset)
  {
    if(fseek(m_filesList, m_fileOffset, SEEK_SET))
    {
      perror("fseek");
      return SYS_ERR;
    } 
    
    ssize_t bytesRead = fread(m_sendBuffer, sizeof(char), BUFFER_SIZE, m_filesList);
    m_currReadSize = bytesRead;
    
    if(bytesRead != BUFFER_SIZE)
    {
      if(feof(m_filesList))
      {
        m_isEof = true;
      }
      else if(ferror(m_filesList))
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


} // namespace adv_cpp


