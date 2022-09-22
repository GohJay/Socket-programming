#pragma once
#include "../Network/include/RingBuffer.h"

struct Player
{
	bool enable;
	unsigned int socket;
	char ip[16];
	int port;
	Jay::RingBuffer recvQ;
	Jay::RingBuffer sendQ;

	unsigned long action;
	unsigned int id;
	unsigned char direction;
	unsigned char hp;
	unsigned short x;
	unsigned short y;
};
