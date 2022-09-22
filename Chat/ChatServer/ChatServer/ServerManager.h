#pragma once
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
	NetServer* _server;
	unsigned long _accumtime;
	static ServerManager _instance;
};
