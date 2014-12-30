/*****************************************************
Author:             Lior Bondorevsky
Creation date:      25/12/2014
Last modofied date: 25/12/2014
Description:        File Operations Implementation
*****************************************************/
#include <iostream> // cout, endl, cerr
#include <fstream>  // ifstream, ofstream
#include <string>   // string
#include <cassert>  // assert
#include <cstdio>   // remove
#include <unistd.h> // access

#include "file_operations.h" // API

#define SYS_ERR -1
#define OK       0

using std::string;
using std::cerr;
using std::endl;
using std::cout;
using std::ifstream;
using std::ofstream;

namespace adv_cpp
{

int PrintFile(const char* _fileName)
{
  assert(_fileName);
  
  string line;
  ifstream file(_fileName);
  if(!file.is_open())
  {
    cerr << "Unable to open file " << _fileName << endl;    
    return SYS_ERR;
  }
  
  while(getline(file, line))
  {
    cout << line << endl;
  }
  file.close();
  
  cout << endl;
  
  return OK;
}


int DivideFile(const char* _origFileName, const char* _evenBytesFileName, const char* _oddBytesFileName)
{
  assert(_origFileName);
  assert(_evenBytesFileName);
  assert(_oddBytesFileName);
  
  ifstream file(_origFileName);
  if(!file.is_open())
  {
    cerr << "Unable to open file " << _origFileName << endl;    
    return SYS_ERR;
  }
  
  ofstream evenBytesFile(_evenBytesFileName);  
  if(!evenBytesFile.is_open())
  {
    cerr << "Unable to open file " << evenBytesFile << endl;    
    return SYS_ERR;
  }
  
  ofstream oddBytesFile(_oddBytesFileName);
  if(!oddBytesFile.is_open())
  {
    cerr << "Unable to open file " << oddBytesFile << endl;    
    return SYS_ERR;
  }
  
  string line;
  while(getline(file, line))
  {
    evenBytesFile << line << endl;
    if(!getline(file, line))
    {
      break;
    }
    oddBytesFile << line << endl;
  }
  
  file.close();
  evenBytesFile.close();
  oddBytesFile.close();
  
  return OK;
}


int CombineFile(const char* _outputFileName, const char* _evenBytesFileName, const char* _oddBytesFileName)
{
  assert(_outputFileName);
  assert(_evenBytesFileName);
  assert(_oddBytesFileName);
  
  ifstream evenBytesFile(_evenBytesFileName);  
  if(!evenBytesFile.is_open())
  {
    cerr << "Unable to open file " << evenBytesFile << endl;    
    return SYS_ERR;
  }
  
  ifstream oddBytesFile(_oddBytesFileName);
  if(!oddBytesFile.is_open())
  {
    cerr << "Unable to open file " << oddBytesFile << endl;    
    return SYS_ERR;
  }
  
  ofstream outputFile(_outputFileName);
  if(!outputFile.is_open())
  {
    cerr << "Unable to open file " << _outputFileName << endl;    
    return SYS_ERR;
  }
  
  string line;
  while(getline(evenBytesFile, line))
  {
    outputFile << line << endl;
    if(!getline(oddBytesFile, line))
    {
      break;
    }
    outputFile << line << endl;
  }
  
  evenBytesFile.close();
  oddBytesFile.close();
  outputFile.close();
  
  return OK;
}


int RemoveFile(const char* _fileName)
{
  if(remove(_fileName))
  {
    perror("remove");
    return SYS_ERR;
  }

  return OK;
}


bool IsFileExist(const char* _fileName)
{
  if(SYS_ERR != access(_fileName, F_OK))
  {
    return true;
  } 

  return false;
}


} // namespace adv_cpp__
