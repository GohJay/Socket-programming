#pragma once
#include "Object.h"
#include "../Common/ObjectPool.h"
#include "../Network/include/SerializationBuffer.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

class NetServer
{
public:
	NetServer();
	~NetServer();
public:
	void Network();
	void Cleanup();
protected:
	void Listen();
	void SelectSocket(DWORD *clientNoTable, SOCKET* clientSockTable, FD_SET* readset, FD_SET* writeset);
	void AcceptProc();
	void RecvProc(int userno);
	int CompleteRecvPacket(CLIENT* client);
	void SendProc(int userno);
	void SendUnicast(CLIENT* target, st_PACKET_HEADER* header, Jay::SerializationBuffer* sc_packet);
	void SendBroadcast(CLIENT* exclusion, st_PACKET_HEADER* header, Jay::SerializationBuffer* sc_packet);
	void SendBroadcast_Room(CLIENT* exclusion, st_PACKET_HEADER* header, Jay::SerializationBuffer* sc_packet, ROOM* room);
	void Disable(CLIENT* client);
	void Disconnect(CLIENT* client);
	void DestroyAll();
	bool PacketProc(CLIENT* client, Jay::SerializationBuffer* cs_packet, WORD type);
private:
	bool PacketProc_Login(CLIENT* client, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_RoomList(CLIENT* client, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_RoomCreate(CLIENT* client, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_RoomEnter(CLIENT* client, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_Chat(CLIENT* client, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_RoomLeave(CLIENT* client, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_StressEcho(CLIENT* client, Jay::SerializationBuffer* cs_packet);
private:
	std::unordered_map<int, CLIENT*> _clientMap;
	Jay::ObjectPool<CLIENT> _clientPool;
	std::unordered_set<std::wstring> _userNameTable;
	std::unordered_map<int, ROOM*> _roomMap;
	Jay::ObjectPool<ROOM> _roomPool;
	std::unordered_set<std::wstring> _roomTitleTable;
	int _keyUserNo;
	int _keyRoomNo;
	Jay::ObjectPool<Jay::SerializationBuffer> _packetPool;
	unsigned int _listenSocket;
};
