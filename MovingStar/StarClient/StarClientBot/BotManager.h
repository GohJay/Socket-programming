#ifndef __BOTMANAGER__H_
#define __BOTMANAGER__H_
#include "ClientBot.h"

class BotManager
{
private:
	BotManager();
	~BotManager();
public:
	static BotManager* GetInstance();
	void Run();
private:
	void GetIPAddress();
	void GetClientSize();
private:
	ClientBot* _client[60];
	int _clientSize;
	char _ipaddress[64];
	static BotManager _instance;
};

#endif