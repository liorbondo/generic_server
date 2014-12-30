/********************************************************
Author:             Lior Bondorevsky
Creation date:      19/12/2014
Last modofied date: 25/12/2014
Description:        Common to fileServer client & server
********************************************************/
#ifndef __FILESERVER_COMMON__
#define __FILESERVER_COMMON__

#include <string> // string

#define SYS_ERR -1

namespace adv_cpp
{

static const size_t MD5_SIZE  = 33;
static const size_t IPv4_SIZE = 16;


struct FileServerHeader
{
  ssize_t m_size; 
  char    m_checkSum[MD5_SIZE]; 
};


class IpPort
{
public:
  IpPort();
  
  IpPort(const char* ip, short port);
  IpPort& operator=(const IpPort& _ipp);
  
  void  SetPort(short port);
  short GetPort() const;
  
  void SetIp(const char* ip);
  const char* GetIp() const;
  
  void Print() const;
  
private:  
  char  m_ip[IPv4_SIZE];
  short m_port;
};


struct FSRepoMsg
{
  enum
  {
    DISCOVERY = 1,
    GET_SERV_DOWNLOAD,
    GET_SERV_UPLOAD,
    UPDATE_FILES_LIST
  }m_type; 
  
  int m_id;
  unsigned long m_hashedfileName;
  char m_fileName[64];
  IpPort m_ipp1;
  IpPort m_ipp2;
};


struct FSServiceMsg
{
  enum
  {
    DOWNLOAD = 1,
    UPLOAD
  }m_type; 
  
  int m_id;
  unsigned long m_hashedfileName;
};


// DJB2 hash function
unsigned long HashFileName(const char* _fileName);

// Return the MD5Checksum of a file
std::string MD5ChkSum(const char* _fileName);

// Return the file size in bytes & -1 for error
ssize_t FileSize(FILE* _fp);

} // namespace adv_cpp

#endif // ifndef __FILESERVER_COMMON__
