#include "stdafx.h"
#include "BotManager.h"

BotManager BotManager::_instance;
BotManager::BotManager()
{
	GetIPAddress();
	GetClientSize();
	system("cls");
	for (int i = 0; i < _clientSize; i++)
	{
		_client[i] = new ClientBot();
	}
}
BotManager::~BotManager()
{
	for (int i = 0; i < _clientSize; i++)
	{
		delete _client[i];
	}
}
BotManager * BotManager::GetInstance()
{
	return &_instance;
}
void BotManager::Run()
{
	for (int i = 0; i < _clientSize; i++)
	{
		if (!_client[i]->IsConnected())
		{
			if (!_client[i]->Connect(_ipaddress, 3000))
				continue;
		}
		_client[i]->Update();
	}
	std::cout << ".";
}
void BotManager::GetIPAddress()
{
	std::cout << "접속할 IP 주소를 입력하세요 : ";
	std::cin >> _ipaddress;
}
void BotManager::GetClientSize()
{
	do
	{
		std::cout << "연결할 봇 클라이언트 수를 입력하세요(1 ~ 60) : ";
		std::cin >> _clientSize;
	} while (_clientSize <= 0 || _clientSize > 60);
}
