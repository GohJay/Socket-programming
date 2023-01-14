#pragma once
#include "../Network/include/RingBuffer.h"
#include "../Common/Protocol.h"
#include <list>

enum POSITION
{
	AUTH = 0,
	LOBBY,
	CHATROOM
};
struct CLIENT
{
	bool enable;
	unsigned int socket;
	wchar_t ip[16];
	int port;
	Jay::RingBuffer recvQ;
	Jay::RingBuffer sendQ;

	POSITION position;
	int userno;
	int enterRoomNo;
	wchar_t nickname[dfNICK_MAX_LEN];
};
struct ROOM
{
	int roomno;
	wchar_t title[256];
	std::list<CLIENT*> userList;
};
