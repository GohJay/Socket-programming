#ifndef __SERVERMANAGER__H_
#define __SERVERMANAGER__H_
#include "Net_Server.h"

class ServerManager
{
private:
	ServerManager();
	~ServerManager();
public:
	static ServerManager* GetInstance();
	void Run();
private:
	void PrintInfo();
private:
	Net_Server* _server;
	static ServerManager _instance;
};

#endif