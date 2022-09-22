#include "stdafx.h"
#include "ServerManager.h"

ServerManager ServerManager::_instance;
ServerManager::ServerManager() : _accumtime(0)
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
	//Network
	_server->Network();

	//Cleanup
	_server->Cleanup();
}
