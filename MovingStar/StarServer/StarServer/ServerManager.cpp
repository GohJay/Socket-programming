#include "stdafx.h"
#include "ServerManager.h"
#include "Console.h"

ServerManager ServerManager::_instance;
ServerManager::ServerManager()
{
	_server = new NetServer();
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
	PrintInfo();
	_server->Update();
	_server->Cleanup();
	_server->Render();
}
void ServerManager::PrintInfo()
{
	cs_MoveCursor(0, 0);
	printf_s("Connected Client : %d", _server->_playerList.size());
}
