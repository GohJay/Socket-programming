#ifndef __LOG__H_
#define __LOG__H_
#include <windows.h>
#include <stdio.h>

namespace Jay
{
	inline
		void WriteLog(const char * args, ...)
	{
		char _log[512];
		*_log = '\0';
		va_list _args;
		va_start(_args, args);
		vsnprintf(_log, sizeof(_log), args, _args);
		va_end(_args);
		OutputDebugStringA(_log);
	}
}

#endif !__LOG__H_
