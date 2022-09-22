#ifndef __CLIENTMANAGER__H_
#define __CLIENTMANAGER__H_
#include "Client.h"

class ClientManager
{
private:
	ClientManager();
	~ClientManager();
public:
	static ClientManager* GetInstance();
	void Run();
private:
	void GetIPAddress(char* ipaddress);
private:
	Client* _client;
	static ClientManager _instance;
};

#endif