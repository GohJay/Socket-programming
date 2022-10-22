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
	void Control();
	void Monitor();
private:
	GameServer* _server;
	bool _controlMode;
	bool _monitoringFlag;
	int _loopCount;
	int _frameCount;
	DWORD _accumtime;
	DWORD _networkProcTime;
	DWORD _logicProcTime;
	DWORD _cleanupProcTime;
	DWORD _monitoringTime;
	static ServerManager _instance;
};
