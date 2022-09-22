#include "stdafx.h"
#include "Client.h"
#include "KeyHandler.h"
#include "ScreenBuffer.h"
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

Client::Client() : _connected(false), _socket(INVALID_SOCKET), _myId(UNKNOWN_ID)
{
	WSADATA wsadata;
	int status = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (status != 0)
	{
		Jay::WriteLog("Client::Client() Failed WSASTARtup: %d\n", status);
		CRASH;
	}
}
Client::~Client()
{
	Cleanup();
	WSACleanup();
}
bool Client::Connect(const char* ipaddress, int port)
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("Client::Connect() Failed socket: %d\n", err);
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
		Jay::WriteLog("Client::Connect() Failed connect: %d\n", err);
		closesocket(_socket);
		return false;
	}
	u_long nonBlockingMode = 1;
	result = ioctlsocket(_socket, FIONBIO, &nonBlockingMode);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("Client::Connect() Failed ioctlsocket: %d\n", err);
		closesocket(_socket);
		return false;
	}
	_connected = true;
	return true;
}
void Client::Disconnect()
{
	closesocket(_socket);
	_connected = false;
}
bool Client::IsConnected()
{
	return _connected;
}
void Client::Update()
{
	if (_myId != UNKNOWN_ID && KeyHandler::GetInstance()->IsMove())
		SendProc();

	RecvProc();
}
void Client::Render()
{
	ScreenBuffer::GetInstance()->Buffer_Clear();
	for (auto iter = _mapPlayer.begin(); iter != _mapPlayer.end(); ++iter)
	{
		Player *player = iter->second;
		ScreenBuffer::GetInstance()->Sprite_Draw(player->x, player->y, '*');
	}
}
void Client::Cleanup()
{
	for (auto iter = _mapPlayer.begin(); iter != _mapPlayer.end();)
	{
		Player *player = iter->second;
		delete player;
		iter = _mapPlayer.erase(iter);
	}
	_myId = UNKNOWN_ID;
}
void Client::RecvProc()
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
			sprintf_s(log, "Client::Recv() Failed select: %d", WSAGetLastError());
			MessageBoxA(NULL, log, "StarClient", MB_ICONERROR);
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
				Jay::WriteLog("Client::Recv() Failed recv: %d\n", err);
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
void Client::SendProc()
{
	auto iter = _mapPlayer.find(_myId);
	if (iter == _mapPlayer.end())
		return;

	Player *player = iter->second;
	if (KeyHandler::GetInstance()->_keyState.left && !ScreenBuffer::IsOutOfScreen(player->x - 1, player->y))
		player->x--;
	if (KeyHandler::GetInstance()->_keyState.right && !ScreenBuffer::IsOutOfScreen(player->x + 1, player->y))
		player->x++;
	if (KeyHandler::GetInstance()->_keyState.up && !ScreenBuffer::IsOutOfScreen(player->x, player->y - 1))
		player->y--;
	if (KeyHandler::GetInstance()->_keyState.down && !ScreenBuffer::IsOutOfScreen(player->x, player->y + 1))
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
		Jay::WriteLog("ClientClient::Move() Failed send: %d\n", err);
	}
}
void Client::MessageProc(const char * message)
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
		{
			STAR_MOVE_PACKET *packet = (STAR_MOVE_PACKET*)message;
			auto iter = _mapPlayer.find(packet->id);
			if (iter != _mapPlayer.end())
			{
				Player *player = iter->second;
				player->x = packet->x;
				player->y = packet->y;
			}
		}
		break;
	default:
		Jay::WriteLog("Client::MessageProc() UNKNOWN_PACKET\n");
		break;
	}
}
