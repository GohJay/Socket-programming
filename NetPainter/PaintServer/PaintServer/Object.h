#pragma once
#include "../../Network/include/RingBuffer.h"

struct Session
{
	unsigned int socket;
	wchar_t ip[16];
	int port;
	bool sendflag;
	Jay::RingBuffer recvBuffer;
	Jay::RingBuffer sendBuffer;
};
