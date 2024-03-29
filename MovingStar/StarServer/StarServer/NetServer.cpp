#include "stdafx.h"
#include "NetServer.h"
#include "ScreenBuffer.h"
#include "../../Common/Log.h"
#include "../../Common/Protocol.h"
#include <mstcpip.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../../Network/lib/Release/Network.lib")

NetServer::NetServer() : _listenSocket(INVALID_SOCKET), _allocId(rand() % 1000)
{
	WSADATA ws;
	int status = WSAStartup(MAKEWORD(2, 2), &ws);
	if (status != 0)
	{
		Jay::WriteLog("NetServer::NetServer() Failed WSAStartup: %d\n", status);
		throw;
	}
	Listen();
}
NetServer::~NetServer()
{
	DestroyAll();
	WSACleanup();
}
void NetServer::Update()
{
	fd_set rfds;
	fd_set sfds;
	FD_ZERO(&rfds);
	FD_ZERO(&sfds);
	FD_SET(_listenSocket, &rfds);
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (player->sendBuffer.GetUseSize() > 0)
			FD_SET(player->socket, &sfds);
		FD_SET(player->socket, &rfds);
	}

	int result = select(NULL, &rfds, &sfds, NULL, NULL);
	if (result == SOCKET_ERROR)
	{
		char msg[64];
		sprintf_s(msg, "NetServer::Update() Failed select: %d", WSAGetLastError());
		MessageBoxA(NULL, msg, "StarServer", MB_ICONERROR);
		throw;
	}

	if (FD_ISSET(_listenSocket, &rfds))
		AcceptProc();

	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (FD_ISSET(player->socket, &sfds))
			SendProc(player);
	}
	
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (FD_ISSET(player->socket, &rfds))
			RecvProc(player);
	}
}  
void NetServer::Render()
{
	ScreenBuffer::GetInstance()->Buffer_Clear();
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		ScreenBuffer::GetInstance()->Sprite_Draw(player->star.x, player->star.y, '*');
	}
}
void NetServer::Cleanup()
{
	for (auto iter = _playerList.begin(); iter != _playerList.end();)
	{
		Player *player = *iter;
		if (!player->enable)
		{
			Disconnect(player);
			delete player;
			iter = _playerList.erase(iter);
		}
		else
			++iter;
	}
}
void NetServer::Listen()
{
	_listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listenSocket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("NetServer::Listen() Failed socket: %d\n", err);
		return;
	}

	linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	int result = setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("NetServer::Listen() Failed setsockopt linger: %d\n", err);
		closesocket(_listenSocket);
		return;
	}

	u_long nonBlockingMode = 1;
	result = ioctlsocket(_listenSocket, FIONBIO, &nonBlockingMode);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf(L"NetServer::Listen() Failed ioctlsocket: %d\n", err);
		closesocket(_listenSocket);
		return;
	}
	
	tcp_keepalive tcpkl;
	tcpkl.onoff = 1;
	tcpkl.keepalivetime = 5000;
	tcpkl.keepaliveinterval = 500;
	DWORD dwRet;
	result = WSAIoctl(_listenSocket, SIO_KEEPALIVE_VALS, &tcpkl, sizeof(tcpkl), 0, 0, &dwRet, NULL, NULL);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("NetServer::Listen() Failed WSAIoctl keepalive: %d\n", err);
		closesocket(_listenSocket);
		return;
	}
		
	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(3000);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(_listenSocket, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("NetServer::Listen() Failed bind: %d\n", err);
		closesocket(_listenSocket);
		return;
	}

	result = listen(_listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		Jay::WriteLog("NetServer::Listen() Failed listen: %d\n", err);
		closesocket(_listenSocket);
		return;
	}
}
void NetServer::AcceptProc()
{
	SOCKADDR_IN clientAddr = {};
	int clientSize = sizeof(clientAddr);
	SOCKET client = accept(_listenSocket, (SOCKADDR*)&clientAddr, &clientSize);
	if (client == INVALID_SOCKET)
	{
		char msg[64];
		sprintf_s(msg, "NetServer::Accept() Failed accept: %d", WSAGetLastError());
		MessageBoxA(NULL, msg, "StarServer", MB_ICONERROR);
		throw;
	}

	Player *new_player = new Player;
	new_player->enable = true;
	new_player->socket = client;
	InetNtopA(AF_INET, &clientAddr.sin_addr, new_player->ip, sizeof(new_player->ip));
	new_player->port = ntohs(clientAddr.sin_port);
	new_player->star.id = _allocId++;
	new_player->star.x = rand() % (dfSCREEN_WIDTH - 2);
	new_player->star.y = rand() % (dfSCREEN_HEIGHT - 1);
	_playerList.push_front(new_player);

	STAR_CREATE_PACKET create_packet;
	create_packet.type = PACKET_TYPE::CREATE;
	create_packet.id = new_player->star.id;
	create_packet.x = new_player->star.x;
	create_packet.y = new_player->star.y;
	SendBroadcast(new_player, (char*)&create_packet, sizeof(STAR_CREATE_PACKET));

	STAR_ALLOC_PACKET alloc_packet;
	alloc_packet.type = PACKET_TYPE::ALLOC;
	alloc_packet.id = new_player->star.id;
	SendUnicast(new_player, (char*)&alloc_packet, sizeof(STAR_ALLOC_PACKET));

	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		STAR_CREATE_PACKET create_packet;
		create_packet.type = PACKET_TYPE::CREATE;
		create_packet.id = player->star.id;
		create_packet.x = player->star.x;
		create_packet.y = player->star.y;
		SendUnicast(new_player, (char*)&create_packet, sizeof(STAR_CREATE_PACKET));
	}
	Jay::WriteLog("NetServer::AcceptProc() Connected IP: '%s', Port: '%d'\n", new_player->ip, new_player->port);
} 
void NetServer::RecvProc(Player* player)
{
	if (!player->enable)
		return;

	int err;
	char buffer[1024];
	int bufferSize = sizeof(buffer);
	int size = recv(player->socket, buffer, bufferSize, 0);
	switch (size)
	{
	case SOCKET_ERROR:
		err = WSAGetLastError();
		if (err != WSAECONNRESET)
			Jay::WriteLog("NetServer::Recv() Failed recv: %d\n", err);
	case 0:
		Disable(player);
		return;
	default:
		break;
	}

	int ret = player->recvBuffer.Enqueue(buffer, size);
	if (ret != size)
	{
		Jay::WriteLog("NetServer::Recv() Failed recvBuffer::Enqueue() [size : %d]\n", ret);
		Disable(player);
		return;
	}

	char message[16];
	int msgSize = sizeof(message);
	while (player->recvBuffer.GetUseSize() >= msgSize)
	{
		ret = player->recvBuffer.Dequeue(message, msgSize);
		if (ret != msgSize)
		{
			Jay::WriteLog("NetServer::Recv() Failed recvBuffer::Dequeue() [size : %d]\n", ret);
			throw;
		}
		MessageProc(player, message);
	}
}
void NetServer::SendProc(Player * player)
{
	if (!player->enable)
		return;

	char message[1024];
	int size = player->sendBuffer.Peek(message, 1024);
	if (size <= 0)
		return;

	int ret = send(player->socket, message, size, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		switch (err)
		{
		case WSAEWOULDBLOCK:
			return;
		case WSAECONNRESET:
			break;
		default:
			Jay::WriteLog("NetServer::SendProc() Failed send error: %d, IP: '%s', Port: '%d'\n", err, player->ip, player->port);
			break;
		}
		Disable(player);
		return;
	}
	player->sendBuffer.MoveFront(ret);
}
void NetServer::MessageProc(Player * player, const char * message)
{
	int *type = (int*)message;
	switch (*type)
	{
	case PACKET_TYPE::MOVE:
		{
			STAR_MOVE_PACKET *packet = (STAR_MOVE_PACKET*)message;
			if (packet->id == player->star.id)
			{
				if (!ScreenBuffer::IsOutOfScreen(packet->x, player->star.y))
					player->star.x = packet->x;
				if (!ScreenBuffer::IsOutOfScreen(player->star.x, packet->y))
					player->star.y = packet->y;

				packet->x = player->star.x;
				packet->y = player->star.y;
				SendBroadcast(player, (char*)packet, sizeof(STAR_MOVE_PACKET));
			}
		}
		break;
	default:
		Jay::WriteLog("NetServer::MessageProc() UNKNOWN_PACKET\n");
		break;
	}
}
void NetServer::SendUnicast(Player * target, const char * message, int size)
{
	int ret = target->sendBuffer.Enqueue(message, size);
	if (ret != size)
	{
		Jay::WriteLog("NetServer::Recv() Failed sendBuffer::Enqueue() [size : %d]\n", ret);
		Disable(target);
		return;
	}
}
void NetServer::SendBroadcast(Player * exclusion, const char * message, int size)
{
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (player != exclusion)
			SendUnicast(player, message, size);
	}
}
void NetServer::Disable(Player * player)
{
	player->enable = false;
}
void NetServer::Disconnect(Player * player)
{
	STAR_DESTROY_PACKET packet;
	packet.type = PACKET_TYPE::DESTROY;
	packet.id = player->star.id;
	SendBroadcast(player, (char*)&packet, sizeof(STAR_DESTROY_PACKET));
	closesocket(player->socket);
	Jay::WriteLog("NetServer::Disconnect() Disconnect IP: '%s', Port: '%d'\n", player->ip, player->port);
}
void NetServer::DestroyAll()
{
	for (auto iter = _playerList.begin(); iter != _playerList.end();)
	{
		Player *player = *iter;
		closesocket(player->socket);
		delete player;
		iter = _playerList.erase(iter);
	}
	closesocket(_listenSocket);
}
