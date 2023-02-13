#include "stdafx.h"
#include "Timer.h"
#pragma comment(lib, "Winmm.lib")

DWORD Timer::_aftertime = timeGetTime();
DWORD Timer::_beforetime;
DWORD Timer::_deltatime;

void Timer::Update()
{
	_beforetime = _aftertime;
	_aftertime = timeGetTime();
	_deltatime = _aftertime - _beforetime;
}
DWORD Timer::GetDeltaTime()
{
	return _deltatime;
}
