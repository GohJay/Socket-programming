#include "stdafx.h"
#include "ServerManager.h"

volatile bool g_Shutdown = false;

int main()
{
	while (!g_Shutdown)
	{
		ServerManager::GetInstance()->Run();
	}
	return 0;
}
