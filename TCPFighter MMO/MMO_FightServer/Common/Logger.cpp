#include "Logger.h"
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "locale.h"

using namespace Jay;
Logger* Logger::_instance = nullptr;

Logger::Logger() : _logLevel(LOG_LEVEL_SYSTEM)
{
	setlocale(LC_ALL, "");
#ifndef _DEBUG
	GetModuleFileName(NULL, _logPath, MAX_PATH);
	for (size_t i = wcslen(_logPath); i > 0; i--)
	{
		if (_logPath[i] == '\\')
		{
			_logPath[i] = L'\0';
			break;
		}
	}
	wcscat_s(_logPath, L"\\Log");

	if (_waccess(_logPath, 0) == -1)
	{
		if (!CreateDirectory(_logPath, NULL))
			GetCurrentDirectory(MAX_PATH, _logPath);
	}
#endif // !_DEBUG
}
Logger::~Logger()
{
}
Logger * Jay::Logger::GetInstance()
{
	if (_instance == nullptr)
	{
		_instance = new Logger();
		atexit(ReleaseInstance);
	}
	return _instance;
}
void Jay::Logger::ReleaseInstance()
{
	delete _instance;
}
void Jay::Logger::SetLogLevel(int logLevel)
{
	_logLevel = logLevel;
}
void Logger::WriteLog(const wchar_t * tag, int logLevel, const wchar_t * fmt, ...)
{
	if (_logLevel > logLevel)
		return;

	wchar_t log[512];
	*log = L'\0';
	va_list args;
	va_start(args, fmt);
	_vsnwprintf_s(log, sizeof(log), fmt, args);
	va_end(args);

	tm stTime;
	time_t timer = time(NULL);
	localtime_s(&stTime, &timer);
#ifndef _DEBUG
	wchar_t logFile[MAX_PATH];
	swprintf_s(logFile, L"%s\\%s_%d-%02d-%02d.log", _logPath, tag, stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday);
	FILE* pFile;
	if (_wfopen_s(&pFile, logFile, L"at") != 0)
		return;
	fwprintf_s(pFile, L"[%d/%02d/%02d %02d:%02d:%02d] %s\n"
		, stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec, log);
	fclose(pFile);
#else
	wprintf_s(L"[%d/%02d/%02d %02d:%02d:%02d] %s\n", stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec, log);
#endif // !_DEBUG
}
