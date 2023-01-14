#include "stdafx.h"
#include "NetServer.h"
#include "Util.h"
#include "Packet.h"
#include "define.h"
#include "../Network/include/SerializationBuffer.h"
#include "../Common/Protocol.h"
#include <mstcpip.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../Network/lib/Network.lib")

NetServer::NetServer() 
	: _listenSocket(INVALID_SOCKET), _keyUserNo(1), _keyRoomNo(1), _clientPool(0), _roomPool(0), _packetPool(0)
{
	WSADATA ws;
	int status = WSAStartup(MAKEWORD(2, 2), &ws);
	if (status != 0)
	{
		wprintf(L"NetServer::NetServer() Failed WSAStartup: %d\n", status);
		CRASH;
	}
	Listen();
}
NetServer::~NetServer()
{
	DestroyAll();
	WSACleanup();
}
void NetServer::Network()
{
	DWORD clientNoTable[FD_SETSIZE];
	SOCKET clientSockTable[FD_SETSIZE];
	int socketCount = 0;
	
	fd_set rfds;
	fd_set sfds;
	FD_ZERO(&rfds);
	FD_ZERO(&sfds);
	memset(clientNoTable, -1, sizeof(clientNoTable));
	memset(clientSockTable, INVALID_SOCKET, sizeof(clientSockTable));

	clientNoTable[socketCount] = 0;
	clientSockTable[socketCount] = _listenSocket;
	FD_SET(_listenSocket, &rfds);
	socketCount++;
	
	for (auto iter = _clientMap.begin(); iter != _clientMap.end(); ++iter)
	{
		CLIENT *client = iter->second;
		clientNoTable[socketCount] = client->userno;
		clientSockTable[socketCount] = client->socket;
		if (client->sendQ.GetUseSize() > 0)
			FD_SET(client->socket, &sfds);
		FD_SET(client->socket, &rfds);
		socketCount++;

		if (socketCount >= FD_SETSIZE)
		{
			SelectSocket(clientNoTable, clientSockTable, &rfds, &sfds);
			FD_ZERO(&rfds);
			FD_ZERO(&sfds);
			memset(clientNoTable, -1, sizeof(clientNoTable));
			memset(clientSockTable, INVALID_SOCKET, sizeof(clientSockTable));
			socketCount = 0;
		}
	}
	if (socketCount > 0)
		SelectSocket(clientNoTable, clientSockTable, &rfds, &sfds);
}
void NetServer::Cleanup()
{
	for (auto iter = _clientMap.begin(); iter != _clientMap.end();)
	{
		CLIENT *client = iter->second;
		if (!client->enable)
		{
			Disconnect(client);
			_clientPool.Free(client);
			iter = _clientMap.erase(iter);
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
		wprintf(L"NetServer::Listen() Failed socket: %d\n", err);
		return;
	}

	linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	int result = setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf(L"NetServer::Listen() Failed setsockopt linger: %d\n", err);
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

	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(dfNETWORK_PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(_listenSocket, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf(L"NetServer::Listen() Failed bind: %d\n", err);
		closesocket(_listenSocket);
		return;
	}

	result = listen(_listenSocket, SOMAXCONN_HINT(65535));
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf(L"NetServer::Listen() Failed listen: %d\n", err);
		closesocket(_listenSocket);
		return;
	}
	printf_s("Listen Success Port: %d\n", dfNETWORK_PORT);
}
void NetServer::SelectSocket(DWORD * clientNoTable, SOCKET * clientSockTable, FD_SET * readset, FD_SET * writeset)
{
	timeval tv = { 0, 0 };
	int result = select(NULL, readset, writeset, NULL, &tv);
	if (result == SOCKET_ERROR)
	{
		wchar_t msg[64];
		swprintf_s(msg, L"NetServer::Network() Failed select: %d", WSAGetLastError());
		MessageBox(NULL, msg, dfSERVER_NAME, MB_ICONERROR);
		CRASH;
	}

	for (int i = 0; i < FD_SETSIZE; i++)
	{
		if (clientSockTable[i] == INVALID_SOCKET)
			continue;

		if (FD_ISSET(clientSockTable[i], writeset))
			SendProc(clientNoTable[i]);

		if (FD_ISSET(clientSockTable[i], readset))
		{
			if (clientNoTable[i] == 0)
				AcceptProc();
			else
				RecvProc(clientNoTable[i]);
		}
	}
}
void NetServer::AcceptProc()
{
	SOCKADDR_IN clientAddr = {};
	int clientSize = sizeof(clientAddr);
	SOCKET socket = accept(_listenSocket, (SOCKADDR*)&clientAddr, &clientSize);
	if (socket == INVALID_SOCKET)
	{
		wchar_t msg[64];
		swprintf_s(msg, L"NetServer::Accept() Failed accept: %d", WSAGetLastError());
		MessageBox(NULL, msg, dfSERVER_NAME, MB_ICONERROR);
		CRASH;
	}

	if (_clientMap.size() >= dfCLIENT_MAX)
	{
		closesocket(socket);
		return;
	}

	CLIENT *new_client = _clientPool.Alloc();
	new_client->enable = true;
	new_client->socket = socket;
	InetNtop(AF_INET, &clientAddr.sin_addr, new_client->ip, sizeof(new_client->ip));
	new_client->port = ntohs(clientAddr.sin_port);
	new_client->recvQ.ClearBuffer();
	new_client->sendQ.ClearBuffer();
	new_client->position = POSITION::AUTH;
	new_client->userno = _keyUserNo++;
	new_client->enterRoomNo = 0;
	_clientMap.insert({new_client->userno, new_client});
}
void NetServer::RecvProc(int userno)
{
	CLIENT* client = _clientMap[userno];
	if (!client->enable)
		return;

	int size = client->recvQ.DirectEnqueueSize();
	if (size <= 0)
	{
		Disable(client);
		return;
	}

	int err;
	int len = recv(client->socket, client->recvQ.GetRearBufferPtr(), size, 0);
	switch (len)
	{
	case SOCKET_ERROR:
		err = WSAGetLastError();
		if (err != WSAECONNRESET && err != WSAECONNABORTED)
			wprintf(L"NetServer::RecvProc() Failed recv: %d, UserNo: %d\n", err, client->userno);
	case 0:
		Disable(client);
		return;
	default:
		break;
	}
	client->recvQ.MoveRear(len);
	wprintf(L"Recv size: %d, Userno: %d\n", len, client->userno);

	for (;;)
	{
		int ret = CompleteRecvPacket(client);
		if (ret == 0)
			break;
		else if (ret == -1)
		{
			Disable(client);
			break;
		}
	}
}
int NetServer::CompleteRecvPacket(CLIENT * client)
{
	int headerSize = sizeof(st_PACKET_HEADER);
	if (client->recvQ.GetUseSize() < headerSize)
		return 0;

	st_PACKET_HEADER header;
	int ret = client->recvQ.Peek((char*)&header, headerSize);
	if (ret != headerSize)
	{
		wprintf(L"NetServer::RecvProc() Failed recvQ.Peek() size : %d, UserNo: %d\n", ret, client->userno);
		CRASH;
	}

	if (header.byCode != dfPACKET_CODE)
		return -1;

	if (client->recvQ.GetUseSize() >= headerSize + header.wPayloadSize)
		client->recvQ.MoveFront(headerSize);
	else
		return 0;

	Jay::SerializationBuffer* packet = _packetPool.Alloc();
	packet->ClearBuffer();
	ret = client->recvQ.Dequeue(packet->GetBufferPtr(), header.wPayloadSize);
	if (ret != header.wPayloadSize)
	{
		wprintf(L"NetServer::RecvProc() Failed recvQ.Dequeue() size: %d, UserNo: %d\n", ret, client->userno);
		CRASH;
	}
	packet->MoveRear(ret);

	if (header.byCheckSum != Packet::MakeChecksum(packet, header.wMsgType))
	{
		_packetPool.Free(packet);
		return -1;
	}
	if (!PacketProc(client, packet, header.wMsgType))
	{
		_packetPool.Free(packet);
		return -1;
	}
	_packetPool.Free(packet);
	return 1;
}
void NetServer::SendProc(int userno)
{
	CLIENT* client = _clientMap[userno];
	if (!client->enable)
		return;
	
	char message[1024];
	int size = client->sendQ.Peek(message, sizeof(message));
	if (size <= 0)
		return;

	int ret = send(client->socket, message, size, 0);
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
			wprintf(L"NetServer::SendProc() Failed send error: %d, IP: %s, Port: %d, UserNo: %d\n", err, client->ip, client->port, client->userno);
			break;
		}
		Disable(client);
		return;
	}
	client->sendQ.MoveFront(ret);
	wprintf(L"Send size: %d, UserNo: %d\n", ret, client->userno);
}
void NetServer::SendUnicast(CLIENT * target, st_PACKET_HEADER* header, Jay::SerializationBuffer* sc_packet)
{
	// Send Header
	int size = sizeof(st_PACKET_HEADER);
	int ret = target->sendQ.Enqueue((char*)header, size);
	if (ret != size)
	{
		wprintf(L"NetServer::SendUnicast() Failed sendQ.Enqueue() size: %d, UserNo: %d\n", ret, target->userno);
		Disable(target);
		return;
	}

	// Send Payload
	size = sc_packet->GetUseSize();
	ret = target->sendQ.Enqueue(sc_packet->GetBufferPtr(), size);
	if (ret != size)
	{
		wprintf(L"NetServer::SendUnicast() Failed sendQ.Enqueue() size: %d, UserNo: %d\n", ret, target->userno);
		Disable(target);
		return;
	}
}
void NetServer::SendBroadcast(CLIENT * exclusion, st_PACKET_HEADER* header, Jay::SerializationBuffer* sc_packet)
{
	for (auto iter = _clientMap.begin(); iter != _clientMap.end(); ++iter)
	{
		CLIENT *client = iter->second;
		if (client != exclusion)
			SendUnicast(client, header, sc_packet);
	}
}
void NetServer::SendBroadcast_Room(CLIENT * exclusion, st_PACKET_HEADER * header, Jay::SerializationBuffer * sc_packet, ROOM * room)
{
	for (auto iter = room->userList.begin(); iter != room->userList.end(); ++iter)
	{
		CLIENT *client = *iter;
		if (client != exclusion)
			SendUnicast(client, header, sc_packet);
	}
}
void NetServer::Disable(CLIENT * client)
{
	client->enable = false;
}
void NetServer::Disconnect(CLIENT * client)
{
	if (client->position == POSITION::CHATROOM)
	{
		// ��ȭ�� ����
		ROOM* room = _roomMap[client->enterRoomNo];

		st_PACKET_HEADER header;
		Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();

		// ��ȭ�濡 �ִ� �����鿡�� ���� ���� ����
		Packet::MakeResRoomLeave(&header, sc_packet, client->userno);
		SendBroadcast_Room(nullptr, &header, sc_packet, room);

		client->position = POSITION::LOBBY;
		room->userList.remove(client);

		// ��ȭ���� ������ �������ٸ� ��ȭ�� ����
		if (room->userList.empty())
		{
			Packet::MakeResRoomDelete(&header, sc_packet, room->roomno);
			SendBroadcast(nullptr, &header, sc_packet);

			_roomMap.erase(room->roomno);
			_roomTitleTable.erase(room->title);
			_roomPool.Free(room);
		}

		_packetPool.Free(sc_packet);
	}
	_userNameTable.erase(client->nickname);
	closesocket(client->socket);
	wprintf(L"Disconnect IP: %s, Port: %d, UserNo: %d\n", client->ip, client->port, client->userno);
}
void NetServer::DestroyAll()
{
	for (auto iter = _clientMap.begin(); iter != _clientMap.end();)
	{
		CLIENT *client = iter->second;
		closesocket(client->socket);
		_clientPool.Free(client);
		iter = _clientMap.erase(iter);
	}
	closesocket(_listenSocket);
}
bool NetServer::PacketProc(CLIENT * client, Jay::SerializationBuffer * cs_packet, WORD type)
{
	wprintf(L"NetServer::PacketProc() Type: %d, UserNo: %d\n", type, client->userno);
	switch (type)
	{
	case df_REQ_LOGIN:
		return PacketProc_Login(client, cs_packet);
	case df_REQ_ROOM_LIST:
		return PacketProc_RoomList(client, cs_packet);
	case df_REQ_ROOM_CREATE:
		return PacketProc_RoomCreate(client, cs_packet);
	case df_REQ_ROOM_ENTER:
		return PacketProc_RoomEnter(client, cs_packet);
	case df_REQ_CHAT:
		return PacketProc_Chat(client, cs_packet);
	case df_REQ_ROOM_LEAVE:
		return PacketProc_RoomLeave(client, cs_packet);
	case df_REQ_STRESS_ECHO:
		return PacketProc_StressEcho(client, cs_packet);
	default:
		wprintf(L"NetServer::PacketProc() UNKNOWN_PACKET\n");
		break;
	}
	return false;
}
bool NetServer::PacketProc_Login(CLIENT * client, Jay::SerializationBuffer* cs_packet)
{
	WCHAR nickname[dfNICK_MAX_LEN];
	cs_packet->GetData((char*)nickname, sizeof(nickname));

	BYTE result = df_RESULT_LOGIN_OK;
	do
	{
		// �α��� ����: ������ �������� �ߺ� �α��� �õ�
		if (client->position != POSITION::AUTH)
		{
			result = df_RESULT_LOGIN_ETC;
			break;
		}

		// �α��� ����: ������ �� ����
		if (_clientMap.size() >= dfLOGIN_MAX)
		{
			result = df_RESULT_LOGIN_MAX;
			break;
		}

		// �α��� ����: �ߺ� �г��� ����
		auto iter = _userNameTable.find(nickname);
		if (iter != _userNameTable.end())
		{
			result = df_RESULT_LOGIN_DNICK;
			break;
		}
		
		// �α��� ����
		wcscpy_s(client->nickname, nickname);
		client->position = POSITION::LOBBY;
		_userNameTable.insert(nickname);
	} while (0);

	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
	Packet::MakeResLogin(&header, sc_packet, result, client->userno);
	SendUnicast(client, &header, sc_packet);
	_packetPool.Free(sc_packet);
	return true;
}
bool NetServer::PacketProc_RoomList(CLIENT * client, Jay::SerializationBuffer * cs_packet)
{
	std::list<ROOM*> roomList;
	for (auto iter = _roomMap.begin(); iter != _roomMap.end(); ++iter)
	{
		ROOM* room = iter->second;
		roomList.push_back(room);
	}

	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
	Packet::MakeResRoomList(&header, sc_packet, roomList);
	SendUnicast(client, &header, sc_packet);
	_packetPool.Free(sc_packet);
	return true;
}
bool NetServer::PacketProc_RoomCreate(CLIENT * client, Jay::SerializationBuffer * cs_packet)
{
	WORD len;
	WCHAR title[256];
	*cs_packet >> len;
	cs_packet->GetData((char*)title, len);
	title[len / 2] = L'\0';

	BYTE result = df_RESULT_ROOM_CREATE_OK;
	do
	{
		// ��ȭ�� ���� ����: ������ ��ġ�� �κ� �ƴ�
		if (client->position != POSITION::LOBBY)
		{
			result = df_RESULT_ROOM_CREATE_ETC;
			break;
		}

		// ��ȭ�� ���� ����: ��ȭ�� ���� �� �ִ�ġ �ʰ�
		if (_roomMap.size() >= dfCHATROOM_MAX)
		{
			result = df_RESULT_ROOM_CREATE_MAX;
			break;
		}

		// ��ȭ�� ���� ����: �ߺ��� ��ȭ�� �̸� ����
		auto iter = _roomTitleTable.find(title);
		if (iter != _roomTitleTable.end())
		{
			result = df_RESULT_ROOM_CREATE_DNICK;
			break;
		}

		// ��ȭ�� ���� ����
		ROOM* room = _roomPool.Alloc();
		room->userList.clear();
		room->roomno = _keyRoomNo++;
		wcscpy_s(room->title, title);
		_roomMap.insert({room->roomno, room});
		_roomTitleTable.insert(title);

		// ��ȭ�� ���� ���� �� ��� �������� ��� ����
		st_PACKET_HEADER header;
		Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
		Packet::MakeResRoomCreate(&header, sc_packet, result, room);
		SendBroadcast(nullptr, &header, sc_packet);
		_packetPool.Free(sc_packet);
		return true;
	} while (0);
	
	// ��ȭ�� ���� ���� �� ��ȭ�� ������ ��û�� �������Ը� ��� ����
	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
	Packet::MakeResRoomCreate(&header, sc_packet, result, nullptr);
	SendUnicast(client, &header, sc_packet);
	_packetPool.Free(sc_packet);
	return true;
}
bool NetServer::PacketProc_RoomEnter(CLIENT * client, Jay::SerializationBuffer * cs_packet)
{
	int roomno;
	*cs_packet >> roomno;

	BYTE result = df_RESULT_ROOM_ENTER_OK;
	do
	{
		// ��ȭ�� ���� ����: ������ ��ġ�� �κ� �ƴ�
		if (client->position != POSITION::LOBBY)
		{
			result = df_RESULT_ROOM_ENTER_ETC;
			break;
		}

		// ��ȭ�� ���� ����: ��ȭ�� �ִ� ���� �ο� �� �ʰ�
		if (_roomMap.size() >= dfCHATUSER_MAX)
		{
			result = df_RESULT_ROOM_ENTER_MAX;
			break;
		}

		// ��ȭ�� ���� ����: �������� �ʴ� ��ȭ�� ��ȣ
		auto iter = _roomMap.find(roomno);
		if (iter == _roomMap.end())
		{
			result = df_RESULT_ROOM_ENTER_NOT;
			break;
		}
		
		// ��ȭ�� ����
		ROOM* room = iter->second;
		room->userList.push_back(client);
		client->position = POSITION::CHATROOM;
		client->enterRoomNo = room->roomno;

		st_PACKET_HEADER header;
		Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();

		// �ű� ������ �������� ���� ��ȭ�濡 �ִ� ������ ���� ����
		Packet::MakeResRoomEnter(&header, sc_packet, result, room);
		SendUnicast(client, &header, sc_packet);
		
		// ���� ��ȭ�濡 �ִ� �����鿡�� �ű� ������ ���� ���� ����
		Packet::MakeResUserEnter(&header, sc_packet, client->nickname, client->userno);
		SendBroadcast_Room(client, &header, sc_packet, room);

		_packetPool.Free(sc_packet);
		return true;
	} while (0);

	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
	Packet::MakeResRoomEnter(&header, sc_packet, result, nullptr);
	SendUnicast(client, &header, sc_packet);
	_packetPool.Free(sc_packet);
	return true;
}
bool NetServer::PacketProc_Chat(CLIENT * client, Jay::SerializationBuffer * cs_packet)
{
	// ä�ø޽��� ���� ����: ������ ��ġ�� ��ȭ���� �ƴ�
	if (client->position != POSITION::CHATROOM)
		return false;

	// ä�ø޽��� ����
	WORD len;
	*cs_packet >> len;
	wchar_t message[512];
	cs_packet->GetData((char*)message, len);

	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
	ROOM* room = _roomMap[client->enterRoomNo];
	Packet::MakeResChat(&header, sc_packet, client->userno, len, message);
	SendBroadcast_Room(client, &header, sc_packet, room);
	_packetPool.Free(sc_packet);
	return true;
}
bool NetServer::PacketProc_RoomLeave(CLIENT * client, Jay::SerializationBuffer * cs_packet)
{
	// ��ȭ�� ���� ����: ������ ��ġ�� ��ȭ���� �ƴ�
	if (client->position != POSITION::CHATROOM)
		return false;

	// ��ȭ�� ����
	ROOM* room = _roomMap[client->enterRoomNo];

	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();

	// ��ȭ�濡 �ִ� �����鿡�� ���� ���� ����
	Packet::MakeResRoomLeave(&header, sc_packet, client->userno);
	SendBroadcast_Room(nullptr, &header, sc_packet, room);

	client->position = POSITION::LOBBY;
	room->userList.remove(client);

	// ��ȭ���� ������ �������ٸ� ��ȭ�� ����
	if (room->userList.empty())
	{
		Packet::MakeResRoomDelete(&header, sc_packet, room->roomno);
		SendBroadcast(nullptr, &header, sc_packet);

		_roomMap.erase(room->roomno);
		_roomTitleTable.erase(room->title);
		_roomPool.Free(room);
	}

	_packetPool.Free(sc_packet);
	return true;
}
bool NetServer::PacketProc_StressEcho(CLIENT * client, Jay::SerializationBuffer * cs_packet)
{
	// ��Ʈ�����׽�Ʈ �޽��� ����
	WORD len;
	*cs_packet >> len;
	wchar_t message[512];
	cs_packet->GetData((char*)message, len);

	st_PACKET_HEADER header;
	Jay::SerializationBuffer* sc_packet = _packetPool.Alloc();
	Packet::MakeResStressEcho(&header, sc_packet, len, message);
	SendUnicast(client, &header, sc_packet);
	_packetPool.Free(sc_packet);
	return true;
}
