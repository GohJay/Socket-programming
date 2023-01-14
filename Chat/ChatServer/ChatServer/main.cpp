#include "stdafx.h"
#include "NetServer.h"
#pragma comment(lib, "Winmm.lib")

volatile bool g_Shutdown = false;

int main()
{
	timeBeginPeriod(1);

	NetServer chatServer;
	while (!g_Shutdown)
	{
		chatServer.Network();
		chatServer.Cleanup();
	}

	timeEndPeriod(1);
	return 0;
}
