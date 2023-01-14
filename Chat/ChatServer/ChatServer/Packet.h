#pragma once
#include "Object.h"
#include "../Network/include/SerializationBuffer.h"
#include "../Common/Protocol.h"

class Packet
{
public:
	static BYTE MakeChecksum(Jay::SerializationBuffer* packet, WORD type);
	static void MakeResLogin(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, BYTE result, int userno);
	static void MakeResRoomList(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, const std::list<ROOM*>& roomList);
	static void MakeResRoomCreate(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, BYTE result, ROOM* room);
	static void MakeResRoomEnter(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, BYTE result, ROOM* room);
	static void MakeResChat(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, int userno, WORD msgsize, WCHAR * message);
	static void MakeResRoomLeave(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, int userno);
	static void MakeResRoomDelete(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, int roomno);
	static void MakeResUserEnter(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, WCHAR* nickname, int userno);
	static void MakeResStressEcho(st_PACKET_HEADER* header, Jay::SerializationBuffer* packet, WORD msgsize, WCHAR * message);
};
