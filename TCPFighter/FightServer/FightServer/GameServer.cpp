#include "stdafx.h"
#include "GameServer.h"
#include "define.h"
#include "Util.h"
#include "Packet.h"
#include <mstcpip.h>
#include <ws2tcpip.h>
#include "../Network/include/SerializationBuffer.h"
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../Network/lib/Release/Network.lib")

GameServer::GameServer() : _listenSocket(INVALID_SOCKET), _allocId(rand() % 1000)
{
	WSADATA ws;
	int status = WSAStartup(MAKEWORD(2, 2), &ws);
	if (status != 0)
	{
		printf("GameServer::GameServer() Failed WSAStartup: %d\n", status);
		CRASH;
	}
	Listen();
}
GameServer::~GameServer()
{
	DestroyAll();
	WSACleanup();
}
void GameServer::Network()
{
	fd_set rfds;
	fd_set sfds;
	FD_ZERO(&rfds);
	FD_ZERO(&sfds);
	FD_SET(_listenSocket, &rfds);
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (player->sendQ.GetUseSize() > 0)
			FD_SET(player->socket, &sfds);
		FD_SET(player->socket, &rfds);
	}

	timeval tv = { 0, 0 };
	int result = select(NULL, &rfds, &sfds, NULL, &tv);
	if (result == SOCKET_ERROR)
	{
		char msg[64];	
		sprintf_s(msg, "GameServer::Network() Failed select: %d", WSAGetLastError());
		MessageBoxA(NULL, msg, dfSERVER_NAME, MB_ICONERROR);
		CRASH;
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
void GameServer::Update()
{
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (!player->enable)
			continue;

		if (player->hp == 0)
		{
			Disable(player);
			continue;
		}

		unsigned short dx = dfPLAYER_SPEED_X;
		unsigned short dy = dfPLAYER_SPEED_Y;
		switch (player->action)
		{
		case dfPACKET_MOVE_DIR_LL:
			if (player->x - dx > dfRANGE_MOVE_LEFT)
				player->x -= dx;
			break;
		case dfPACKET_MOVE_DIR_LU:
			if (player->x - dx > dfRANGE_MOVE_LEFT)
				player->x -= dx;
			if (player->y - dy > dfRANGE_MOVE_TOP)
				player->y -= dy;
			break;
		case dfPACKET_MOVE_DIR_UU:
			if (player->y - dy > dfRANGE_MOVE_TOP)
				player->y -= dy;
			break;
		case dfPACKET_MOVE_DIR_RU:
			if (player->x + dx < dfRANGE_MOVE_RIGHT)
				player->x += dx;
			if (player->y - dy > dfRANGE_MOVE_TOP)
				player->y -= dy;
			break;
		case dfPACKET_MOVE_DIR_RR:
			if (player->x + dx < dfRANGE_MOVE_RIGHT)
				player->x += dx;
			break;
		case dfPACKET_MOVE_DIR_RD:
			if (player->x + dx < dfRANGE_MOVE_RIGHT)
				player->x += dx;
			if (player->y + dy < dfRANGE_MOVE_BOTTOM)
				player->y += dy;
			break;
		case dfPACKET_MOVE_DIR_DD:
			if (player->y + dy < dfRANGE_MOVE_BOTTOM)
				player->y += dy;
			break;
		case dfPACKET_MOVE_DIR_LD:
			if (player->x - dx > dfRANGE_MOVE_LEFT)
				player->x -= dx;
			if (player->y + dy < dfRANGE_MOVE_BOTTOM)
				player->y += dy;
			break;
		default:
			break;
		}
	}
}
void GameServer::Cleanup()
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
void GameServer::Listen()
{
	_listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listenSocket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		printf("GameServer::Listen() Failed socket: %d\n", err);
		return;
	}

	linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	int result = setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		printf("GameServer::Listen() Failed setsockopt linger: %d\n", err);
		closesocket(_listenSocket);
		return;
	}

	u_long nonBlockingMode = 1;
	result = ioctlsocket(_listenSocket, FIONBIO, &nonBlockingMode);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf(L"GameServer::Listen() Failed ioctlsocket: %d\n", err);
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
		printf("GameServer::Listen() Failed WSAIoctl keepalive: %d\n", err);
		closesocket(_listenSocket);
		return;
	}

	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(dfSERVER_PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(_listenSocket, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		printf("GameServer::Listen() Failed bind: %d\n", err);
		closesocket(_listenSocket);
		return;
	}

	result = listen(_listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		printf("GameServer::Listen() Failed listen: %d\n", err);
		closesocket(_listenSocket);
		return;
	}
	printf_s("Listen Success Port: %d\n", dfSERVER_PORT);
}
void GameServer::AcceptProc()
{
	SOCKADDR_IN clientAddr = {};
	int clientSize = sizeof(clientAddr);
	SOCKET client = accept(_listenSocket, (SOCKADDR*)&clientAddr, &clientSize);
	if (client == INVALID_SOCKET)
	{
		char msg[64];
		sprintf_s(msg, "GameServer::Accept() Failed accept: %d", WSAGetLastError());
		MessageBoxA(NULL, msg, dfSERVER_NAME, MB_ICONERROR);
		CRASH;
	}

	if (_playerList.size() >= dfMAX_PLAYER)
	{
		closesocket(client);
		return;
	}

	Player *new_player = new Player;
	new_player->enable = true;
	new_player->socket = client;
	InetNtopA(AF_INET, &clientAddr.sin_addr, new_player->ip, sizeof(new_player->ip));
	new_player->port = ntohs(clientAddr.sin_port);
	new_player->action = -1;
	new_player->id = _allocId++;
	new_player->direction = dfPACKET_MOVE_DIR_LL;
	new_player->hp = dfPLAYER_HP;
	new_player->x = rangeRand(dfRANGE_MOVE_LEFT + 1, dfRANGE_MOVE_RIGHT - 1);
	new_player->y = rangeRand(dfRANGE_MOVE_TOP + 1, dfRANGE_MOVE_BOTTOM - 1);
	printf("Connected IP: %s, Port: %d, ID: %d\n", new_player->ip, new_player->port, new_player->id);

	Jay::SerializationBuffer my_character_packet;
	Packet::MakeCreateMyCharacter(&my_character_packet, new_player->id, new_player->direction, new_player->x, new_player->y, new_player->hp);
	SendUnicast(new_player, &my_character_packet);

	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		Jay::SerializationBuffer other_character_packet;
		Packet::MakeCreateOtherCharacter(&other_character_packet, player->id, player->direction, player->x, player->y, player->hp);
		SendUnicast(new_player, &other_character_packet);

		switch (player->action)
		{
		case dfPACKET_MOVE_DIR_LL:
		case dfPACKET_MOVE_DIR_LU:
		case dfPACKET_MOVE_DIR_UU:
		case dfPACKET_MOVE_DIR_RU:
		case dfPACKET_MOVE_DIR_RR:
		case dfPACKET_MOVE_DIR_RD:
		case dfPACKET_MOVE_DIR_DD:
		case dfPACKET_MOVE_DIR_LD:
			{
				Jay::SerializationBuffer move_start_packet;
				Packet::MakeMoveStart(&move_start_packet, player->id, (char)player->action, player->x, player->y);
				SendUnicast(new_player, &move_start_packet);
			}
			break;
		default:
			break;
		}
	}
	_playerList.push_back(new_player);
	
	Jay::SerializationBuffer other_character_packet;
	Packet::MakeCreateOtherCharacter(&other_character_packet, new_player->id, new_player->direction, new_player->x, new_player->y, new_player->hp);
	SendBroadcast(new_player, &other_character_packet);
}
void GameServer::RecvProc(Player* player)
{
	if (!player->enable)
		return;

	int err;
	char buffer[512];
	int size = recv(player->socket, buffer, sizeof(buffer), 0);
	switch (size)
	{
	case SOCKET_ERROR:
		err = WSAGetLastError();
		if (err != WSAECONNRESET && err != WSAECONNABORTED)
			printf("GameServer::RecvProc() Failed recv: %d\n", err);
	case 0:
		Disable(player);
		return;
	default:
		break;
	}
	
	printf("Recv size: %d\n", size);
	int ret = player->recvQ.Enqueue(buffer, size);
	if (ret != size)
	{
		printf("GameServer::RecvProc() Failed recvQ::Enqueue() [size : %d]\n", ret);
		Disable(player);
		return;
	}
	printf("RecvQ enqueue: %d\n", ret);

	for (;;)
	{
		PACKET_HEADER header;
		int headerSize = sizeof(header);
		if (player->recvQ.GetUseSize() <= headerSize)
			break;
		
		ret = player->recvQ.Peek((char*)&header, headerSize);
		if (ret != headerSize)
		{
			printf("GameServer::RecvProc() Failed recvQ::Peek() [size : %d]\n", ret);
			CRASH;
		}
		
		if (header.byCode != dfPACKET_CODE)
		{
			Disable(player);
			break;
		}

		if (player->recvQ.GetUseSize() < headerSize + header.bySize)
			break;

		Jay::SerializationBuffer packet;
		player->recvQ.MoveFront(headerSize);
		ret = player->recvQ.Dequeue(packet.GetBufferPtr(), header.bySize);
		if (ret != header.bySize)
		{
			printf("GameServer::RecvProc() Failed recvQ::Dequeue() [size : %d]\n", ret);
			CRASH;
		}
		packet.MoveRear(ret);
		TranslatePacket(player, &packet, header.byType);
	}
}
void GameServer::SendProc(Player * player)
{
	if (!player->enable)
		return;
	
	char message[1024];
	int size = player->sendQ.Peek(message, sizeof(message));
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
		case WSAECONNABORTED:
		case WSAECONNRESET:
			break;
		default:
			printf("GameServer::SendProc() Failed send error: %d, IP: '%s', Port: '%d'\n", err, player->ip, player->port);
			break;
		}
		Disable(player);
		return;
	}
	player->sendQ.MoveFront(ret);
	printf("Send size: %d\n", ret);
}
void GameServer::SendUnicast(Player * target, Jay::SerializationBuffer* sc_packet)
{
	int size = sc_packet->GetUseSize();
	printf("SendQ enqueue: %d\n", size);
	int ret = target->sendQ.Enqueue(sc_packet->GetBufferPtr(), size);
	if (ret != size)
	{
		printf("GameServer::SendUnicast() Failed sendQ::Enqueue() [size : %d]\n", ret);
		Disable(target);
		return;
	}
}
void GameServer::SendBroadcast(Player * exclusion, Jay::SerializationBuffer* sc_packet)
{
	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player *player = *iter;
		if (player != exclusion)
			SendUnicast(player, sc_packet);
	}
}
void GameServer::Disable(Player * player)
{
	player->enable = false;
}
void GameServer::Disconnect(Player * player)
{
	printf("Disconnect IP: %s, Port: %d, ID: %d\n", player->ip, player->port, player->id);
	Jay::SerializationBuffer sc_packet;
	Packet::MakeDeleteCharacter(&sc_packet, player->id);
	SendBroadcast(player, &sc_packet);
	closesocket(player->socket);
}
void GameServer::DestroyAll()
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
void GameServer::TranslatePacket(Player * player, Jay::SerializationBuffer* cs_packet, int type)
{
	switch (type)
	{
	case dfPACKET_CS_MOVE_START:
		PacketProc_MoveStart(player, cs_packet);
		break;
	case dfPACKET_CS_MOVE_STOP:
		PacketProc_MoveStop(player, cs_packet);
		break;
	case dfPACKET_CS_ATTACK1:
		PacketProc_Attack1(player, cs_packet);
		break;
	case dfPACKET_CS_ATTACK2:
		PacketProc_Attack2(player, cs_packet);
		break;
	case dfPACKET_CS_ATTACK3:
		PacketProc_Attack3(player, cs_packet);
		break;
	case dfPACKET_CS_SYNC:
		PacketProc_Sync(player, cs_packet);
		break;
	default:
		printf("GameServer::TranslatePacket() UNKNOWN_PACKET\n");
		Disable(player);
		break;
	}
}
void GameServer::PacketProc_MoveStart(Player * player, Jay::SerializationBuffer* cs_packet)
{
	printf_s("MoveStart before ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);
	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;
	switch (direction)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		player->direction = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		player->direction = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
	player->x = x;
	player->y = y;
	player->action = direction;
	printf_s("MoveStart after ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);

	Jay::SerializationBuffer sc_packet;
	Packet::MakeMoveStart(&sc_packet, player->id, (char)player->action, player->x, player->y);
	SendBroadcast(player, &sc_packet);
}
void GameServer::PacketProc_MoveStop(Player * player, Jay::SerializationBuffer* cs_packet)
{
	printf_s("MoveStop before ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);
	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;
	if (abs(x - player->x) >= dfERROR_RANGE || abs(y - player->y) >= dfERROR_RANGE)
	{
		Disable(player);
		return;
	}
	switch (direction)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		player->direction = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		player->direction = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
	player->x = x;
	player->y = y;
	player->action = -1;
	printf_s("MoveStop after ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);

	Jay::SerializationBuffer sc_packet;
	Packet::MakeMoveStop(&sc_packet, player->id, (char)player->direction, player->x, player->y);
	SendBroadcast(player, &sc_packet);
}
void GameServer::PacketProc_Attack1(Player * player, Jay::SerializationBuffer* cs_packet)
{
	printf_s("Attack1 ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);
	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;
	Jay::SerializationBuffer attack_packet;
	Packet::MakeAttack1(&attack_packet, player->id, player->direction, player->x, player->y);
	SendBroadcast(player, &attack_packet);

	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player* target = *iter;
		if (player->id == target->id)
			continue;

		if ((player->direction == dfPACKET_MOVE_DIR_LL && target->x > player->x - dfATTACK1_RANGE_X && target->x < player->x) ||
			(player->direction == dfPACKET_MOVE_DIR_RR && target->x > player->x && target->x < player->x + dfATTACK1_RANGE_X))
		{
			if (target->y < player->y - dfATTACK1_RANGE_Y || target->y > player->y + dfATTACK1_RANGE_Y)
				continue;
			
			target->hp -= (target->hp > dfATTACK1_DAMAGE) ? dfATTACK1_DAMAGE : target->hp;
			Jay::SerializationBuffer damage_packet;
			Packet::MakeDamage(&damage_packet, player->id, target->id, target->hp);
			SendBroadcast(nullptr, &damage_packet);
		}
	}
}
void GameServer::PacketProc_Attack2(Player * player, Jay::SerializationBuffer* cs_packet)
{
	printf_s("Attack2 ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);
	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;
	Jay::SerializationBuffer attack_packet;
	Packet::MakeAttack2(&attack_packet, player->id, player->direction, player->x, player->y);
	SendBroadcast(player, &attack_packet);

	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player* target = *iter;
		if (player->id == target->id)
			continue;

		if ((player->direction == dfPACKET_MOVE_DIR_LL && target->x > player->x - dfATTACK2_RANGE_X && target->x < player->x) ||
			(player->direction == dfPACKET_MOVE_DIR_RR && target->x > player->x && target->x < player->x + dfATTACK2_RANGE_X))
		{
			if (target->y < player->y - dfATTACK2_RANGE_Y || target->y > player->y + dfATTACK2_RANGE_Y)
				continue;

			target->hp -= (target->hp > dfATTACK2_DAMAGE) ? dfATTACK2_DAMAGE : target->hp;
			Jay::SerializationBuffer damage_packet;
			Packet::MakeDamage(&damage_packet, player->id, target->id, target->hp);
			SendBroadcast(nullptr, &damage_packet);
		}
	}
}
void GameServer::PacketProc_Attack3(Player * player, Jay::SerializationBuffer* cs_packet)
{
	printf_s("Attack3 ID: %d / Dir: %d / X: %d / Y: %d\n", player->id, player->direction, player->x, player->y);
	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;
	Jay::SerializationBuffer attack_packet;
	Packet::MakeAttack3(&attack_packet, player->id, player->direction, player->x, player->y);
	SendBroadcast(player, &attack_packet);

	for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
	{
		Player* target = *iter;
		if (player->id == target->id)
			continue;

		if ((player->direction == dfPACKET_MOVE_DIR_LL && target->x > player->x - dfATTACK3_RANGE_X && target->x < player->x) ||
			(player->direction == dfPACKET_MOVE_DIR_RR && target->x > player->x && target->x < player->x + dfATTACK3_RANGE_X))
		{
			if (target->y < player->y - dfATTACK3_RANGE_Y || target->y > player->y + dfATTACK3_RANGE_Y)
				continue;

			target->hp -= (target->hp > dfATTACK3_DAMAGE) ? dfATTACK3_DAMAGE : target->hp;
			Jay::SerializationBuffer damage_packet;
			Packet::MakeDamage(&damage_packet, player->id, target->id, target->hp);
			SendBroadcast(nullptr, &damage_packet);
		}
	}
}
void GameServer::PacketProc_Sync(Player * player, Jay::SerializationBuffer* cs_packet)
{
	unsigned short x;
	unsigned short y;
	*cs_packet >> x >> y;
	player->x = x;
	player->y = y;

	Jay::SerializationBuffer sc_packet;
	Packet::MakeSync(&sc_packet, player->id, player->x, player->y);
	SendBroadcast(player, &sc_packet);
}
