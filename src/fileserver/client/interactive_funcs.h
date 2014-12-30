/*******************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			25/12/2014
Last modofied date: 25/12/2014
Description: 				Fileserver intreractive client API
*******************************************************/
#ifndef __APP_FUNC_H__
#define __APP_FUNC_H__

namespace adv_cpp
{

class RepoProtocol;

enum menu_t
{
	UPLOAD = 1,
	DISCOVERY,
  DOWNLOAD
};


void StartMsg();

void EndMsg();

void PrintMenu();

void PrintLine();

void PauseClear();

int OptionSwitch(menu_t option, RepoProtocol& _repo, int clientId);

} // namespace adv_cpp

#endif /* #ifndef __APP_FUNC_H__ */


