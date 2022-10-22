#pragma once
#include "Object.h"
#include "define.h"
#include "../Common/Protocol.h"
#include "../Common/ObjectPool.h"
#include "../Common/Logger.h"
#include "../Network/include/SerializationBuffer.h"
#include <unordered_map>
#include <list>
#include <queue>

class GameServer
{
public:
	GameServer();
	~GameServer();
public:
	void Network();
	void Update();
	void Cleanup();
private:
	void Listen();
	void SelectSocket(SOCKET* userSockTable, FD_SET* readset, FD_SET* writeset);
	void AcceptProc();
	void RecvProc(SOCKET socket);
	int CompleteRecvPacket(SESSION* session);
	void SendProc(SOCKET socket);
	void SendUnicast(SESSION* session, Jay::SerializationBuffer* sc_packet);
	void SendBroadcast(SESSION* exclusion, Jay::SerializationBuffer* sc_packet);
	void SendSectorOne(SESSION* exclusion, Jay::SerializationBuffer* sc_packet, int sectorX, int sectorY);
	void SendSectorAround(SESSION* session, Jay::SerializationBuffer* sc_packet, bool sendMe = false);
	void Disable(SESSION* session);
	void DestroyAll();
	SESSION* CreateSession(SOCKET socket, SOCKADDR_IN* socketAddr);
	void DisconnectSession(SOCKET socket);
	bool PacketProc(SESSION* session, Jay::SerializationBuffer* cs_packet, WORD type);
private:
	CHARACTER* CreateCharacter(SESSION* session);
	void DestroyCharacter(CHARACTER* character);
	void AddCharacter_Sector(CHARACTER* character);
	void RemoveCharacter_Sector(CHARACTER* character);
	bool UpdateCharacter_Sector(CHARACTER* character);
	void UpdatePacket_Sector(CHARACTER* character);
	void GetSectorAround(int sectorX, int sectorY, SECTOR_AROUND* sectorAround);
	void GetUpdateSectorAround(CHARACTER* character, SECTOR_AROUND* removeSector, SECTOR_AROUND* addSector);
	bool IsMovableCharacter(int x, int y);
	bool CollisionCheck_Attack1(CHARACTER* attacker, CHARACTER* target);
	bool CollisionCheck_Attack2(CHARACTER* attacker, CHARACTER* target);
	bool CollisionCheck_Attack3(CHARACTER* attacker, CHARACTER* target);
private:
	bool PacketProc_MoveStart(SESSION* session, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_MoveStop(SESSION* session, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_Attack1(SESSION* session, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_Attack2(SESSION* session, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_Attack3(SESSION* session, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_Sync(SESSION* session, Jay::SerializationBuffer* cs_packet);
	bool PacketProc_Echo(SESSION* session, Jay::SerializationBuffer* cs_packet);
private:
	std::unordered_map<DWORD, SESSION*> _sessionMap;
	Jay::ObjectPool<SESSION> _sessionPool;
	std::queue<SESSION*> _gcQueue;
	std::unordered_map<DWORD, CHARACTER*> _characterMap;
	Jay::ObjectPool<CHARACTER> _characterPool;
	Jay::ObjectPool<Jay::SerializationBuffer> _packetPool;
	std::list<CHARACTER*> _sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];
	Jay::Logger* _log;
	unsigned int _listenSocket;
	int _keySessionID;
	int _syncErrorCount;
	friend class ServerManager;
};
