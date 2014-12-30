/********************************************
Author: 			 			Lior Bondorevsky
Creation date: 			17/12/2014
Last modofied date: 25/12/2014
Description: 				FileServer single client
********************************************/
#include <iostream> // cout, endl
#include <string>   // string
#include <cstdlib>  // system

#include "fileserver_protocolos.h" // RepoProtocol
#include "file_operations.h"       // PrintFile
#include "fileserver_common.h"     // IpPort

#define SYS_ERR -1
#define OK       0
#define ERR      1

#define PRINT_MSG(MSG)  (cout << "(*) " << (MSG) << endl << endl)
#define PRINT_ERR(ERR)  (cerr << "(!) " << (ERR) << endl << endl)

using namespace adv_cpp;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

static const char* DOWNLOAD_PATH = "/home/student/test/client/download/";
static const char* UPLOAD_PATH   = "/home/student/test/client/upload/";
static const char* DISC_FILE     = "/home/student/test/client/discovery_file"; 
static const int   CLIENT_ID     = 99;

static const char* SERVER_IP = "127.0.0.1";
static const short REPO_PORT = 5061;

int main()
{
  system("clear");
  
  // Repository init --> throws exception if ctor failed
  RepoProtocol repo(SERVER_IP, REPO_PORT, CLIENT_ID);

  // GET_SERV_UPLOAD PART
  IpPort ippAdd1;
  IpPort ippAdd2;
  if(!repo.GetServersUpload(&ippAdd1, &ippAdd2))
  {
    PRINT_ERR("GetServersUpload Failed!");
    return ERR;
  }
  PRINT_MSG("Got servers for upload");
  
  ippAdd1.Print();
  ippAdd2.Print();
  
  
  // Scope for FileServiceProtocol Upload
  {
    // Service init --> throws exception if ctor failed
    FileServiceProtocol fservUpload1(ippAdd1.GetIp(), ippAdd1.GetPort(), CLIENT_ID);
    FileServiceProtocol fservUpload2(ippAdd2.GetIp(), ippAdd2.GetPort(), CLIENT_ID);
    
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
      return ERR;
    }
    PRINT_MSG("File upload to servers OK");

  
    // UPDATE_FILES_LIST PART
    cout << "Specify uploaded file to update repository list: ";
    string addedFilename;
    cin >> addedFilename;
    cout << endl;
    
    if(!repo.UpdateFilesList(addedFilename.c_str(), ippAdd1, ippAdd2))
    {
      PRINT_ERR("UpdateFilesList Failed!");
      return ERR;
    }
    PRINT_MSG("UpdateFilesList OK :)");
  } // Upload Scope end
  
    
  // DISCOVERY PART
  PRINT_MSG("client sending DISCOVERY");
  if(!repo.Discovery(DISC_FILE))
  {
    PRINT_ERR("Discovery Failed!");
    return ERR;
  }
  PRINT_MSG("Repository files list transfer & MD5 checksum OK");
  
  PRINT_MSG("Printing repository discovery file:");
  PrintFile(DISC_FILE);
  
             
  // GET_SERV_DOWNLOAD PART
  cout << "Specify filename for download: ";
  string reqFilename;
  cin >> reqFilename;
  cout << endl;
  
  IpPort ippGet1;
  IpPort ippGet2;
  if(!repo.GetServersDownload(reqFilename.c_str(), &ippGet1, &ippGet2))
  {
    PRINT_ERR("GetServersDownload Failed!");
    return ERR;
  }
  PRINT_MSG("Got server for download");
  ippGet1.Print();
  ippGet2.Print();
  
  
  // Scope for FileServiceProtocol Download
  {
    // Service init --> throws exception if ctor failed
    FileServiceProtocol fservDown1(ippGet1.GetIp(), ippGet1.GetPort(), CLIENT_ID);
    FileServiceProtocol fservDown2(ippGet2.GetIp(), ippGet2.GetPort(), CLIENT_ID);
    
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
      return ERR;
    }
    PRINT_MSG("File download from servers OK");
  } // Download Scope end
  
  cout << endl;
 
	return OK;
}
