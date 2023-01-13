#include "stdafx.h"
#include "GameServer.h"
#include "define.h"
#include "Packet.h"
#include <mstcpip.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../Lib/Network/lib/Network.lib")

GameServer::GameServer() : _listenSocket(INVALID_SOCKET), _keySessionID(0), _sessionPool(dfMAX_USER), _characterPool(dfMAX_USER), _packetPool(0), _syncErrorCount(0)
{
	WSADATA ws;
	int status = WSAStartup(MAKEWORD(2, 2), &ws);
	if (status != 0)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed WSAStartup: %d", __FUNCTIONW__, status);
		CRASH;
	}
}
GameServer::~GameServer()
{
	DestroyAll();
	WSACleanup();
}
void GameServer::Network()
{
	SOCKET userSockTable[FD_SETSIZE];
	fd_set rfds;
	fd_set sfds;
	FD_ZERO(&rfds);
	FD_ZERO(&sfds);
	memset(userSockTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

	int socketCount = 0;
	userSockTable[socketCount++] = _listenSocket;
	FD_SET(_listenSocket, &rfds);

	for (auto iter = _sessionMap.begin(); iter != _sessionMap.end(); ++iter)
	{
		SESSION *session = iter->second;
		userSockTable[socketCount++] = session->socket;

		if (session->sendQ.GetUseSize() > 0)
			FD_SET(session->socket, &sfds);
		FD_SET(session->socket, &rfds);

		if (socketCount >= FD_SETSIZE)
		{
			SelectSocket(userSockTable, &rfds, &sfds);
			FD_ZERO(&rfds);
			FD_ZERO(&sfds);
			memset(userSockTable, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			socketCount = 0;
			userSockTable[socketCount++] = _listenSocket;
			FD_SET(_listenSocket, &rfds);
		}
	}
	if (socketCount > 0)
		SelectSocket(userSockTable, &rfds, &sfds);
}
void GameServer::Update()
{
	DWORD currentTime = timeGetTime();
	for (auto iter = _characterMap.begin(); iter != _characterMap.end(); ++iter)
	{
		CHARACTER *character = iter->second;
		SESSION *session = character->session;
		if (!session->enable)
			continue;

		if (character->hp == 0)
		{
			Disable(session);
			continue;
		}

		if (currentTime - session->lastRecvTime > dfNETWORK_PACKET_RECV_TIMEOUT)
		{
			Disable(session);
			continue;
		}

		unsigned short dx = dfSPEED_PLAYER_X;
		unsigned short dy = dfSPEED_PLAYER_Y;
		switch (character->action)
		{
		case dfPACKET_MOVE_DIR_LL:
			if (IsMovableCharacter(character->x - dx, character->y))
				character->x -= dx;
			break;
		case dfPACKET_MOVE_DIR_LU:
			if (IsMovableCharacter(character->x - dx, character->y - dy))
			{
				character->x -= dx;
				character->y -= dy;
			}
			break;
		case dfPACKET_MOVE_DIR_UU:
			if (IsMovableCharacter(character->x, character->y - dy))
				character->y -= dy;
			break;
		case dfPACKET_MOVE_DIR_RU:
			if (IsMovableCharacter(character->x + dx, character->y - dy))
			{
				character->x += dx;
				character->y -= dy;
			}
			break;
		case dfPACKET_MOVE_DIR_RR:
			if (IsMovableCharacter(character->x + dx, character->y))
				character->x += dx;
			break;
		case dfPACKET_MOVE_DIR_RD:
			if (IsMovableCharacter(character->x + dx, character->y + dy))
			{
				character->x += dx;
				character->y += dy;
			}
			break;
		case dfPACKET_MOVE_DIR_DD:
			if (IsMovableCharacter(character->x, character->y + dy))
				character->y += dy;
			break;
		case dfPACKET_MOVE_DIR_LD:
			if (IsMovableCharacter(character->x - dx, character->y + dy))
			{
				character->x -= dx;
				character->y += dy;
			}
			break;
		default:
			break;
		}

		if (character->action >= dfPACKET_MOVE_DIR_LL &&
			character->action <= dfPACKET_MOVE_DIR_LD)
		{
			if (UpdateCharacter_Sector(character))
				UpdatePacket_Sector(character);
		}
	}
}
void GameServer::Cleanup()
{
	while (_gcQueue.size() > 0)
	{
		SESSION* session = _gcQueue.front();
		DisconnectSession(session->socket);
		_gcQueue.pop();
	}
}
void GameServer::Listen(int port)
{
	_listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listenSocket == INVALID_SOCKET)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed socket: %d", __FUNCTIONW__, WSAGetLastError());
		return;
	}

	linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	int result = setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (result == SOCKET_ERROR)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed setsockopt linger: %d", __FUNCTIONW__, WSAGetLastError());
		closesocket(_listenSocket);
		return;
	}

	u_long nonBlockingMode = 1;
	result = ioctlsocket(_listenSocket, FIONBIO, &nonBlockingMode);
	if (result == SOCKET_ERROR)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed ioctlsocket: %d", __FUNCTIONW__, WSAGetLastError());
		closesocket(_listenSocket);
		return;
	}
	
	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(port);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(_listenSocket, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	if (result == SOCKET_ERROR)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed bind: %d", __FUNCTIONW__, WSAGetLastError());
		closesocket(_listenSocket);
		return;
	}
	
	result = listen(_listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed listen: %d", __FUNCTIONW__, WSAGetLastError());
		closesocket(_listenSocket);
		return;
	}
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_SYSTEM, L"%s() Listen port: %d", __FUNCTIONW__, port);
}
void GameServer::SelectSocket(SOCKET * userSockTable, FD_SET * readset, FD_SET * writeset)
{
	timeval tv = { 0, 0 };
	int ret = select(NULL, readset, writeset, NULL, &tv);
	if (ret > 0)
	{
		for (int i = 0; ret > 0 && i < FD_SETSIZE; i++)
		{
			if (userSockTable[i] == INVALID_SOCKET)
				continue;

			if (FD_ISSET(userSockTable[i], writeset))
			{
				ret--;
				SendProc(userSockTable[i]);
			}

			if (FD_ISSET(userSockTable[i], readset))
			{
				ret--;
				if (userSockTable[i] == _listenSocket)
					AcceptProc();
				else
					RecvProc(userSockTable[i]);
			}
		}
	}
	else if (ret == SOCKET_ERROR)
	{
		wchar_t msg[64];
		swprintf_s(msg, L"%s() Failed select: %d", __FUNCTIONW__, WSAGetLastError());
		MessageBox(NULL, msg, dfSERVER_NAME, MB_ICONERROR);
		CRASH;
	}
}
void GameServer::AcceptProc()
{
	SOCKADDR_IN clientAddr = {};
	int clientSize = sizeof(clientAddr);
	SOCKET client = accept(_listenSocket, (SOCKADDR*)&clientAddr, &clientSize);
	if (client == INVALID_SOCKET)
	{
		wchar_t msg[64];
		swprintf_s(msg, L"%s() Failed accept: %d", __FUNCTIONW__, WSAGetLastError());
		MessageBox(NULL, msg, dfSERVER_NAME, MB_ICONERROR);
		CRASH;
	}

	if (_sessionMap.size() >= dfMAX_USER)
	{
		closesocket(client);
		return;
	}

	CreateSession(client, &clientAddr);
}
void GameServer::RecvProc(SOCKET socket)
{
	auto iter = _sessionMap.find(socket);
	SESSION* session = iter->second;
	if (!session->enable)
		return;

	int size = session->recvQ.DirectEnqueueSize();
	if (size <= 0)
	{
		Disable(session);
		return;
	}
	session->lastRecvTime = timeGetTime();
		
	int err;
	int len = recv(session->socket, session->recvQ.GetRearBufferPtr(), size, 0);
	switch (len)
	{
	case SOCKET_ERROR:
		err = WSAGetLastError();
		if (err != WSAECONNRESET && err != WSAECONNABORTED)
			Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed recv - sessionID: %d, error: %d", __FUNCTIONW__, session->sessionID, err);
	case 0:
		Disable(session);
		return;
	default:
		break;
	}
	session->recvQ.MoveRear(len);
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() recv - sessionID: %d, size: %d", __FUNCTIONW__, session->sessionID, len);
	
	for (;;)
	{
		int ret = CompleteRecvPacket(session);
		if (ret == 1)
			break;
		else if (ret == -1)
		{
			Disable(session);
			break;
		}
	}
}
int GameServer::CompleteRecvPacket(SESSION * session)
{
	PACKET_HEADER header;
	int headerSize = sizeof(PACKET_HEADER);
	if (session->recvQ.GetUseSize() <= headerSize)
		return 1;

	int ret = session->recvQ.Peek((char*)&header, headerSize);
	if (ret != headerSize)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed recvQ peek - sessionID: %d, size: %d, ret: %d", __FUNCTIONW__, session->sessionID, headerSize, ret);
		CRASH;
	}

	if (header.byCode != dfPACKET_CODE)
	{
		_unknownPacketCount++;
		return -1;
	}

	if (session->recvQ.GetUseSize() < headerSize + header.bySize)
		return 1;

	session->recvQ.MoveFront(headerSize);

	Jay::SerializationBuffer* packet = _packetPool.Alloc();
	packet->ClearBuffer();
	ret = session->recvQ.Dequeue(packet->GetBufferPtr(), header.bySize);
	if (ret != header.bySize)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed recvQ dequeue - sessionID: %d, size: %d, ret: %d", __FUNCTIONW__, session->sessionID, header.bySize, ret);
		CRASH;
	}
	packet->MoveRear(ret);
	
	if (!PacketProc(session, packet, header.byType))
	{
		_packetPool.Free(packet);
		return -1;
	}
	_packetPool.Free(packet);

	return 0;
}
void GameServer::SendProc(SOCKET socket)
{
	auto iter = _sessionMap.find(socket);
	SESSION* session = iter->second;
	if (!session->enable)
		return;
	
	int size = session->sendQ.DirectDequeueSize();
	int ret = send(session->socket, session->sendQ.GetFrontBufferPtr(), size, 0);
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
			Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed send - sessionID: %d, error: %d", __FUNCTIONW__, session->sessionID, err);
			break;
		}
		Disable(session);
		return;
	}
	session->sendQ.MoveFront(ret);
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() send - sessionID: %d, size: %d", __FUNCTIONW__, session->sessionID, ret);
}
void GameServer::SendUnicast(SESSION * session, Jay::SerializationBuffer* sc_packet)
{
	int size = sc_packet->GetUseSize();
	int ret = session->sendQ.Enqueue(sc_packet->GetBufferPtr(), size);
	if (ret != size)
	{
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() Failed sendQ enqueue - sessionID: %d, size: %d, ret: %d", __FUNCTIONW__, session->sessionID, size, ret);
		Disable(session);
		return;
	}
}
void GameServer::SendBroadcast(SESSION * exclusion, Jay::SerializationBuffer* sc_packet)
{
	for (auto iter = _sessionMap.begin(); iter != _sessionMap.end(); ++iter)
	{
		SESSION *session = iter->second;
		if (session != exclusion)
			SendUnicast(session, sc_packet);
	}
}
void GameServer::SendSectorOne(SESSION * exclusion, Jay::SerializationBuffer * sc_packet, int sectorX, int sectorY)
{
	auto sectorList = &_sector[sectorY][sectorX];
	for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
	{
		SESSION* session = (*iter)->session;
		if (session != exclusion)
			SendUnicast(session, sc_packet);
	}
}
void GameServer::SendSectorAround(SESSION * session, Jay::SerializationBuffer * sc_packet, bool sendMe)
{
	auto iter = _characterMap.find(session->socket);
	CHARACTER *character = iter->second;
	SESSION *exclusion = (sendMe == false) ? session : nullptr;

	SECTOR_AROUND sectorAround;
	GetSectorAround(character->curSector.x, character->curSector.y, &sectorAround);
	for (int i = 0; i < sectorAround.count; i++)
	{
		SendSectorOne(exclusion, sc_packet, sectorAround.around[i].x, sectorAround.around[i].y);
	}
}
void GameServer::Disable(SESSION * session)
{
	if (session->enable)
	{
		session->enable = false;
		_gcQueue.push(session);
	}
}
void GameServer::DestroyAll()
{
	for (auto iter = _sessionMap.begin(); iter != _sessionMap.end();)
	{
		SESSION *session = iter->second;
		CHARACTER *character = _characterMap.find(session->socket)->second;

		RemoveCharacter_Sector(character);
		_characterMap.erase(session->socket);
		_characterPool.Free(character);

		closesocket(session->socket);
		_sessionPool.Free(session);
		iter = _sessionMap.erase(iter);
	}
	closesocket(_listenSocket);
}
SESSION * GameServer::CreateSession(SOCKET socket, SOCKADDR_IN * socketAddr)
{
	SESSION *new_session = _sessionPool.Alloc();
	new_session->enable = true;
	new_session->socket = socket;
	InetNtop(AF_INET, &socketAddr->sin_addr, new_session->ip, sizeof(new_session->ip) / 2);
	new_session->port = ntohs(socketAddr->sin_port);
	new_session->sessionID = ++_keySessionID;
	new_session->lastRecvTime = timeGetTime();
	new_session->recvQ.ClearBuffer();
	new_session->sendQ.ClearBuffer();
	_sessionMap.insert({ new_session->socket, new_session });
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() Connected IP: %s, Port: %d, ID: %d", __FUNCTIONW__
		, new_session->ip, new_session->port, new_session->sessionID);
	CreateCharacter(new_session);
	return new_session;
}
void GameServer::DisconnectSession(SOCKET socket)
{
	auto iter = _characterMap.find(socket);
	CHARACTER* character = iter->second;
	SESSION* session = character->session;
	DestroyCharacter(character);
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() Disconnect - sessionID: %d, IP: %s, Port: %d", __FUNCTIONW__
		, session->sessionID, session->ip, session->port);
	_sessionMap.erase(session->socket);
	closesocket(session->socket);
	_sessionPool.Free(session);
}
CHARACTER * GameServer::CreateCharacter(SESSION * session)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	CHARACTER *new_character = _characterPool.Alloc();

	new_character->session = session;
	new_character->sessionID = session->sessionID;
	new_character->action = -1;
	new_character->direction = dfPACKET_MOVE_DIR_LL;
	new_character->hp = dfCHARACTER_HP;
	new_character->x = rand() % dfRANGE_MOVE_RIGHT;
	new_character->y = rand() % dfRANGE_MOVE_BOTTOM;
	AddCharacter_Sector(new_character);
	_characterMap.insert({ session->socket, new_character });

	Packet::MakeCreateMyCharacter(sc_packet
		, new_character->sessionID
		, new_character->direction
		, new_character->x
		, new_character->y
		, new_character->hp);
	SendUnicast(session, sc_packet);

	SECTOR_AROUND sectorAround;
	GetSectorAround(new_character->curSector.x, new_character->curSector.y, &sectorAround);
	for (int i = 0; i < sectorAround.count; i++)
	{
		auto sectorList = &_sector[sectorAround.around[i].y][sectorAround.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
		{
			CHARACTER* existCharacter = *iter;
			if (existCharacter == new_character)
				continue;

			Packet::MakeCreateOtherCharacter(sc_packet
				, existCharacter->sessionID
				, existCharacter->direction
				, existCharacter->x
				, existCharacter->y
				, existCharacter->hp);
			SendUnicast(new_character->session, sc_packet);

			switch (existCharacter->action)
			{
			case dfPACKET_MOVE_DIR_LL:
			case dfPACKET_MOVE_DIR_LU:
			case dfPACKET_MOVE_DIR_UU:
			case dfPACKET_MOVE_DIR_RU:
			case dfPACKET_MOVE_DIR_RR:
			case dfPACKET_MOVE_DIR_RD:
			case dfPACKET_MOVE_DIR_DD:
			case dfPACKET_MOVE_DIR_LD:
				Packet::MakeMoveStart(sc_packet
					, existCharacter->sessionID
					, existCharacter->action
					, existCharacter->x
					, existCharacter->y);
				SendUnicast(new_character->session, sc_packet);
				break;
			default:
				break;
			}
		}
	}

	Packet::MakeCreateOtherCharacter(sc_packet
		, new_character->sessionID
		, new_character->direction
		, new_character->x
		, new_character->y
		, new_character->hp);
	SendSectorAround(session, sc_packet, false);

	_packetPool.Free(sc_packet);
	return new_character;
}
void GameServer::DestroyCharacter(CHARACTER * character)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	Packet::MakeDeleteCharacter(sc_packet, character->sessionID);
	SendSectorAround(character->session, sc_packet, false);
	RemoveCharacter_Sector(character);
	_characterMap.erase(character->session->socket);
	_characterPool.Free(character);
	_packetPool.Free(sc_packet);
}
bool GameServer::PacketProc(SESSION * session, Jay::SerializationBuffer* cs_packet, WORD type)
{
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() PacketProc [%d] - sessionID: %d", __FUNCTIONW__, type, session->sessionID);
	switch (type)
	{
	case dfPACKET_CS_MOVE_START:
		return PacketProc_MoveStart(session, cs_packet);
	case dfPACKET_CS_MOVE_STOP:
		return PacketProc_MoveStop(session, cs_packet);
	case dfPACKET_CS_ATTACK1:
		return PacketProc_Attack1(session, cs_packet);
	case dfPACKET_CS_ATTACK2:
		return PacketProc_Attack2(session, cs_packet);
	case dfPACKET_CS_ATTACK3:
		return PacketProc_Attack3(session, cs_packet);
	case dfPACKET_CS_SYNC:
		return PacketProc_Sync(session, cs_packet);
	case dfPACKET_CS_ECHO:
		return PacketProc_Echo(session, cs_packet);
	default:
		_unknownPacketCount++;
		break;
	}
	return false;
}
void GameServer::AddCharacter_Sector(CHARACTER * character)
{
	character->curSector.x = character->x / dfSECTOR_SIZE_X;
	character->curSector.y = character->y / dfSECTOR_SIZE_Y;
	_sector[character->curSector.y][character->curSector.x].push_back(character);
}
void GameServer::RemoveCharacter_Sector(CHARACTER * character)
{
	_sector[character->curSector.y][character->curSector.x].remove(character);
	character->oldSector.x = character->curSector.x;
	character->oldSector.y = character->curSector.y;
}
bool GameServer::UpdateCharacter_Sector(CHARACTER * character)
{
	int sectorX = character->x / dfSECTOR_SIZE_X;
	int sectorY = character->y / dfSECTOR_SIZE_Y;
	if (character->curSector.x == sectorX && character->curSector.y == sectorY)
		return false;
	
	RemoveCharacter_Sector(character);
	AddCharacter_Sector(character);
	return true;
}
void GameServer::GetSectorAround(int sectorX, int sectorY, SECTOR_AROUND * sectorAround)
{
	sectorX--;
	sectorY--;

	sectorAround->count = 0;
	for (int y = 0; y < 3; y++)
	{
		if (sectorY + y < 0 || sectorY + y >= dfSECTOR_MAX_Y)
			continue;

		for (int x = 0; x < 3; x++)
		{
			if (sectorX + x < 0 || sectorX + x >= dfSECTOR_MAX_X)
				continue;

			sectorAround->around[sectorAround->count].x = sectorX + x;
			sectorAround->around[sectorAround->count].y = sectorY + y;
			sectorAround->count++;
		}
	}
}
void GameServer::GetUpdateSectorAround(CHARACTER * character, SECTOR_AROUND * removeSector, SECTOR_AROUND * addSector)
{
	SECTOR_AROUND oldSectorAround, curSectorAround;
	GetSectorAround(character->oldSector.x, character->oldSector.y, &oldSectorAround);
	GetSectorAround(character->curSector.x, character->curSector.y, &curSectorAround);

	// 이전섹터(oldSector) 정보 중 현재섹터(curSector) 에는 없는 정보를 찾아서 removeSector에 넣는다.
	removeSector->count = 0;
	for (int old = 0; old < oldSectorAround.count; old++)
	{
		bool find = false;
		for (int cur = 0; cur < curSectorAround.count; cur++)
		{
			if (oldSectorAround.around[old].x == curSectorAround.around[cur].x &&
				oldSectorAround.around[old].y == curSectorAround.around[cur].y)
			{
				find = true;
				break;
			}
		}
		if (!find)
		{
			removeSector->around[removeSector->count].x = oldSectorAround.around[old].x;
			removeSector->around[removeSector->count].y = oldSectorAround.around[old].y;
			removeSector->count++;
		}
	}

	// 현재섹터(curSector) 정보 중 이전섹터(oldSector) 에는 없는 정보를 찾아서 addSector에 넣는다.
	addSector->count = 0;
	for (int cur = 0; cur < curSectorAround.count; cur++)
	{
		bool find = false;
		for (int old = 0; old < oldSectorAround.count; old++)
		{
			if (oldSectorAround.around[old].x == curSectorAround.around[cur].x &&
				oldSectorAround.around[old].y == curSectorAround.around[cur].y)
			{
				find = true;
				break;
			}
		}
		if (!find)
		{
			addSector->around[addSector->count].x = curSectorAround.around[cur].x;
			addSector->around[addSector->count].y = curSectorAround.around[cur].y;
			addSector->count++;
		}
	}
}
void GameServer::UpdatePacket_Sector(CHARACTER * character)
{
	//--------------------------------------------------------------------------------------
	// 1. 이전섹터에 존재하는 캐릭터들에게 - 이동하는 캐릭터의 삭제 메시지
	// 2. 이동하는 캐릭터에게 - 이전섹터에 존재하는 캐릭터들의 삭제 메시지
	// 3. 현재섹터에 존재하는 캐릭터들에게 - 이동하는 캐릭터의 생성 및 이동 메시지
	// 4. 이동하는 캐릭터에게 - 현재섹터에 존재하는 캐릭터들의 생성 및 이동 메시지
	//--------------------------------------------------------------------------------------
	SECTOR_AROUND removeSector, addSector;
	GetUpdateSectorAround(character, &removeSector, &addSector);

	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();

	//--------------------------------------------------------------------------------------
	// 1. removeSector에 이동하는 캐릭터의 삭제 메시지 보내기
	//--------------------------------------------------------------------------------------
	Packet::MakeDeleteCharacter(sc_packet, character->sessionID);
	for (int i = 0; i < removeSector.count; i++)
	{
		SendSectorOne(character->session, sc_packet, removeSector.around[i].x, removeSector.around[i].y);
	}

	//--------------------------------------------------------------------------------------
	// 2. 이동하는 캐릭터에게 removeSector 캐릭터들의 삭제 메시지 보내기
	//--------------------------------------------------------------------------------------
	for (int i = 0; i < removeSector.count; i++)
	{
		auto *sectorList = &_sector[removeSector.around[i].y][removeSector.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
		{
			Packet::MakeDeleteCharacter(sc_packet, (*iter)->sessionID);
			SendUnicast(character->session, sc_packet);
		}
	}

	//--------------------------------------------------------------------------------------
	// 3-1. addSector에 이동하는 캐릭터의 생성 메시지 보내기
	//--------------------------------------------------------------------------------------
	Packet::MakeCreateOtherCharacter(sc_packet
		, character->sessionID
		, character->direction
		, character->x
		, character->y
		, character->hp);
	for (int i = 0; i < addSector.count; i++)
	{
		SendSectorOne(character->session, sc_packet, addSector.around[i].x, addSector.around[i].y);
	}

	//--------------------------------------------------------------------------------------
	// 3-2. addSector에 이동하는 캐릭터의 이동 메시지 보내기
	//--------------------------------------------------------------------------------------
	Packet::MakeMoveStart(sc_packet
		, character->sessionID
		, (char)character->action
		, character->x
		, character->y);
	for (int i = 0; i < addSector.count; i++)
	{
		SendSectorOne(character->session, sc_packet, addSector.around[i].x, addSector.around[i].y);
	}
	
	//--------------------------------------------------------------------------------------
	// 4. 이동하는 캐릭터에게 addSector 캐릭터들의 생성 및 이동 메시지 보내기
	//--------------------------------------------------------------------------------------
	for (int i = 0; i < addSector.count; i++)
	{
		auto sectorList = &_sector[addSector.around[i].y][addSector.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
		{
			CHARACTER* existCharacter = *iter;
			if (existCharacter == character)
				continue;
			
			// addSector 캐릭터의 생성 메시지 보내기
			Packet::MakeCreateOtherCharacter(sc_packet
				, existCharacter->sessionID
				, existCharacter->direction
				, existCharacter->x
				, existCharacter->y
				, existCharacter->hp);
			SendUnicast(character->session, sc_packet);

			// addSector 캐릭터가 이동하고 있었다면 이동 메시지 보내기
			switch (existCharacter->action)
			{
			case dfPACKET_MOVE_DIR_LL:
			case dfPACKET_MOVE_DIR_LU:
			case dfPACKET_MOVE_DIR_UU:
			case dfPACKET_MOVE_DIR_RU:
			case dfPACKET_MOVE_DIR_RR:
			case dfPACKET_MOVE_DIR_RD:
			case dfPACKET_MOVE_DIR_DD:
			case dfPACKET_MOVE_DIR_LD:
				Packet::MakeMoveStart(sc_packet
					, existCharacter->sessionID
					, existCharacter->action
					, existCharacter->x
					, existCharacter->y);
				SendUnicast(character->session, sc_packet);
				break;
			default:
				break;
			}
		} 
	}
	
	_packetPool.Free(sc_packet);
}
bool GameServer::IsMovableCharacter(int x, int y)
{
	if (x >= dfRANGE_MOVE_LEFT && x <= dfRANGE_MOVE_RIGHT &&  y >= dfRANGE_MOVE_TOP && y <= dfRANGE_MOVE_BOTTOM)
		return true;

	return false;
}
bool GameServer::CollisionCheck_Attack1(CHARACTER * attacker, CHARACTER * target)
{
	if (attacker->direction == dfPACKET_MOVE_DIR_LL && target->x < attacker->x && target->x >= attacker->x - dfATTACK1_RANGE_X)
	{
		if (abs(target->y - attacker->y) <= dfATTACK1_RANGE_Y)
			return true;
	}
	else if (attacker->direction == dfPACKET_MOVE_DIR_RR && target->x > attacker->x && target->x <= attacker->x + dfATTACK1_RANGE_X)
	{
		if (abs(target->y - attacker->y) <= dfATTACK1_RANGE_Y)
			return true;
	}
	return false;
}
bool GameServer::CollisionCheck_Attack2(CHARACTER * attacker, CHARACTER * target)
{
	if (attacker->direction == dfPACKET_MOVE_DIR_LL && target->x < attacker->x && target->x >= attacker->x - dfATTACK2_RANGE_X)
	{
		if (abs(target->y - attacker->y) <= dfATTACK2_RANGE_Y)
			return true;
	}
	else if (attacker->direction == dfPACKET_MOVE_DIR_RR && target->x > attacker->x && target->x <= attacker->x + dfATTACK2_RANGE_X)
	{
		if (abs(target->y - attacker->y) <= dfATTACK2_RANGE_Y)
			return true;
	}
	return false;
}
bool GameServer::CollisionCheck_Attack3(CHARACTER * attacker, CHARACTER * target)
{
	if (attacker->direction == dfPACKET_MOVE_DIR_LL && target->x < attacker->x && target->x >= attacker->x - dfATTACK3_RANGE_X)
	{
		if (abs(target->y - attacker->y) <= dfATTACK3_RANGE_Y)
			return true;
	}
	else if (attacker->direction == dfPACKET_MOVE_DIR_RR && target->x > attacker->x && target->x <= attacker->x + dfATTACK3_RANGE_X)
	{
		if (abs(target->y - attacker->y) <= dfATTACK3_RANGE_Y)
			return true;
	}
	return false;
}
bool GameServer::PacketProc_MoveStart(SESSION * session, Jay::SerializationBuffer* cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	auto iter = _characterMap.find(session->socket);
	CHARACTER* character = iter->second;
	
	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() MoveStart before - sessionID: %d / Dir: %d / X: %d / Y: %d / SectionX: %d / SectionY: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y, character->curSector.x, character->curSector.y);

	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;
	
	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		_syncErrorCount++;
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() MoveStart Sync error - sessionID: %d / Dir: %d / X: %d / Y: %d / ErrorX: %d / ErrorY: %d / ErrorCount: %d", __FUNCTIONW__
			, character->sessionID, character->action, character->x, character->y, x, y, _syncErrorCount);

		Packet::MakeSync(sc_packet, character->sessionID, character->x, character->y);
		SendSectorAround(character->session, sc_packet, true);

		x = character->x;
		y = character->y;
	}
	character->x = x;
	character->y = y;

	switch (direction)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		character->direction = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		character->direction = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
	character->action = direction;

	Packet::MakeMoveStart(sc_packet, character->sessionID, (char)character->action, character->x, character->y);
	SendSectorAround(session, sc_packet, false);

	if (UpdateCharacter_Sector(character))
		UpdatePacket_Sector(character);

	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() MoveStart after - sessionID: %d / Dir: %d / X: %d / Y: %d / SectionX: %d / SectionY: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y, character->curSector.x, character->curSector.y);

	_packetPool.Free(sc_packet);
	return true;
}
bool GameServer::PacketProc_MoveStop(SESSION * session, Jay::SerializationBuffer* cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	auto iter = _characterMap.find(session->socket);
	CHARACTER* character = iter->second;

	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() MoveStop before - sessionID: %d / Dir: %d / X: %d / Y: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y);
	
	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;

	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		_syncErrorCount++;
		Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_ERROR, L"%s() MoveStop Sync error - sessionID: %d / Dir: %d / X: %d / Y: %d / ErrorX: %d / ErrorY: %d / ErrorCount: %d", __FUNCTIONW__
			, character->sessionID, character->action, character->x, character->y, x, y, _syncErrorCount);

		Packet::MakeSync(sc_packet, character->sessionID, character->x, character->y);
		SendSectorAround(character->session, sc_packet, true);

		x = character->x;
		y = character->y;
	}
	character->x = x;
	character->y = y;

	if (UpdateCharacter_Sector(character))
		UpdatePacket_Sector(character);

	switch (direction)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		character->direction = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		character->direction = dfPACKET_MOVE_DIR_RR;
		break;
	default:
		break;
	}
	character->action = -1;

	Packet::MakeMoveStop(sc_packet, character->sessionID, (char)character->direction, character->x, character->y);
	SendSectorAround(session, sc_packet, false);

	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() MoveStop after - sessionID: %d / Dir: %d / X: %d / Y: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y);

	_packetPool.Free(sc_packet);
	return true;
}
bool GameServer::PacketProc_Attack1(SESSION * session, Jay::SerializationBuffer* cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	auto iter = _characterMap.find(session->socket);
	CHARACTER* character = iter->second;

	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() Attack1 - sessionID: %d / Dir: %d / X: %d / Y: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y);

	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;

	Packet::MakeAttack1(sc_packet, character->sessionID, character->direction, character->x, character->y);
	SendSectorAround(session, sc_packet, false);

	bool find = false;
	SECTOR_AROUND sectorAround;
	GetSectorAround(character->curSector.x, character->curSector.y, &sectorAround);
	for (int i = 0; !find && i < sectorAround.count; i++)
	{
		auto sectorList = &_sector[sectorAround.around[i].y][sectorAround.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
		{
			CHARACTER* target_character = *iter;
			if (target_character->sessionID == character->sessionID)
				continue;

			if (CollisionCheck_Attack1(character, target_character))
			{
				target_character->hp -= (target_character->hp > dfATTACK1_DAMAGE) ? dfATTACK1_DAMAGE : target_character->hp;
				Packet::MakeDamage(sc_packet, character->sessionID, target_character->sessionID, target_character->hp);
				SendSectorAround(target_character->session, sc_packet, true);
				find = true;
				break;
			}
		}
	}

	_packetPool.Free(sc_packet);
	return true;
}
bool GameServer::PacketProc_Attack2(SESSION * session, Jay::SerializationBuffer* cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	auto iter = _characterMap.find(session->socket);
	CHARACTER* character = iter->second;

	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() Attack2 - sessionID: %d / Dir: %d / X: %d / Y: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y);

	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;

	Packet::MakeAttack2(sc_packet, character->sessionID, character->direction, character->x, character->y);
	SendSectorAround(session, sc_packet, false);

	bool find = false;
	SECTOR_AROUND sectorAround;
	GetSectorAround(character->curSector.x, character->curSector.y, &sectorAround);
	for (int i = 0; !find && i < sectorAround.count; i++)
	{
		auto sectorList = &_sector[sectorAround.around[i].y][sectorAround.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
		{
			CHARACTER* target_character = *iter;
			if (target_character->sessionID == character->sessionID)
				continue;

			if (CollisionCheck_Attack2(character, target_character))
			{
				target_character->hp -= (target_character->hp > dfATTACK2_DAMAGE) ? dfATTACK2_DAMAGE : target_character->hp;
				Packet::MakeDamage(sc_packet, character->sessionID, target_character->sessionID, target_character->hp);
				SendSectorAround(target_character->session, sc_packet, true);
				find = true;
				break;
			}
		}
	}

	_packetPool.Free(sc_packet);
	return true;
}
bool GameServer::PacketProc_Attack3(SESSION * session, Jay::SerializationBuffer* cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	auto iter = _characterMap.find(session->socket);
	CHARACTER* character = iter->second;

	Jay::Logger::WriteLog(L"Dev", LOG_LEVEL_DEBUG, L"%s() Attack3 - sessionID: %d / Dir: %d / X: %d / Y: %d", __FUNCTIONW__
		, character->sessionID, character->direction, character->x, character->y);

	unsigned char direction;
	unsigned short x;
	unsigned short y;
	*cs_packet >> direction >> x >> y;

	Packet::MakeAttack3(sc_packet, character->sessionID, character->direction, character->x, character->y);
	SendSectorAround(session, sc_packet, false);

	bool find = false;
	SECTOR_AROUND sectorAround;
	GetSectorAround(character->curSector.x, character->curSector.y, &sectorAround);
	for (int i = 0; !find && i < sectorAround.count; i++)
	{
		auto sectorList = &_sector[sectorAround.around[i].y][sectorAround.around[i].x];
		for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
		{
			CHARACTER* target_character = *iter;
			if (target_character->sessionID == character->sessionID)
				continue;

			if (CollisionCheck_Attack3(character, target_character))
			{
				target_character->hp -= (target_character->hp > dfATTACK3_DAMAGE) ? dfATTACK3_DAMAGE : target_character->hp;
				Packet::MakeDamage(sc_packet, character->sessionID, target_character->sessionID, target_character->hp);
				SendSectorAround(target_character->session, sc_packet, true);
				find = true;
				break;
			}
		}
	}

	_packetPool.Free(sc_packet);
	return true;
}
bool GameServer::PacketProc_Sync(SESSION * session, Jay::SerializationBuffer* cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();
	auto iter = _characterMap.find(session->socket);
	CHARACTER* character = iter->second;

	unsigned short x;
	unsigned short y;
	*cs_packet >> x >> y;

	character->x = x;
	character->y = y;

	Packet::MakeSync(sc_packet, character->sessionID, character->x, character->y);
	SendSectorAround(session, sc_packet, true);

	_packetPool.Free(sc_packet);
	return true;
}
bool GameServer::PacketProc_Echo(SESSION * session, Jay::SerializationBuffer * cs_packet)
{
	Jay::SerializationBuffer *sc_packet = _packetPool.Alloc();

	unsigned long time;
	*cs_packet >> time;

	Packet::MakeEcho(sc_packet, time);
	SendUnicast(session, sc_packet);

	_packetPool.Free(sc_packet);
	return true;
}
