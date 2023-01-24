#ifndef  __CRASHDUMP__H_
#define  __CRASHDUMP__H_
#include <Windows.h>

namespace Jay
{
	class CrashDump
	{
		/**
		* @file		CrashDump.h
		* @brief	Crash ������ ���� ������ ���� �޸� ���� Ŭ����
		* @details	Crash ���� �߻� �� ���� ���Ͽ� �޸� ���¸� �α��Ѵ�.
		* @author   ������
		* @date		2022-11-28
		* @version  1.0.2
		**/
	private:
		CrashDump();
		~CrashDump();
	public:
		static void Crash(void);
	private:
		static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer);
		static void SetHandlerDump(void);
		static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
		static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue);
		static void myPureCallHandler(void);
	private:
		static long _DumpFlag;
		static CrashDump _instance;
	};
}

#endif
