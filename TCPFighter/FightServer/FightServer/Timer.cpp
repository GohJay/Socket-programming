#include "stdafx.h"
#include "Timer.h"
#pragma comment(lib, "Winmm.lib")

Timer Timer::_instance;
Timer::Timer()
{
	_aftertime = timeGetTime();
}
Timer::~Timer()
{
}
Timer * Timer::GetInstance()
{
	return &_instance;
}
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
