/*****************************************
Author: 			 			Lior Bondorevsky
Creation date: 			17/12/2014
Last modofied date: 24/12/2014
Description: 				FileServer Protocols 
*****************************************/
#ifndef __FILESERVER_PROTOCOLOS__
#define __FILESERVER_PROTOCOLOS__

namespace adv_cpp
{

class FileServerHeader;

// Base class. Used only by inheritors
class FileServerProtocol
{
protected:
  FileServerProtocol(const char* ip, short port, int id); 
  ~FileServerProtocol();
  
  // Write Socket data into a file
  bool    WriteSocketToFile(const char* fileName) const;
  ssize_t WriteFileToSocket(const char* fileName) const;
  bool    SendHeader(const char* fileName, ssize_t fileSize) const;
  
  int m_fd;
  int m_id;
  FileServerHeader* m_recvHeader;
};

class IpPort;

class RepoProtocol : public FileServerProtocol
{
public: 
  RepoProtocol(const char* ip, short port, int id);
  // ~RepoProtocol() = default;
  
  // Recieve the repository files list
  // Return false for error & true if transfer & MD5ChkSum OK
  bool Discovery(const char* outputFilePath) const;
  
  // Get the pair of IP:PORT for requested file Downloading
  // Return false for error & true if OK
  bool GetServersDownload(const char* fileName, IpPort*, IpPort*) const;  
  
  // Get the pair of IP:PORT for requested file Uploading
  // Return false for error & true if OK
  bool GetServersUpload(IpPort*, IpPort*) const;
  
  // Update Repository file list with a new file and his IP:PORT pair
  bool UpdateFilesList(const char* fileName, const IpPort&, const IpPort&); 
};


class FileServiceProtocol : public FileServerProtocol
{
public: 
  FileServiceProtocol(const char* ip, short port, int id);
  // ~FileServiceProtocol() = default;
  
  // Download the repository requestedFile list to outputFilePath
  // Return false for error & true if transfer & MD5ChkSum OK
  bool Download(const char* outputFilePath, const char* requestedFile) const;
  
  // Upload file to server recieved from repository
  // Return false for error & true if transfer & MD5ChkSum OK (recieve server confirmation)
  bool Upload(const char* _fileName) const;
};


// Upload file to a pair of FileService servers recieved from repository
bool UploadPair(const char* fileName, const FileServiceProtocol&, const FileServiceProtocol&);

// Download requestedFileName file, saved as outputFileName from a pair of FileService Servers recieved from repository
bool DownloadPair(const char* outputFileName, const char* requestedFileName, const FileServiceProtocol&, const FileServiceProtocol&);

} // namespace adv_cpp

#endif /* #ifndef __FILE_SERVER_PROTOCOLOS__ */
