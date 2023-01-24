#include "stdafx.h"
#include "../Common/CrashDump.h"
#include "../Common/Logger.h"
#include "../Lib/TextParser/include/ConfigParser.h"
#include "Define.h"
#include "ChatServer.h"
#pragma comment(lib, "../Lib/TextParser/lib/TextParser.lib")
#pragma comment(lib, "Winmm.lib")

SERVER_INFO g_ServerInfo;
SERVICE_INFO g_ServiceInfo;
ChatServer g_Server;
bool g_StopSignal = false;
bool g_ControlMode = false;

void Run();
bool LoadData();
bool Init();
void Monitor();
void Control();

int main()
{
	timeBeginPeriod(1);

	Run();

	wprintf_s(L"Press any key to continue . . . ");
	_getwch();

	timeEndPeriod(1);
	return 0;
}

void Run()
{
	if (!LoadData())
		return;

	if (!Init())
		return;

	if (!g_Server.Start(g_ServerInfo.ip
		, g_ServerInfo.port
		, g_ServerInfo.workerCreateCnt
		, g_ServerInfo.workerRunningCnt
		, g_ServerInfo.sessionMax
		, g_ServerInfo.userMax
		, g_ServerInfo.packetCode
		, g_ServerInfo.packetKey
		, g_ServiceInfo.timeoutSec))
		return;

	while (!g_StopSignal)
	{
		Monitor();
		Control();
		Sleep(1000);
	}

	g_Server.Stop();
}

bool LoadData()
{
	Jay::ConfigParser confParser;
	if (!confParser.LoadFile(L"Config.cnf"))
		return false;

	confParser.GetValue(L"SERVER", L"IP", g_ServerInfo.ip);
	confParser.GetValue(L"SERVER", L"PORT", &g_ServerInfo.port);
	confParser.GetValue(L"SERVER", L"IOCP_WORKER_CREATE", &g_ServerInfo.workerCreateCnt);
	confParser.GetValue(L"SERVER", L"IOCP_WORKER_RUNNING", &g_ServerInfo.workerRunningCnt);
	confParser.GetValue(L"SERVER", L"SESSION_MAX", (int*)&g_ServerInfo.sessionMax);
	confParser.GetValue(L"SERVER", L"USER_MAX", (int*)&g_ServerInfo.userMax);
	confParser.GetValue(L"SERVER", L"PACKET_CODE", (int*)&g_ServerInfo.packetCode);
	confParser.GetValue(L"SERVER", L"PACKET_KEY", (int*)&g_ServerInfo.packetKey);
	confParser.GetValue(L"SERVER", L"LOG", &g_ServerInfo.logLevel);
	confParser.GetValue(L"SERVER", L"LOG_PATH", g_ServerInfo.logPath);
	confParser.GetValue(L"SERVICE", L"TIMEOUT_SEC", &g_ServiceInfo.timeoutSec);
	return true;
}

bool Init()
{
	Jay::Logger::SetLogLevel(g_ServerInfo.logLevel);
	Jay::Logger::SetLogPath(g_ServerInfo.logPath);
	return true;
}

void Monitor()
{
	tm stTime;
	time_t timer;
	timer = time(NULL);
	localtime_s(&stTime, &timer);

	wprintf_s(L"\
[%d/%02d/%02d %02d:%02d:%02d]\n\
------------------------------------\n\
Session Count: %d\n\
Character Count: %d\n\
------------------------------------\n\
Character Pool Use: %d\n\
Packet Pool Use: %d\n\
Job Pool Use: %d\n\
Job Queue Count: %d\n\
------------------------------------\n\
Total Accept: %d\n\
Accept TPS: %d\n\
Recv TPS: %d\n\
Send TPS: %d\n\
------------------------------------\n\
\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		, stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec
		, g_Server.GetSessionCount()
		, g_Server.GetCharacterCount()
		, g_Server.GetUseCharacterPool()
		, g_Server.GetUsePacketCount()
		, g_Server.GetUseJobPool()
		, g_Server.GetJobQueueCount()
		, g_Server.GetTotalAcceptCount()
		, g_Server.GetAcceptTPS()
		, g_Server.GetRecvTPS()
		, g_Server.GetSendTPS());
}

void Control()
{
	wchar_t controlKey;
	if (_kbhit())
	{
		controlKey = _getwch();

		// 키보드 제어 허용
		if (controlKey == L'u' || controlKey == L'U')
		{
			g_ControlMode = true;
			wprintf_s(L"Control Mode: Press Q - Quit\n");
			wprintf_s(L"Control Mode: Press L - Key Lock\n");
		}

		// 키보드 제어 잠금
		if ((controlKey == L'l' || controlKey == L'L') && g_ControlMode)
		{
			g_ControlMode = false;
			wprintf_s(L"Control Lock! Press U - Key UnLock\n");
		}

		// 키보드 제어 풀림 상태에서의 특정 기능
		if ((controlKey == L'q' || controlKey == L'Q') && g_ControlMode)
		{
			g_StopSignal = true;
		}
	}
}
