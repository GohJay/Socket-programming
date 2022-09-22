#pragma once
#include "Object.h"
#include "../Network/include/SerializationBuffer.h"
#include "../Common/List.h"

class GameServer
{
public:
	GameServer();
	~GameServer();
public:
	void Network();
	void Update();
	void Cleanup();
protected:
	void Listen();
	void AcceptProc();
	void RecvProc(Player* player);
	void SendProc(Player* player);
	void SendUnicast(Player* target, Jay::SerializationBuffer* sc_packet);
	void SendBroadcast(Player* exclusion, Jay::SerializationBuffer* sc_packet);
	void Disable(Player* player);
	void Disconnect(Player* player);
	void DestroyAll();
	void TranslatePacket(Player* player, Jay::SerializationBuffer* cs_packet, int type);
private:
	void PacketProc_MoveStart(Player* player, Jay::SerializationBuffer* cs_packet);
	void PacketProc_MoveStop(Player* player, Jay::SerializationBuffer* cs_packet);
	void PacketProc_Attack1(Player* player, Jay::SerializationBuffer* cs_packet);
	void PacketProc_Attack2(Player* player, Jay::SerializationBuffer* cs_packet);
	void PacketProc_Attack3(Player* player, Jay::SerializationBuffer* cs_packet);
	void PacketProc_Sync(Player* player, Jay::SerializationBuffer* cs_packet);
private:
	unsigned int _listenSocket;
	int _allocId;
	Jay::list<Player*> _playerList;
};
