#ifndef __SERVERMANAGER__H_
#define __SERVERMANAGER__H_
#include "NetServer.h"

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
	NetServer* _server;
	static ServerManager _instance;
};

#endif