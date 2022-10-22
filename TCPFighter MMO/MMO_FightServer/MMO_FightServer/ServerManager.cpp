#include "stdafx.h"
#include "ServerManager.h"
#include "Timer.h"
#include "define.h"
#include <conio.h>

extern bool g_Shutdown;
ServerManager ServerManager::_instance;
ServerManager::ServerManager() : _controlMode(false), _monitoringFlag(false), _loopCount(0), _frameCount(0), _accumtime(0), _monitoringTime(0)
{
	_server = new GameServer();
}
ServerManager::~ServerManager()
{
	delete _server;
}
ServerManager * ServerManager::GetInstance()
{
	return &_instance;
}
void ServerManager::Run()
{
	DWORD beforeTime;

	// Network IO
	beforeTime = timeGetTime();
	_server->Network();
	_networkProcTime += timeGetTime() - beforeTime;

	// Logic Update
	_accumtime += Timer::GetInstance()->GetDeltaTime();
	if (_accumtime >= dfINTERVAL)
	{
		beforeTime = timeGetTime();
		_server->Update();
		_logicProcTime += timeGetTime() - beforeTime;

		_accumtime -= dfINTERVAL;
		_frameCount++;
	}

	// Session Cleanup
	beforeTime = timeGetTime();
	_server->Cleanup();
	_cleanupProcTime += timeGetTime() - beforeTime;

	_loopCount++;
}
void ServerManager::Control()
{
	if (_kbhit())
	{
		wchar_t controlKey = _getch();

		// 키보드 제어 허용
		if (controlKey == L'u' || controlKey == L'U')
		{
			_controlMode = true;
			wprintf_s(L"Control Mode: Press Q - Quit\n");
			wprintf_s(L"Control Mode: Press L - Key Lock\n");
			wprintf_s(L"Control Mode: Press M - Monitoring\n");
		}

		// 키보드 제어 잠금
		if ((controlKey == L'l' || controlKey == L'L') && _controlMode)
		{
			_controlMode = false;
			wprintf_s(L"Control Lock! Press U - Key UnLock\n");
		}

		// 키보드 제어 풀림 상태에서의 특정 기능
		if ((controlKey == L'q' || controlKey == L'Q') && _controlMode)
		{
			g_Shutdown = true;
		}

		if ((controlKey == L'm' || controlKey == L'M') && _controlMode)
		{
			_monitoringFlag = !_monitoringFlag;
		}
	}
}
void ServerManager::Monitor()
{
	DWORD currentTime = timeGetTime();
	if (currentTime - _monitoringTime < 1000)
		return;

	tm stTime;
	time_t timer;
	if (_monitoringFlag)
	{
		timer = time(NULL);
		localtime_s(&stTime, &timer);

		// 모니터링 정보 콘솔 출력
		wprintf_s(L"[%d/%02d/%02d %02d:%02d:%02d]\n", stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
		wprintf_s(L"Frame: %d, Loop/sec: %d\n", _frameCount, _loopCount);
		wprintf_s(L"------------------------------------\n");
		wprintf_s(L"Session Count: %d\n", _server->_sessionMap.size());
		wprintf_s(L"Character Count: %d\n", _server->_characterMap.size());
		wprintf_s(L"------------------------------------\n");
		wprintf_s(L"NetworkProc Time: %d ms\n", _networkProcTime);
		wprintf_s(L"Logic Time: %d ms\n", _logicProcTime);
		wprintf_s(L"Cleanup Time: %d ms\n", _cleanupProcTime);
		wprintf_s(L"------------------------------------\n");
		wprintf_s(L"Sync Message Count: %d\n", _server->_syncErrorCount);
		wprintf_s(L"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}

	// 모니터링 정보 초기화
	_loopCount = 0;
	_frameCount = 0;
	_networkProcTime = 0;
	_logicProcTime = 0;
	_cleanupProcTime = 0;
	_monitoringTime = currentTime;
}
