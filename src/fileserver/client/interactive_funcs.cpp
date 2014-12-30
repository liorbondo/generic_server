/*************************************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			25/12/2014
Last modofied date: 25/12/2014
Description: 				Application Functions implementation
*************************************************************************/
#include <iostream>  // cout
#include <cstdio>    // getchar 
#include <cstdlib>   // system 

#include "interactive_funcs.h"     // API
#include "fileserver_protocolos.h" // RepoProtocol
#include "file_operations.h"       // PrintFile
#include "fileserver_common.h"     // IpPort

#define SYS_ERR -1
#define OK       0
#define FALSE 	 0
#define TRUE  	 1
#define ERR      1

#define PRINT_MSG(MSG)  (cout << "(*) " << (MSG) << endl << endl)
#define PRINT_ERR(ERR)  (cerr << "(!) " << (ERR) << endl << endl)

using namespace adv_cpp;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

namespace adv_cpp
{

static const char* DOWNLOAD_PATH = "/home/lior/test/client/download/";
static const char* UPLOAD_PATH   = "/home/lior/test/client/upload/";
static const char* DISC_FILE     = "/home/lior/test/client/discovery_file"; 


void PauseClear()
{
	cout << endl;
	getchar();
	cout << "please press ENTER to continue. ";
	getchar();
	system("clear");
}


void PrintMenu()
{
	cout << endl;
	cout << "MENU:" 			    << endl;
	cout << "-----" 				  << endl;
	cout << "1. Upload file"  << endl;
	cout << "2. Get repository files list" << endl;
	cout << "3. Download file" << endl;

	cout << "Any another number - stop" << endl << endl;
	cout << "Choose option: ";
}


void PrintLine()
{
	cout << "______________________________" << endl << endl;
}

void EndMsg()
{
	cout << endl;
	cout << "***App finished***" << endl << endl;
}


void StartMsg()
{
	system("clear");
	cout << endl;
	cout << "***Fileserver interactive App***" << endl << endl;
	cout << "press ENTER to start... ";
	getchar();
	system("clear");
}


static bool ClientUpload(const IpPort& _ipp1, const IpPort& _ipp2, int _clientId)
{
  // Service init --> throws exception if ctor failed
  FileServiceProtocol fservUpload1(_ipp1.GetIp(), _ipp1.GetPort(), _clientId);
  FileServiceProtocol fservUpload2(_ipp2.GetIp(), _ipp2.GetPort(), _clientId);
  
  // UPLOAD filename input
  cout << "Specify filename to upload: ";
  string inFilename;
  cin >> inFilename;
  inFilename = UPLOAD_PATH + inFilename;
  cout << endl;
  
  // UPLOAD 
  if(!UploadPair(inFilename.c_str(), fservUpload1, fservUpload2))
  {
    PRINT_ERR("UploadPair failed");
    return false;
  }
  PRINT_MSG("File upload to servers OK");
  
  return true;
}


static bool ClientUpdateRepo(RepoProtocol& _repo, const IpPort& _ipp1, const IpPort& _ipp2)
{
  // UPDATE_FILES_LIST PART
  cout << "Specify uploaded file to update repository list: ";
  string addedFilename;
  cin >> addedFilename;
  cout << endl;
  
  if(!_repo.UpdateFilesList(addedFilename.c_str(), _ipp1, _ipp2))
  {
    PRINT_ERR("UpdateFilesList Failed!");
    return false;
  }
  PRINT_MSG("UpdateFilesList OK :)");
  
  return true;
}


static bool ClientGetUploadServers(const RepoProtocol& _repo, IpPort* _ipp1, IpPort* _ipp2)
{
  
  if(!_repo.GetServersUpload(_ipp1, _ipp2))
  {
    PRINT_ERR("GetServersUpload Failed!");
    return false;
  }
  PRINT_MSG("Got servers for upload");
  
  _ipp1->Print();
  _ipp2->Print();
  
  return true;
}


static bool ClientDiscovery(const RepoProtocol& _repo)
{
  PRINT_MSG("client sending DISCOVERY");
  if(!_repo.Discovery(DISC_FILE))
  {
    PRINT_ERR("Discovery Failed!");
    return false;
  }
  PRINT_MSG("Repository files list transfer & MD5 checksum OK");
  
  PRINT_MSG("Printing repository discovery file:");
  PrintFile(DISC_FILE);
  
  return true;
}


static bool ClientDownload(const RepoProtocol& _repo, int _clientId)
{
  // GET_SERV_DOWNLOAD PART
  cout << "Specify filename for download: ";
  string reqFilename;
  cin >> reqFilename;
  cout << endl;
  
  IpPort ippGet1;
  IpPort ippGet2;
  if(!_repo.GetServersDownload(reqFilename.c_str(), &ippGet1, &ippGet2))
  {
    PRINT_ERR("GetServersDownload Failed!");
    return false;
  }
  PRINT_MSG("Got server for download");
  ippGet1.Print();
  ippGet2.Print();
  
  // Service init --> throws exception if ctor failed
  FileServiceProtocol fservDown1(ippGet1.GetIp(), ippGet1.GetPort(), _clientId);
  FileServiceProtocol fservDown2(ippGet2.GetIp(), ippGet2.GetPort(), _clientId);
  
  // DOWNLOAD PART
  cout << "save the file as: ";
  string outFilename;
  cin >> outFilename;
  outFilename = DOWNLOAD_PATH + outFilename;
  cout << endl;
  PRINT_MSG("Starting file download from server...");
  
  if(!DownloadPair(outFilename.c_str(), reqFilename.c_str(), fservDown1, fservDown2))
  {
    PRINT_ERR("DownloadPair failed");
    return false;
  }
  PRINT_MSG("File download from servers OK");
  
  return true;
} 


int OptionSwitch(menu_t _option, RepoProtocol& _repo, int _clientId)
{
  switch(_option)
  {
  
  case UPLOAD:
  {
    IpPort ippUp1;
    IpPort ippUp2;
    
    if(!ClientGetUploadServers(_repo, &ippUp1, &ippUp2))
    {
      PRINT_ERR("ClientGetUploadServers Failed!");
      break;
    }
    
    if(!ClientUpload(ippUp1, ippUp2, _clientId))
    {
      PRINT_ERR("ClientUpload Failed!");
      break;
    }
    
    if(!ClientUpdateRepo(_repo, ippUp1, ippUp2))
    {
      PRINT_ERR("ClientUpdateRepo Failed!");
      break;
    }
    
    break;
  }
  
  
	case DISCOVERY:
	
    if(!ClientDiscovery(_repo))
    {
      PRINT_ERR("ClientDiscovery Failed!");
    }
     
	break;
	
	
	case DOWNLOAD:
	
    if(!ClientDownload(_repo, _clientId))
    {
      PRINT_ERR("ClientDownload Failed!");
    }
     
	break;
	
	default:
	  return 0; 
  }
  
  return 1;
}

} // namespace adv_cpp
