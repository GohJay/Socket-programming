#pragma once
#include "../Lib/Network/include/RingBuffer.h"

struct SECTOR
{
	int x;
	int y;
};

struct SECTOR_AROUND
{
	int count;
	SECTOR around[9];
};

struct SESSION
{
	bool enable;
	DWORD64 socket;
	wchar_t ip[16];
	int port;
	DWORD sessionID;
	DWORD lastRecvTime;
	Jay::RingBuffer recvQ;
	Jay::RingBuffer sendQ;
};

struct CHARACTER
{
	SESSION* session;
	DWORD sessionID;
	unsigned long action;
	unsigned char direction;
	unsigned char hp;
	float x;
	float y;
	SECTOR curSector;
	SECTOR oldSector;
};
