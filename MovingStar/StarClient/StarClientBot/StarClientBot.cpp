#include "stdafx.h"
#include "BotManager.h"

#define FRAME		30
#define INTERVAL	1000 / FRAME

int main()
{
	for (;;)
	{
		BotManager::GetInstance()->Run();
		Sleep(INTERVAL);
	}
	return 0;
}
