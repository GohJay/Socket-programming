#include "stdafx.h"
#include "Timer.h"
#include "ServerManager.h"

bool g_Shutdown = false;

int main()
{
	timeBeginPeriod(1);

	while (!g_Shutdown)
	{
		Timer::GetInstance()->Update();
		ServerManager::GetInstance()->Run();
		ServerManager::GetInstance()->Control();
		ServerManager::GetInstance()->Monitor();
	}

	wprintf_s(L"Press any key to continue . . . ");
	_getwch();

	timeEndPeriod(1);
	return 0;
}
