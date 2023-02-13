#include "stdafx.h"
#include "ServerManager.h"
#include "Timer.h"
#include "define.h"

ServerManager ServerManager::_instance;
ServerManager::ServerManager() : _accumtime(0)
{
	_server = new GameServer();
}
ServerManager::~ServerManager()
{
	delete _server;
}
ServerManager * ServerManager::GetInstance()
{
	return &_instance;
}
void ServerManager::Run()
{
	//Network
	_server->Network();

	//Logic Update
	_accumtime += Timer::GetDeltaTime();
	if (_accumtime >= dfINTERVAL)
	{
		_accumtime -= dfINTERVAL;
		_server->Update();
	}

	//Session Cleanup
	_server->Cleanup();
}
