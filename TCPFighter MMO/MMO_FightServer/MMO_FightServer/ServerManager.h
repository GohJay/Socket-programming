#pragma once
#include "GameServer.h"

class ServerManager
{
private:
	struct SERVER_INFO
	{
		int port;
		int logLevel;
	};
private:
	ServerManager();
	~ServerManager();
public:
	static ServerManager* GetInstance();
	void Run();
	void Control();
	void Monitor();
private:
	bool LoadData();
	bool Init();
private:
	GameServer _server;
	SERVER_INFO _serverInfo;
	bool _controlMode;
	bool _monitoringFlag;
	int _loopCount;
	DWORD _networkProcTime;
	DWORD _logicProcTime;
	DWORD _cleanupProcTime;
	DWORD _monitoringTime;
	static ServerManager _instance;
};
