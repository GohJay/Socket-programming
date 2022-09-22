#include "stdafx.h"
#include "ClientManager.h"
#include "Console.h"

ClientManager ClientManager::_instance;
ClientManager::ClientManager()
{
	_client = new Client();
}
ClientManager::~ClientManager()
{
	delete _client;
}
ClientManager* ClientManager::GetInstance()
{
	return &_instance;
}
void ClientManager::Run()
{
	if (!_client->IsConnected())
	{
		char ip[16];
		GetIPAddress(ip);
		if (!_client->Connect(ip, 3000))
			return;
	}
	_client->Update();
	_client->Render();
}
void ClientManager::GetIPAddress(char * ipaddress)
{
	cs_ClearScreen();
	cs_MoveCursor(0, 0);
	std::cout << "접속할 IP 주소를 입력하세요 : ";
	std::cin >> ipaddress;
}
