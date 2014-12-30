/********************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			17/12/2014
Last modofied date: 24/12/2014
Description: 				FileServer Protocols implementation 
********************************************************/
#include <cstdio>   // sprintf, fwrite
#include <unistd.h> // read, write
#include <cstring>  // memset
#include <cassert>  // assert
#include <string>   // string
#include <cstdlib>  // system
#include <sys/socket.h> // socket 
#include <netinet/in.h> // sockaddr_in 
#include <arpa/inet.h>  // inet_pton 
#include <stdexcept>    // std::runtime_error
#include <iostream>     // cerr

#include "fileserver_protocolos.h" // Class Headers
#include "fileserver_common.h"     // FileServerHeader, MD5ChkSum, IpPort
#include "file_operations.h"       // DivideFile, RemoveFile

#define SYS_ERR -1

#define PRINT_MSG(MSG)  (cout << "(*) " << (MSG) << endl << endl)
#define PRINT_ERR(ERR)  (cerr << "(!) " << (ERR) << endl << endl)

using std::cerr;
using std::cout;
using std::endl;
using std::string;

namespace adv_cpp
{

// Inner Implementation

namespace Details
{

template <typename MSG>
inline bool SendMsg(int _fd, const MSG& _msg)
{
  int bytesWritten = write(_fd, &_msg, sizeof(MSG));
  if(SYS_ERR == bytesWritten || bytesWritten != sizeof(MSG))
  {
    perror("write");
    return false;
  }
  
  return true;
}


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
    cerr << "error: connection closed" << endl;
    return SYS_ERR;
  }
  
  return currBytesRead;
}

} // namespace Details


// XXX FileServerProtocol Implementation XXX

FileServerProtocol::FileServerProtocol(const char* _ip, short _port, int _id)
: m_fd(-1),
  m_id(_id),
  m_recvHeader(new FileServerHeader) 
{
  memset(m_recvHeader, 0, sizeof(FileServerHeader));
  
  // Init serverAddr 
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port   = htons(_port);
  inet_pton(AF_INET, _ip, &serverAddr.sin_addr); 

  // Init client socket 
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(SYS_ERR == m_fd)
  {
    perror("socket");
    throw std::runtime_error("socket failed");
  }
    
  // Connect to server 
  if(SYS_ERR == connect(m_fd, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)))
  {
    perror("connect");
    throw std::runtime_error("connect failed");
  }
}
  
  
FileServerProtocol::~FileServerProtocol()
{
  // Close client socket
  if(SYS_ERR == close(m_fd))
  {
    perror("close");
  }
  
  if(m_recvHeader)
  {
    delete m_recvHeader;
  }
  
  m_recvHeader = NULL;
}  


inline bool FileServerProtocol::SendHeader(const char* _fileName, ssize_t _fileSize) const
{
  FileServerHeader fileHeader; 
  memset(&fileHeader, 0, sizeof(fileHeader));
  
  // Init Header message: FileSize & MD5 checksum
  fileHeader.m_size = _fileSize;
  
  if(_fileName)
  {
    strcpy(fileHeader.m_checkSum, MD5ChkSum(_fileName).c_str());
  }
  
  // Send response
  int bytesWritten = write(m_fd, &fileHeader, sizeof(fileHeader));
  if(SYS_ERR == bytesWritten || bytesWritten != sizeof(fileHeader))
  {
    perror("write");
    return false;
  }
  
  return true;
}


ssize_t FileServerProtocol::WriteFileToSocket(const char* _fileName) const
{
  assert(_fileName);
  
  static const size_t BUFFER_SIZE = 65536;
  char buffer[BUFFER_SIZE] = {0};
  ssize_t totBytesWritten  = 0;
  
  // Open filesList file
  FILE* pFile = fopen(_fileName, "rb");
	if(!pFile)
	{
		perror("fopen");
    return SYS_ERR;
	}
  
  // Send header
  if(!SendHeader(_fileName, FileSize(pFile)))    
  {
    cerr << "error: SendHeader failed" << endl;
    return SYS_ERR;
  }
  
  ssize_t fileSize = FileSize(pFile);
  if(SYS_ERR == fileSize)
  {
    cerr << "error: FileSize failed" << endl;
    return SYS_ERR;
  }
  
  // Breaks when file recived or on write error
  while(true)
  {
    // Recieve file chunk 
    int currBytesRead = fread(buffer, sizeof(char), sizeof(buffer), pFile);
    if(SYS_ERR == currBytesRead)
    {
      perror("fread");
      return SYS_ERR;
    }
    
    int currBytesWritten = write(m_fd, buffer, currBytesRead);
    if(SYS_ERR == currBytesWritten)    
    {
      perror("write");
      return SYS_ERR;
    }
    if(currBytesWritten == 0)
    {
      cerr << "error: connection closed" << endl;
      return SYS_ERR;
    }
    
    totBytesWritten += currBytesWritten;
    if(totBytesWritten == fileSize)
    {
      break;
    }
  }
  
  // Close file 
  if(fclose(pFile))
  {
  	perror("fclose");
    return SYS_ERR;
  }
  
  return fileSize;
}


bool FileServerProtocol::WriteSocketToFile(const char* _fileName) const
{
  assert(_fileName);
  
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
    int currBytesRead = Details::SocketRead(m_fd, buffer, sizeof(buffer));
    if(SYS_ERR == currBytesRead)
    {
      cerr << "error: SocketRead failed" << endl;
      return false;
    }
    
    int currBytesWritten = fwrite(buffer, sizeof(char), currBytesRead, pFile);
    if(SYS_ERR == currBytesWritten)    
    {
      perror("write");
      return false;
    }
    if(currBytesWritten == 0)
    {
      cerr << "error: connection closed" << endl;
      return false;
    }
    
    totBytesWritten += currBytesWritten; 
    if(totBytesWritten == m_recvHeader->m_size)
    {
      break;
    }
  }
  
  // Close file 
  if(fclose(pFile))
  {
  	perror("fclose");
    return false;
  }
  
  if(MD5ChkSum(_fileName) != m_recvHeader->m_checkSum)
  {
    cerr << "error: File MD5 Checksum Failed" << endl;
    return false;
  }
  
  return true;
}


// XXX RepoProtocol Implementation XXX


RepoProtocol::RepoProtocol(const char* _ip, short _port, int _id)
: FileServerProtocol(_ip, _port, _id) {} 


bool RepoProtocol::Discovery(const char* _fileName) const
{
  assert(_fileName);
  
  FSRepoMsg repoMsg;
  memset(&repoMsg, 0, sizeof(FSRepoMsg));
  repoMsg.m_type = FSRepoMsg::DISCOVERY;
  repoMsg.m_id   = m_id;
  
  // Write DISCOVERY message 
  if(!Details::SendMsg(m_fd, repoMsg))
  {
    cerr << "error: SendMsg failed" << endl;
    return false;
  }
  
  // Recieve Header 
  if(SYS_ERR == read(m_fd, m_recvHeader, sizeof(FileServerHeader)))    
  {
    perror("read");
    return false;
  }
  
  if(SYS_ERR == m_recvHeader->m_size)
  {
    cerr << "error: Invalid file" << endl;
    return false;
  }
  
  return WriteSocketToFile(_fileName);
}


bool RepoProtocol::GetServersDownload(const char* _fileName, IpPort* _ipp1, IpPort* _ipp2) const
{
  assert(_fileName);
  assert(_ipp1);
  assert(_ipp2);
  
  FSRepoMsg repoMsg;
  memset(&repoMsg, 0, sizeof(FSRepoMsg));
  repoMsg.m_type = FSRepoMsg::GET_SERV_DOWNLOAD;
  repoMsg.m_hashedfileName = HashFileName(_fileName);
  
  // Write GET_SERV_DOWNLOAD message 
  if(!Details::SendMsg(m_fd, repoMsg))
  {
    cerr << "error: SendMsg failed" << endl;
    return false;
  }
  
  memset(&repoMsg, 0, sizeof(FSRepoMsg));
  if(SYS_ERR == Details::SocketRead(m_fd, &repoMsg, sizeof(FSRepoMsg)))
  {
    cerr << "error: SocketRead failed" << endl;
    return false;
  }
  
  if(SYS_ERR == repoMsg.m_ipp1.GetPort() || SYS_ERR == repoMsg.m_ipp2.GetPort())
  {
    cerr << "error: Server not found" << endl;
    return false;
  }
  
  // Set the IP:PORT pair
  *_ipp1 = repoMsg.m_ipp1;
  *_ipp2 = repoMsg.m_ipp2;
  
  return true;
}


bool RepoProtocol::UpdateFilesList(const char* _fileName, const IpPort& _ipp1, const IpPort& _ipp2)
{
  assert(_fileName);
  
  // Write UPDATE_FILES_LIST message 
  FSRepoMsg repoMsg;
  memset(&repoMsg, 0, sizeof(FSRepoMsg));
  repoMsg.m_type = FSRepoMsg::UPDATE_FILES_LIST;
  strcpy(repoMsg.m_fileName, _fileName);
  repoMsg.m_ipp1 = _ipp1;
  repoMsg.m_ipp2 = _ipp2;
  
  if(!Details::SendMsg(m_fd, repoMsg))
  {
    fprintf(stderr, "\nSendMsg failed\n\n");
    return false;
  }
  
  return true;
}


bool RepoProtocol::GetServersUpload(IpPort* _ipp1, IpPort* _ipp2) const
{
  FSRepoMsg repoMsg;
  memset(&repoMsg, 0, sizeof(FSRepoMsg));
  repoMsg.m_type = FSRepoMsg::GET_SERV_UPLOAD;
  
  if(!Details::SendMsg(m_fd, repoMsg))
  {
    fprintf(stderr, "\nSendMsg failed\n\n");
    return false;
  }
  
  memset(&repoMsg, 0, sizeof(FSRepoMsg));
  if(SYS_ERR == Details::SocketRead(m_fd, &repoMsg, sizeof(FSRepoMsg)))
  {
    fprintf(stderr, "\nSocketRead failed\n\n");
    return false;
  }
  
  if(SYS_ERR == repoMsg.m_ipp1.GetPort() || SYS_ERR == repoMsg.m_ipp2.GetPort())
  {
    fprintf(stderr, "\nServers not found\n\n");
    return false;
  }
  
  // Set the IP:PORT
  *_ipp1 = repoMsg.m_ipp1;
  *_ipp2 = repoMsg.m_ipp2;
  
  return true;
}


FileServiceProtocol::FileServiceProtocol(const char* _ip, short _port, int _id)
: FileServerProtocol(_ip, _port, _id) {} 


bool FileServiceProtocol::Download(const char* _outputFilePath, const char* _requestedFile) const
{
  assert(_outputFilePath);
  assert(_requestedFile);
  
  FSServiceMsg serviceMsg;
  memset(&serviceMsg, 0, sizeof(FSServiceMsg));
  serviceMsg.m_type = FSServiceMsg::DOWNLOAD;
  serviceMsg.m_id   = m_id;
  serviceMsg.m_hashedfileName = HashFileName(_requestedFile);
  
  // Write DOWNLOAD message 
  if(!Details::SendMsg(m_fd, serviceMsg))
  {
    std::cerr << "SendMsg failed" << std::endl;
    return false;
  }
  
  // Recieve Header 
  if(SYS_ERR == read(m_fd, m_recvHeader, sizeof(FileServerHeader)))    
  {
    perror("read");
    return false;
  }
  
  if(SYS_ERR == m_recvHeader->m_size)
  {
    std::cerr << "error: invalid file" << std::endl << std::endl;
    return false;
  }
  
  return WriteSocketToFile(_outputFilePath);
}


bool FileServiceProtocol::Upload(const char* _fileName) const
{
  assert(_fileName);
  
  FSServiceMsg serviceMsg;
  memset(&serviceMsg, 0, sizeof(FSServiceMsg));
  serviceMsg.m_type = FSServiceMsg::UPLOAD;
  serviceMsg.m_id   = m_id;
  
  // Remove path from filename
  string fileName(_fileName);
  fileName.erase(0, fileName.find_last_of("/") + 1);

  serviceMsg.m_hashedfileName = HashFileName(fileName.c_str());
  
  // Write UPLOAD message 
  if(!Details::SendMsg(m_fd, serviceMsg))
  {
    fprintf(stderr, "\nSendMsg failed\n\n");
    return false;
  }
  
  // Upload the file 
  ssize_t fileSize = WriteFileToSocket(_fileName);
  if(SYS_ERR == fileSize)    
  {
    cerr << "error: WriteFileToSocket failed" << endl;
    return false;
  }
  
  // Recieve Header for upload size confirmation 
  if(SYS_ERR == read(m_fd, m_recvHeader, sizeof(FileServerHeader)))    
  {
    perror("read");
    return false;
  }
  
  if(fileSize != m_recvHeader->m_size)
  {
    cerr << "error: invalid file size" << endl;
    return false;
  }
  
  return true;
}


// XXX non class members functions

bool UploadPair(const char* _fileName, const FileServiceProtocol& _fs1, const FileServiceProtocol& _fs2)
{
  string evenBytesFileName(_fileName);
  string oddBytesFileName(_fileName);
  
  evenBytesFileName += "@even";
  oddBytesFileName  += "@odd";
  
  const char* evenBytesFileNameCStr = evenBytesFileName.c_str();
  const char* oddBytesFileNameCStr  = oddBytesFileName.c_str();
  
  // Divide the file for the 2 servers
  if(SYS_ERR == DivideFile(_fileName, evenBytesFileNameCStr, oddBytesFileNameCStr))
  {
    PRINT_ERR("DivideFile failed");
    return false;
  }
  PRINT_MSG("File division OK");
    
  // Uploading first part (even bytes)
  PRINT_MSG("Starting file upload to server1...");
  if(!_fs1.Upload(evenBytesFileNameCStr))
  {
    PRINT_ERR("Upload to server1 Failed!");
    return false;
  }
  PRINT_MSG("Upload to server1 & MD5 checksum OK");
   
  // Uploading second part (odd bytes)
  PRINT_MSG("Starting file upload to server2...");
  if(!_fs2.Upload(oddBytesFileNameCStr))
  {
    PRINT_ERR("Upload to server2 Failed!");
    return false;
  }
  PRINT_MSG("Upload to server2 & MD5 checksum OK");
  
  // Remove the 2 temporary file part after uploading
  if(SYS_ERR == RemoveFile(evenBytesFileNameCStr))
  {
    PRINT_ERR("RemoveFile failed");
    return false;
  }
  if(SYS_ERR == RemoveFile(oddBytesFileNameCStr))
  {
    PRINT_ERR("RemoveFile failed");
    return false;
  }
  
  return true;
}



bool DownloadPair(const char* _outputFileName, const char* _requestedFileName, const FileServiceProtocol& _fs1, const FileServiceProtocol& _fs2)
{
  string evenFileNameServ(_requestedFileName);
  string oddFileNameServ(_requestedFileName);
  
  evenFileNameServ += "@even";
  oddFileNameServ  += "@odd";
  
  string evenFileNameClient(_outputFileName);
  string oddFileNameClient(_outputFileName);
  
  evenFileNameClient += "@even";
  oddFileNameClient  += "@odd";
  
  const char* evenFileNameClientCStr = evenFileNameClient.c_str();
  const char* oddFileNameClientCStr  = oddFileNameClient.c_str();
  
  // Downloading first part (even bytes)
  PRINT_MSG("Starting file download from server1...");
  if(!_fs1.Download(evenFileNameClientCStr, evenFileNameServ.c_str()))
  {
    PRINT_ERR("Download from server1 Failed!");
    return false;
  }
  PRINT_MSG("Download from server1 & MD5 checksum OK");
   
  // Downloading second part (odd bytes)
  PRINT_MSG("Starting file download from server2...");
  if(!_fs2.Download(oddFileNameClientCStr, oddFileNameServ.c_str()))
  {
    PRINT_ERR("Download from server2 Failed!");
    return false;
  }
  PRINT_MSG("Download from server2 & MD5 checksum OK");
   
  // Combine the files part to one file
  if(SYS_ERR == CombineFile(_outputFileName, evenFileNameClientCStr, oddFileNameClientCStr))
  {
    PRINT_ERR("CombineFile failed");
    return false;
  }
  PRINT_MSG("File combine OK");
  
  // Remove the 2 temporary file part after combine
  if(SYS_ERR == RemoveFile(evenFileNameClientCStr))
  {
    PRINT_ERR("RemoveFile failed");
    return false;
  }
  if(SYS_ERR == RemoveFile(oddFileNameClientCStr))
  {
    PRINT_ERR("RemoveFile failed");
    return false;
  }
  
  return true;
}


} // namespace adv_cpp


