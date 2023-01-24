#include "CrashDump.h"
#include <stdio.h>
#include <crtdbg.h>
#include <minidumpapiset.h>
#pragma comment(lib, "DbgHelp.lib")

using namespace Jay;
long CrashDump::_DumpFlag = FALSE;
CrashDump CrashDump::_instance;

CrashDump::CrashDump()
{
	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = myInvalidParameterHandler;
	oldHandler = _set_invalid_parameter_handler(newHandler);	// crt �Լ��� null ������ ���� �־��� ��

	_CrtSetReportMode(_CRT_WARN, 0);		// CRT ���� �޽��� ǥ�� �ߴ�. �ٷ� ���� ������
	_CrtSetReportMode(_CRT_ASSERT, 0);		// CRT ���� �޽��� ǥ�� �ߴ�. �ٷ� ���� ������
	_CrtSetReportMode(_CRT_ERROR, 0);		// CRT ���� �޽��� ǥ�� �ߴ�. �ٷ� ���� ������
	_CrtSetReportHook(_custom_Report_hook);

	_set_purecall_handler(myPureCallHandler);
	SetHandlerDump();
}
CrashDump::~CrashDump()
{
}
void CrashDump::Crash(void)
{
	int* p = nullptr;
	*p = 0;
}
LONG __stdcall CrashDump::MyExceptionFilter(PEXCEPTION_POINTERS pExceptionPointer)
{
	//--------------------------------------------------------------------
	// �ٸ� �����忡�� �̹� ���� ������ �������� ��� Sleep
	//--------------------------------------------------------------------
	if (InterlockedExchange(&_DumpFlag, TRUE) == TRUE)
		Sleep(INFINITE);

	//--------------------------------------------------------------------
	// ���� ��¥�� �ð��� �˾ƿ´�.
	//--------------------------------------------------------------------
	SYSTEMTIME stNowTime;
	WCHAR filename[MAX_PATH];
	GetLocalTime(&stNowTime);

	swprintf(filename, MAX_PATH, L"Dump_%d%02d%02d_%02d.%02d.%02d.dmp",
		stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	wprintf_s(L"\n\n\n!!! Crash Error !!! %d.%02d.%02d / %02d:%02d:%02d\n",
		stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	wprintf_s(L"Now save dump file...\n");

	//--------------------------------------------------------------------
	// ���� ������ �����Ѵ�.
	//--------------------------------------------------------------------
	HANDLE hDumpFile = CreateFile(
		filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	//--------------------------------------------------------------------
	// ���� ���Ͽ� ���� ���μ����� ��� �޸� ���¸� �α��Ѵ�.
	//--------------------------------------------------------------------
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;
		MinidumpExceptionInformation.ThreadId = GetCurrentThreadId();
		MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		MinidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&MinidumpExceptionInformation,
			NULL,
			NULL);

		CloseHandle(hDumpFile);
		wprintf_s(L"Crash dump file save finish!\n");
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
void CrashDump::SetHandlerDump(void)
{
	SetUnhandledExceptionFilter(MyExceptionFilter);
}
void CrashDump::myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	Crash();
}
int CrashDump::_custom_Report_hook(int ireposttype, char* message, int* returnvalue)
{
	Crash();
	return true;
}
void CrashDump::myPureCallHandler(void)
{
	Crash();
}
