/********************************************************
Author:             Lior Bondorevsky
Creation date:      19/12/2014
Last modofied date: 25/12/2014
Description:        Common to fileServer client & server
********************************************************/
#include <string>   // string
#include <cstring>  // memset
#include <cassert>  // assert
#include <iostream> // cout, endl
#include <cstdlib>  // system
#include <cstdio>   // files functions

#include "fileserver_common.h" // API

#define SYS_ERR -1

namespace adv_cpp
{


IpPort::IpPort()
: m_port(0)
{ 
  memset(m_ip, 0, sizeof(m_ip));
}
  
  
IpPort::IpPort(const char* _ip, short _port)
: m_port(_port)
{
  strcpy(m_ip, _ip);
}
  
  
IpPort& IpPort::operator=(const IpPort& _ipp)
{
  if(this != &_ipp)
  {
    strcpy(m_ip, _ipp.m_ip);
    m_port = _ipp.m_port;
  }
  return *this;
}
 
  
void IpPort::Print() const
{
  std::cout << "IP: " << m_ip << " PORT: " << m_port << std::endl;
  std::cout << std::endl;
}


void IpPort::SetPort(short _port)
{
  m_port = _port;
}


short IpPort::GetPort() const
{
  return m_port;
}


void IpPort::SetIp(const char* _ip)
{
  strncpy(m_ip, _ip, IPv4_SIZE);
}


const char* IpPort::GetIp() const
{
  return m_ip;
}


// XXX Non class members functions

// DJB2 hash function
unsigned long HashFileName(const char* _fileName)
{
  int c;
  unsigned long hashed = 5381;

  while((c = *_fileName++))
  {
    hashed = ((hashed << 5) + hashed) + c;
  }    

  return hashed;
}


std::string MD5ChkSum(const char* _fileName)
{
  FILE* chkSumFile = NULL;
  
  static const char* CHKSUM_FILENAME = "md5_chk_sum.temp";
  
  assert(_fileName);
  
  std::string md5sum;
  md5sum = md5sum + "md5sum " + _fileName + " > " + CHKSUM_FILENAME;
  system(md5sum.c_str());
  
  // Open checksum file 
  chkSumFile = fopen(CHKSUM_FILENAME, "r");
  if(!chkSumFile)
  {
    perror("fopen");
  }
  
  char result[MD5_SIZE] = {0};
  fread(result, sizeof(char), MD5_SIZE - 1, chkSumFile);
  if(ferror(chkSumFile))
  {
    perror("fread");
  }
  result[MD5_SIZE-1] = '\0';
  
  // Close server file 
  if(fclose(chkSumFile))
  {
  	perror("fclose");
  }
  
  std::string rmFile;
  rmFile = rmFile + "rm " + CHKSUM_FILENAME;
  system(rmFile.c_str()); 
  
  return std::string(result);
}


ssize_t FileSize(FILE* _fp)
{
	ssize_t result;
	
	assert(_fp);
	
	if(fseek(_fp, 0, SEEK_END))
	{
    perror("fseek");
    return SYS_ERR;
  } 
  
	result = ftell(_fp);
	if(SYS_ERR == result)
	{
    perror("ftell");
    return SYS_ERR;
  } 
	
	if(fseek(_fp, 0, SEEK_SET))
	{
    perror("fseek");
    return SYS_ERR;
  } 
	
	return result;
}

} // namespace adv_cpp
