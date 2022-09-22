#ifndef __CLIENT__H_
#define __CLIENT__H_
#include <unordered_map>

class ClientManager;
class Client
{
private:
	struct Player
	{
		int id;
		int x;
		int y;
	};
public:
	Client();
	~Client();
public:
	bool Connect(const char* ipaddress, int port);
	void Disconnect();
	bool IsConnected();
	void Update();
	void Render();
	void Cleanup();
private:
	void RecvProc();
	void SendProc();
	void MessageProc(const char* message);
private:
	bool _connected;
	unsigned int _socket;
	int _myId;
	std::unordered_map<int, Player*> _mapPlayer;
	friend class ClientManager;
};

#endif