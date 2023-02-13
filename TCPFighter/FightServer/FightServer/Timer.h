#pragma once

class Timer
{
public:
	static void Update();
	static DWORD GetDeltaTime();
private:
	static DWORD _aftertime;
	static DWORD _beforetime;
	static DWORD _deltatime;
};
