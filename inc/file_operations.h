/*****************************************************
Author:             Lior Bondorevsky
Creation date:      20/12/2014
Last modofied date: 25/12/2014
Description:        File Operations Inline Fucntions
*****************************************************/
#ifndef __FILE_OPERATIONS__
#define __FILE_OPERATIONS__

namespace adv_cpp
{

// Prints a file to std output
int PrintFile(const char* fileName);

// Divides a file (_origFileName) to 2 spearate file of even & odd blocks of the original file. return -1 on error
int DivideFile(const char* origFileName, const char* evenFileName, const char* oddFileName);

// Combine the 2 file divided by DivideFile back to the original file. return -1 on error
int CombineFile(const char* outputFileName, const char* evenBytesFileName, const char* oddBytesFileName);

// Removes a file from disk. return -1 on error
int RemoveFile(const char* fileName);

// return true if the file exist and false if not
bool IsFileExist(const char* _fileName);

} // namespace adv_cpp

#endif // ifndef __FILE_OPERATIONS__
