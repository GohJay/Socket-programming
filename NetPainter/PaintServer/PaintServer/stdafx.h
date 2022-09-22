// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows Header Files:
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>

// TODO: reference additional headers your program requires here
//#include "MyNew.h"
//#pragma comment(lib, "MyNew.lib")