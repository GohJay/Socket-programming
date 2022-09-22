#ifndef __OBJECT__H_
#define __OBJECT__H_
#include "../../Network/include/RingBuffer.h"

struct Star
{
	int id;
	int x;
	int y;
};
struct Player
{
	bool enable;
	unsigned int socket;
	char ip[16];
	int port;
	Star star;
	Jay::RingBuffer recvBuffer;
	Jay::RingBuffer sendBuffer;
};

#endif
