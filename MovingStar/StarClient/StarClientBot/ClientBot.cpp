#include "stdafx.h"
#include "ClientBot.h"
#include "../../Common/Log.h"
#include "../../Common/Protocol.h"
#pragma comment(lib, "ws2_32")

#define UNKNOWN_ID -1
#define CRASH \
do\
{\
	int *ptr = nullptr;\
	*ptr = 100;\
} while(0)

#define dfSCREEN_WIDTH	81
#define dfSCREEN_HEIGHT 23
bool IsOutOfScreen(int x, int y)
{
	if (x < 0 || y < 0 || x >= dfSCREEN_WIDTH - 1 || y >= dfSCREEN_HEIGHT)
		return true;

	return false;
}

ClientBot::ClientBot() : _connected(false), _socket(INVALID_SOCKET), _myId(UNKNOWN_ID)
{
	WSADATA wsadata;
	int status = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (status != 0)
	{
		Jay::WriteLog("ClientBot::Client() Failed WSASTARtup: %d\n", status);
		CRASH;
	}
}
ClientBot::~ClientBot()
{
	Cleanup();
	WSACleanup();
}
bool ClientBot::Connect(const char* ipaddress, int port)
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("ClientBot::Connect() Failed socket: %d\n", err);
		return false;
	}
	SOCKADDR_IN svrAddr = {};
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_port = htons(port);
	svrAddr.sin_addr.s_addr = inet_addr(ipaddress);
	int result = connect(_socket, (SOCKADDR*)&svrAddr, sizeof(svrAddr));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("ClientBot::Connect() Failed connect: %d\n", err);
		closesocket(_socket);
		return false;
	}
	u_long nonBlockingMode = 1;
	result = ioctlsocket(_socket, FIONBIO, &nonBlockingMode);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("ClientBot::Connect() Failed ioctlsocket: %d\n", err);
		closesocket(_socket);
		return false;
	}
	_connected = true;
	return true;
}
void ClientBot::Disconnect()
{
	closesocket(_socket);
	_connected = false;
}
bool ClientBot::IsConnected()
{
	return _connected;
}
void ClientBot::Update()
{
	AutoMove();
	if (_myId != UNKNOWN_ID && IsMove())
		SendProc();
	RecvProc();
}
void ClientBot::Cleanup()
{
	for (auto iter = _mapPlayer.begin(); iter != _mapPlayer.end();)
	{
		Player *player = iter->second;
		delete player;
		iter = _mapPlayer.erase(iter);
	}
	_myId = UNKNOWN_ID;
}
void ClientBot::RecvProc()
{
	for (;;)
	{
		fd_set rfds;
		timeval tv = { 0, 0 };
		FD_ZERO(&rfds);
		FD_SET(_socket, &rfds);
		int result = select(NULL, &rfds, NULL, NULL, &tv);
		if (result == SOCKET_ERROR)
		{
			char log[64];
			sprintf_s(log, "ClientBot::Recv() Failed select: %d", WSAGetLastError());
			MessageBoxA(NULL, log, "StarClientBot", MB_ICONERROR);
			CRASH;
		}

		if (!FD_ISSET(_socket, &rfds))
			break;

		int err;
		char buffer[16];
		result = recv(_socket, buffer, sizeof(buffer), 0);
		switch (result)
		{
		case SOCKET_ERROR:
			err = WSAGetLastError();
			if (err != WSAECONNRESET)
				Jay::WriteLog("ClientBot::Recv() Failed recv: %d\n", err);
		case 0:
			Disconnect();
			Cleanup();
			return;
		default:
			MessageProc(buffer);
			break;
		}
	}
}
void ClientBot::SendProc()
{
	auto iter = _mapPlayer.find(_myId);
	if (iter == _mapPlayer.end())
		return;

	Player *player = iter->second;
	if (_autoMove.left && !IsOutOfScreen(player->x - 1, player->y))
		player->x--;
	if (_autoMove.right && !IsOutOfScreen(player->x + 1, player->y))
		player->x++;
	if (_autoMove.up && !IsOutOfScreen(player->x, player->y - 1))
		player->y--;
	if (_autoMove.down && !IsOutOfScreen(player->x, player->y + 1))
		player->y++;

	STAR_MOVE_PACKET packet;
	packet.type = PACKET_TYPE::MOVE;
	packet.id = player->id;
	packet.x = player->x;
	packet.y = player->y;
	int result = send(_socket, (char*)&packet, sizeof(STAR_MOVE_PACKET), 0);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("ClientBot::Move() Failed send: %d\n", err);
	}
}
void ClientBot::MessageProc(const char * message)
{
	int *type = (int*)message;
	switch (*type)
	{
	case PACKET_TYPE::ALLOC:
		{
			STAR_ALLOC_PACKET *packet = (STAR_ALLOC_PACKET*)message;
			_myId = packet->id;
		}
		break;
	case PACKET_TYPE::CREATE:
		{
			STAR_CREATE_PACKET *packet = (STAR_CREATE_PACKET*)message;
			auto iter = _mapPlayer.find(packet->id);
			if (iter == _mapPlayer.end())
			{
				Player *player = new Player;
				player->id = packet->id;
				player->x = packet->x;
				player->y = packet->y;
				_mapPlayer.insert(std::pair<int, Player*>(packet->id, player));
			}
		}
		break;
	case PACKET_TYPE::DESTROY:
		{
			STAR_DESTROY_PACKET *packet = (STAR_DESTROY_PACKET*)message;
			auto iter = _mapPlayer.find(packet->id);
			if (iter != _mapPlayer.end())
			{
				Player *player = iter->second;
				delete player;
				_mapPlayer.erase(iter);
			}
		}
		break;
	case PACKET_TYPE::MOVE:
		break;
	default:
		Jay::WriteLog("ClientBot::MessageProc() UNKNOWN_PACKET\n");
		break;
	}
}
void ClientBot::AutoMove()
{
	int autoMove = rand() % 4;
	switch (autoMove)
	{
	case 0:
		_autoMove.down = true;
		_autoMove.left = false;
		_autoMove.right = false;
		_autoMove.up = false;
		break;
	case 1:
		_autoMove.down = false;
		_autoMove.left = true;
		_autoMove.right = false;
		_autoMove.up = false;
		break;
	case 2:
		_autoMove.down = false;
		_autoMove.left = false;
		_autoMove.right = true;
		_autoMove.up = false;
		break;
	case 3:
		_autoMove.down = false;
		_autoMove.left = false;
		_autoMove.right = false;
		_autoMove.up = true;
		break;
	default:
		_autoMove.down = false;
		_autoMove.left = false;
		_autoMove.right = false;
		_autoMove.up = false;
		break;
	}
}
bool ClientBot::IsMove()
{
	return _autoMove.down || _autoMove.left || _autoMove.right || _autoMove.up;
}
