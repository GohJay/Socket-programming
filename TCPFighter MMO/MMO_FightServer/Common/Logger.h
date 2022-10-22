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
	private:
		Logger();
		~Logger();
	public:
		static Logger* GetInstance();
		static void ReleaseInstance();
	public:
		void SetLogLevel(int logLevel);
		void WriteLog(const wchar_t * tag, int logLevel, const wchar_t * fmt, ...);
	private:
		wchar_t _logPath[MAX_PATH];
		int _logLevel;
		static Logger* _instance;
	};
}

#endif //__LOG__H_
