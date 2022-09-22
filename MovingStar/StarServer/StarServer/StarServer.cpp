#include "stdafx.h"
#include "ServerManager.h"
#include "ScreenBuffer.h"

int main()
{
	for (;;)
	{
		ServerManager::GetInstance()->Run();
		ScreenBuffer::GetInstance()->Buffer_Flip();
	}
	return 0;
}
