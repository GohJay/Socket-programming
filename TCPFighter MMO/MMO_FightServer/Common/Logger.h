#ifndef __LOG__H_
#define __LOG__H_
#include <Windows.h>

#define LOG_LEVEL_DEBUG		0
#define LOG_LEVEL_SYSTEM	1
#define LOG_LEVEL_ERROR		2

namespace Jay
{
	class Logger
	{
		/**
		* @file		Logger.h
		* @brief	File Logger Class
		* @details	���� �α� ��¿� Ŭ����
		* @author   ������
		* @date		2022-12-24
		* @version  1.0.2
		**/
	private:
		Logger();
		~Logger();
	public:
		static void SetLogLevel(int logLevel);
		static void SetLogPath(const wchar_t* logPath);
		static void WriteLog(const wchar_t* type, int logLevel, const wchar_t* fmt, ...);
		static void WriteHex(const wchar_t* type, int logLevel, const wchar_t* log, BYTE* byte, int byteLen);
	private:
		static void WriteProc(const wchar_t* type, int logLevel, const wchar_t* buffer, bool truncated);
	private:
		static DWORD _logIndex;
		static int _logLevel;
		static wchar_t _logPath[MAX_PATH];
		static SRWLOCK _logLock;
		static Logger _instance;
	};
}

#endif //__LOG__H_
