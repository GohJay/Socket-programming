#include "stdafx.h"
#include "KeyHandler.h"
#include "ClientManager.h"
#include "ScreenBuffer.h"

#pragma comment(lib, "Winmm.lib")
#define FRAME		30
#define INTERVAL	1000 / FRAME

void Work()
{
	DWORD deltatime;
	DWORD aftertime;
	DWORD beforetime = timeGetTime();
	for (;;)
	{
		// Input
		KeyHandler::GetInstance()->KeyProcess();

		// Logic
		ClientManager::GetInstance()->Run();

		aftertime = timeGetTime();
		deltatime = aftertime - beforetime;
		if (deltatime < INTERVAL)
		{
			// Render
			ScreenBuffer::GetInstance()->Buffer_Flip();
			Sleep(INTERVAL - deltatime);
		}
		beforetime += INTERVAL;
	}
}
int main()
{
	timeBeginPeriod(1);
	Work();
	timeEndPeriod(1);
	return 0;
}
