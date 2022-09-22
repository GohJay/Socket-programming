#include "stdafx.h"
#include "Packet.h"

BYTE Packet::MakeChecksum(Jay::SerializationBuffer * packet, WORD type)
{
	BYTE* byte = (BYTE*)packet->GetBufferPtr();
	int size = packet->GetUseSize();
	int checksum = type;
	for (int i = 0; i < size; i++)
	{
		checksum += *byte;
		byte++;
	}
	return checksum % 256;
}
void Packet::MakeResLogin(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, BYTE result, int userno)
{
	packet->ClearBuffer();
	*packet << result;
	*packet << userno;
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_LOGIN;
	header->byCheckSum = MakeChecksum(packet, df_RES_LOGIN);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResRoomList(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, const std::list<ROOM*>& roomList)
{
	packet->ClearBuffer();

	// 대화방 개수
	WORD size = roomList.size();
	*packet << size;
	for (auto iter = roomList.begin(); iter != roomList.end(); ++iter)
	{
		// 대화방 번호
		ROOM* room = *iter;
		*packet << room->roomno;

		// 대화방 이름 byte size
		WORD len = wcslen(room->title) * sizeof(WCHAR);
		*packet << len;

		// 대화방 이름 (유니코드)
		packet->PutData((char*)room->title, len);

		// 참여인원
		BYTE count = room->userList.size();
		*packet << count;

		// 참여인원 닉네임
		for (auto iter = room->userList.begin(); iter != room->userList.end(); ++iter)
		{
			CLIENT* client = *iter;
			packet->PutData((char*)client->nickname, sizeof(client->nickname));
		}
	}

	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_ROOM_LIST;
	header->byCheckSum = MakeChecksum(packet, df_RES_ROOM_LIST);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResRoomCreate(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, BYTE result, ROOM* room)
{
	packet->ClearBuffer();
	*packet << result;
	if (result == df_RESULT_ROOM_CREATE_OK)
	{
		*packet << room->roomno;
		WORD len = wcslen(room->title) * sizeof(WCHAR);
		*packet << len;
		packet->PutData((char*)room->title, len);
	}
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_ROOM_CREATE;
	header->byCheckSum = MakeChecksum(packet, df_RES_ROOM_CREATE);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResRoomEnter(st_PACKET_HEADER * header, Jay::SerializationBuffer * packet, BYTE result, ROOM * room)
{
	packet->ClearBuffer();
	*packet << result;
	if (result == df_RESULT_ROOM_ENTER_OK)
	{
		*packet << room->roomno;
		WORD len = wcslen(room->title) * sizeof(WCHAR);
		*packet << len;
		packet->PutData((char*)room->title, len);

		BYTE count = room->userList.size();
		*packet << count;
		for (auto iter = room->userList.begin(); iter != room->userList.end(); ++iter)
		{
			CLIENT* client = *iter;
			packet->PutData((char*)client->nickname, sizeof(client->nickname));
			*packet << client->userno;
		}
	}
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_ROOM_ENTER;
	header->byCheckSum = MakeChecksum(packet, df_RES_ROOM_ENTER);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResChat(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, int userno, WORD msgsize, WCHAR * message)
{
	packet->ClearBuffer();
	*packet << userno;
	*packet << msgsize;
	packet->PutData((char*)message, msgsize);
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_CHAT;
	header->byCheckSum = MakeChecksum(packet, df_RES_CHAT);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResRoomLeave(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, int userno)
{
	packet->ClearBuffer();
	*packet << userno;
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_ROOM_LEAVE;
	header->byCheckSum = MakeChecksum(packet, df_RES_ROOM_LEAVE);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResRoomDelete(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, int roomno)
{
	packet->ClearBuffer();
	*packet << roomno;
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_ROOM_DELETE;
	header->byCheckSum = MakeChecksum(packet, df_RES_ROOM_DELETE);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResUserEnter(st_PACKET_HEADER* header, Jay::SerializationBuffer * packet, WCHAR * nickname, int userno)
{
	packet->ClearBuffer();
	packet->PutData((char*)nickname, dfNICK_MAX_LEN * sizeof(WCHAR));
	*packet << userno;
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_USER_ENTER;
	header->byCheckSum = MakeChecksum(packet, df_RES_USER_ENTER);
	header->wPayloadSize = packet->GetUseSize();
}
void Packet::MakeResStressEcho(st_PACKET_HEADER * header, Jay::SerializationBuffer * packet, WORD msgsize, WCHAR * message)
{
	packet->ClearBuffer();
	*packet << msgsize;
	packet->PutData((char*)message, msgsize);
	header->byCode = dfPACKET_CODE;
	header->wMsgType = df_RES_STRESS_ECHO;
	header->byCheckSum = MakeChecksum(packet, df_RES_STRESS_ECHO);
	header->wPayloadSize = packet->GetUseSize();
}
