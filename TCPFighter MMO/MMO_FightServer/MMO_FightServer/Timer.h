#pragma once

class Timer
{
private:
	Timer();
	~Timer();
public:
	static Timer* GetInstance();
	void Update();
	DWORD GetDeltaTime();
private:
	DWORD _aftertime;
	DWORD _beforetime;
	DWORD _deltatime;
	static Timer _instance;
};
