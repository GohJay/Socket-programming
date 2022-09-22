#pragma once
#include "GameServer.h"

class ServerManager
{
private:
	ServerManager();
	~ServerManager();
public:
	static ServerManager* GetInstance();
	void Run();
private:
	GameServer* _server;
	unsigned long _accumtime;
	static ServerManager _instance;
};
