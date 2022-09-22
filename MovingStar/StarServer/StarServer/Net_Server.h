#ifndef __NET_SERVER__H_
#define __NET_SERVER__H_
#include "Object.h"
#include "../../Common/List.h"

class ServerManager;
class Net_Server
{
public:
	Net_Server();
	~Net_Server();
public:
	void Update();
	void Render();
	void Cleanup();
private:
	void Listen();
	void AcceptProc();
	void RecvProc(Player* player);
	void SendProc(Player* player);
	void MessageProc(Player* player, const char* message);
	void SendUnicast(Player* target, const char* message, int size);
	void SendBroadcast(Player* exclusion, const char* message, int size);
	void Disable(Player* player);
	void Disconnect(Player* player);
	void DestroyAll();
private:
	unsigned int _listenSocket;
	int _allocId;
	Jay::list<Player*> _playerList;
	friend class ServerManager;
};

#endif
