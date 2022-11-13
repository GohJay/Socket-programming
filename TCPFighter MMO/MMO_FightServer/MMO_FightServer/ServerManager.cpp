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
ServerManager::ServerManager() : _controlMode(false), _monitoringFlag(false), _loopCount(0), _frameCount(0), _accumtime(0), _monitoringTime(0)
{
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
	_accumtime += Timer::GetInstance()->GetDeltaTime();
	if (_accumtime >= dfINTERVAL)
	{
		beforeTime = timeGetTime();
		_server.Update();
		_logicProcTime += timeGetTime() - beforeTime;

		_accumtime -= dfINTERVAL;
		_frameCount++;
	}

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
			wprintf_s(L"Control Mode: Press R - Reload LogLevel\n");
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

		if ((controlKey == L'r' || controlKey == L'R') && _controlMode)
		{
			Jay::ConfigParser confParser;
			confParser.LoadFile(L"Config.cnf");

			int logLevel;
			confParser.GetValue(L"SERVER", L"LOG", &logLevel);

			Jay::Logger::GetInstance()->SetLogLevel(logLevel);
			wprintf_s(L"Succeed Reload LogLevel [%d] . . . \n", logLevel);
		}
	}
}
void ServerManager::Monitor()
{
	DWORD currentTime = timeGetTime();
	if (currentTime - _monitoringTime < 1000)
		return;

	if (_frameCount != dfFRAME)
	{
		// 모니터링 정보 파일 로그 출력
		Jay::Logger::GetInstance()->WriteLog(L"Dev", LOG_LEVEL_SYSTEM, L"\n\
Frame: %d, Loop/sec: %d\n\
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
\n====================================\n"
			, _frameCount, _loopCount
			, _server._sessionMap.Size()
			, _server._characterMap.Size()
			, _networkProcTime
			, _logicProcTime
			, _cleanupProcTime
			, _server._syncErrorCount
			, _server._unknownPacketCount);
	}

	tm stTime;
	time_t timer;
	if (_monitoringFlag)
	{
		timer = time(NULL);
		localtime_s(&stTime, &timer);

		// 모니터링 정보 콘솔 출력
		wprintf_s(L"\
[%d/%02d/%02d %02d:%02d:%02d]\n\
Frame: %d, Loop/sec: %d\n\
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
			, _frameCount, _loopCount
			, _server._sessionMap.Size()
			, _server._characterMap.Size()
			, _networkProcTime
			, _logicProcTime
			, _cleanupProcTime
			, _server._syncErrorCount
			, _server._unknownPacketCount);
	}

	// 모니터링 정보 초기화
	_loopCount = 0;
	_frameCount = 0;
	_networkProcTime = 0;
	_logicProcTime = 0;
	_cleanupProcTime = 0;
	_monitoringTime = currentTime;
}
bool ServerManager::Init()
{
	Jay::ConfigParser confParser;
	if (!confParser.LoadFile(L"Config.cnf"))
		return false;

	int logLevel;
	int serverPort;
	confParser.GetValue(L"SERVER", L"LOG", &logLevel);
	confParser.GetValue(L"SERVER", L"PORT", &serverPort);

	Jay::Logger::GetInstance()->SetLogLevel(logLevel);
	_server.Listen(serverPort);
	return true;
}
