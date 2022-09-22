#ifndef __CLIENT__H_
#define __CLIENT__H_
#include <unordered_map>

class BotManager;
class ClientBot
{
private:
	struct Move
	{
		bool left;
		bool right;
		bool up;
		bool down;
	};
	struct Player
	{
		int id;
		int x;
		int y;
	};
public:
	ClientBot();
	~ClientBot();
public:
	bool Connect(const char* ipaddress, int port);
	void Disconnect();
	bool IsConnected();
	void Update();
	void Cleanup();
private:
	void RecvProc();
	void SendProc();
	void AutoMove();
	bool IsMove();
	void MessageProc(const char* message);
private:
	bool _connected;
	unsigned int _socket;
	int _myId;
	Move _autoMove;
	std::unordered_map<int, Player*> _mapPlayer;
	friend class BotManager;
};

#endif