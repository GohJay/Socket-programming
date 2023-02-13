#include "stdafx.h"
#include "ServerManager.h"
#include "Timer.h"
#include "define.h"
#include <conio.h>
#include "../Common/Logger.h"
#include "../Lib/TextParser/include/ConfigParser.h"
#pragma comment(lib, "../Lib/TextParser/lib/TextParser.lib")

extern bool g_Shutdown;
ServerManager ServerManager::_instance;

ServerManager::ServerManager() : _controlMode(false), _monitoringFlag(false), _loopCount(0), _monitoringTime(0)
{
	LoadData();
	Init();
}
ServerManager::~ServerManager()
{
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
	_server.Network();
	_networkProcTime += timeGetTime() - beforeTime;

	// Logic Update
	beforeTime = timeGetTime();
	_server.Update();
	_logicProcTime += timeGetTime() - beforeTime;

	// Session Cleanup
	beforeTime = timeGetTime();
	_server.Cleanup();
	_cleanupProcTime += timeGetTime() - beforeTime;

	_loopCount++;
}
void ServerManager::Control()
{
	if (_kbhit())
	{
		wchar_t controlKey = _getwch();

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
		wprintf_s(L"\
[%d/%02d/%02d %02d:%02d:%02d]\n\
Loop/sec: %d\n\
------------------------------------\n\
Session Count: %d\n\
Character Count: %d\n\
------------------------------------\n\
NetworkProc Time: %d ms\n\
Logic Time: %d ms\n\
Cleanup Time: %d ms\n\
------------------------------------\n\
Sync Message Count: %d\n\
Unknown Message Count: %d\n\
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
			, stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec
			, _loopCount
			, _server._sessionMap.size()
			, _server._characterMap.size()
			, _networkProcTime
			, _logicProcTime
			, _cleanupProcTime
			, _server._syncErrorCount
			, _server._unknownPacketCount);
	}

	// 모니터링 정보 초기화
	_loopCount = 0;
	_networkProcTime = 0;
	_logicProcTime = 0;
	_cleanupProcTime = 0;
	_monitoringTime = currentTime;
}
bool ServerManager::LoadData()
{
	Jay::ConfigParser confParser;
	if (!confParser.LoadFile(L"Config.cnf"))
		return false;

	confParser.GetValue(L"SERVER", L"LOG", &_serverInfo.logLevel);
	confParser.GetValue(L"SERVER", L"PORT", &_serverInfo.port);
	return true;
}
bool ServerManager::Init()
{
	Jay::Logger::SetLogLevel(_serverInfo.logLevel);
	_server.Listen(_serverInfo.port);
	return true;
}
