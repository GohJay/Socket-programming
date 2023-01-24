#pragma once
#include "Define.h"
#include <Windows.h>

struct SECTOR
{
	int x;
	int y;
};

struct SECTOR_AROUND
{
	int count;
	SECTOR around[9];
};

struct CHARACTER
{
	enum STATUS
	{
		AUTH = 0,
		LOGIN,
		PLAY
	};
	STATUS status;
	DWORD64 sessionID;
	INT64 accountNo;
	WCHAR id[20];
	WCHAR nickname[20];
	char sessionKey[64];
	SECTOR sector;
};
