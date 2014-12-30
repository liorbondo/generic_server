/*******************************************************
Author:             Lior Bondorevsky
Creation date:      25/12/2014
Last modofied date: 25/12/2014
Description:        Interactive client for fileserver
*******************************************************/
#include <cstdio> // scanf

#include "interactive_funcs.h"     // CLIENT_ID, PrintMenu, OptionSwitch
#include "fileserver_protocolos.h" // RepoProtocol

static const char* SERVER_IP = "127.0.0.1";
static const short REPO_PORT = 5061;
static const int   CLIENT_ID = 99;

using namespace adv_cpp;

int main()
{
  menu_t option;
	bool cont = true;

  // Repository init --> throws exception if ctor failed
  RepoProtocol repo(SERVER_IP, REPO_PORT, CLIENT_ID);
  
	StartMsg();
	
	while(cont)
	{
	  PrintMenu();
	  scanf("%d", (int*)&option);
	  PrintLine();
	  
	  cont = OptionSwitch(option, repo, CLIENT_ID);
	  PauseClear();
	}
	
	EndMsg();
	
	return 0;
}

