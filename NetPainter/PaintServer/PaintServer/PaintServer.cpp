#include "stdafx.h"
#include "resource.h"
#include "Object.h"
#include "../../Common/Protocol.h"
#include "../../Common/HashMap.h"
#include <commctrl.h>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../../Network/lib/Release/Network.lib")

#define ERROR(err)		Error(__FILE__, __FUNCTION__, __LINE__, err)
#define WM_NETWORK		WM_USER + 1
#define WM_MESSAGE		WM_NETWORK + 1
#define UM_CONNECT		1
#define UM_DISCONNECT	UM_CONNECT + 1

HWND	g_hMainWnd;
SOCKET	g_ListenSocket;
Jay::HashMap<int, Session*> *g_SessionMap;

VOID Error(const char* file, const char* func, int line, int err);
VOID Init(HWND hWnd);
BOOL Listen(HWND hWnd);
VOID AcceptEvent();
VOID RecvEvent(SOCKET socket);
VOID WriteEvent(SOCKET socket);
VOID CloseEvent(SOCKET socket);
VOID Flush(Session* session);
VOID SendBroadcast(const char* message, int size);
VOID Disconnect(SOCKET socket);
VOID DestroyAll();
VOID Network(WPARAM wParam, LPARAM lParam);
VOID Message(WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	WSADATA wsadata;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (ret != 0)
	{
		ERROR(ret);
		return FALSE;
	}
	
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DLGMAIN), NULL, DlgProc);

	WSACleanup();
	return FALSE;
}
VOID Error(const char * file, const char * func, int line, int err)
{
	printf_s("%s::%s() line: %d, error: %d\n", file, func, line, err);
}
VOID Init(HWND hWnd)
{
	HWND hListWnd = GetDlgItem(hWnd, IDC_LISTSESSION);
	LVCOLUMN column;
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.fmt = LVCFMT_CENTER;
	column.cx = 50;
	column.pszText = L"No";
	ListView_InsertColumn(hListWnd, 0, &column);
	column.fmt = LVCFMT_CENTER;
	column.cx = 100;
	column.pszText = L"IP";
	ListView_InsertColumn(hListWnd, 1, &column);
	column.fmt = LVCFMT_CENTER;
	column.cx = 100;
	column.pszText = L"Port";
	ListView_InsertColumn(hListWnd, 2, &column);
	column.fmt = LVCFMT_CENTER;
	column.cx = 150;
	column.pszText = L"Status";
	ListView_InsertColumn(hListWnd, 3, &column);
	SetFocus(hListWnd);
}
BOOL Listen(HWND hWnd)
{
	g_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_ListenSocket == INVALID_SOCKET)
	{
		ERROR(WSAGetLastError());
		return FALSE;
	}

	linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	int ret = setsockopt(g_ListenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (ret == SOCKET_ERROR)
	{
		ERROR(WSAGetLastError());
		closesocket(g_ListenSocket);
		return FALSE;
	}

	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(25000);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(g_ListenSocket, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	if (ret == SOCKET_ERROR)
	{
		ERROR(WSAGetLastError());
		closesocket(g_ListenSocket);
		return FALSE;
	}

	ret = listen(g_ListenSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		ERROR(WSAGetLastError());
		closesocket(g_ListenSocket);
		return FALSE;
	}

	ret = WSAAsyncSelect(g_ListenSocket, hWnd, WM_NETWORK, FD_ACCEPT | FD_CLOSE);
	if (ret == SOCKET_ERROR)
	{
		ERROR(WSAGetLastError());
		closesocket(g_ListenSocket);
		return FALSE;
	}
	return TRUE;
}
VOID AcceptEvent()
{
	SOCKADDR_IN clientAddr = {};
	int clientSize = sizeof(clientAddr);
	SOCKET client = accept(g_ListenSocket, (SOCKADDR*)&clientAddr, &clientSize);
	if (client == INVALID_SOCKET)
	{
		ERROR(WSAGetLastError());
		CRASH;
	}
	
	int ret = WSAAsyncSelect(client, g_hMainWnd, WM_NETWORK, FD_WRITE | FD_READ | FD_CLOSE);
	if (ret == SOCKET_ERROR)
	{
		ERROR(WSAGetLastError());
		return;
	}

	Session *new_session = new Session;
	new_session->socket = client;
	InetNtop(AF_INET, &clientAddr.sin_addr, new_session->ip, sizeof(new_session->ip) / 2);
	new_session->port = ntohs(clientAddr.sin_port);
	new_session->sendflag = true;

	if (!g_SessionMap->insert(new_session->socket, new_session))
	{
		ERROR(-1);
		CRASH;
	}

	SendMessage(g_hMainWnd, WM_MESSAGE, UM_CONNECT, (LPARAM)new_session);
}
VOID RecvEvent(SOCKET socket)
{
	auto iter = g_SessionMap->find(socket);
	if (iter == g_SessionMap->end())
	{
		ERROR(-1);
		CRASH;
	}

	Session *session = iter.second();
	int err;
	char buffer[512];
	int size = recv(session->socket, buffer, sizeof(buffer), 0);
	switch (size)
	{		
	case SOCKET_ERROR:
		err = WSAGetLastError();
		if (err != WSAECONNRESET && err != WSAECONNABORTED)
			ERROR(err);
	case 0:
		Disconnect(session->socket);
		return;
	default:
		break;
	}
	
	int ret = session->recvBuffer.Enqueue(buffer, size);
	if (ret != size)
	{
		ERROR(ret);
		Disconnect(session->socket);
		return;
	}

	Flush(session);
}
VOID WriteEvent(SOCKET socket)
{
	auto iter = g_SessionMap->find(socket);
	if (iter == g_SessionMap->end())
	{
		ERROR(-1);
		CRASH;
	}

	Session *session = iter.second();
	session->sendflag = true;

	char buffer[1024];
	int size = session->sendBuffer.Peek(buffer, sizeof(buffer));
	if (size <= 0)
		return;

	int ret = send(session->socket, buffer, size, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		switch (err)
		{
		case WSAEWOULDBLOCK:
			session->sendflag = false;
			return;
		case WSAECONNABORTED:
		case WSAECONNRESET:
			break;
		default:
			ERROR(err);
			break;
		}
		Disconnect(session->socket);
		return;
	}
	session->sendBuffer.MoveFront(ret);
}
VOID CloseEvent(SOCKET socket)
{
	Disconnect(socket);
}
VOID Flush(Session *session)
{
	int headerSize = sizeof(stHEADER);
	int dataSize = sizeof(st_DRAW_PACKET);
	int packetSize = headerSize + dataSize;
	char* packet = new char[packetSize];
	stHEADER* header = (stHEADER*)packet;
	st_DRAW_PACKET* data = (st_DRAW_PACKET*)(packet + headerSize);
	for (;;)
	{
		if (session->recvBuffer.GetUseSize() <= headerSize)
			break;

		int ret = session->recvBuffer.Peek((char*)header, headerSize);
		if (ret != headerSize)
		{
			ERROR(ret);
			CRASH;
		}

		if (session->recvBuffer.GetUseSize() < headerSize + header->Len)
			break;

		if (header->Len != dataSize)
		{
			ERROR(header->Len);
			Disconnect(session->socket);
			break;
		}

		session->recvBuffer.MoveFront(headerSize);
		ret = session->recvBuffer.Dequeue((char*)data, header->Len);
		if (ret != header->Len)
		{
			ERROR(ret);
			CRASH;
		}

		SendBroadcast(packet, packetSize);
	}
	delete[] packet;
}
VOID SendBroadcast(const char* message, int size)
{
	for (auto iter = g_SessionMap->begin(); iter != g_SessionMap->end();)
	{
		Session *session = iter.second();
		int ret = session->sendBuffer.Enqueue(message, size);
		if (ret != size)
		{
			SendMessage(g_hMainWnd, WM_MESSAGE, UM_DISCONNECT, (LPARAM)session);
			iter = g_SessionMap->erase(iter);
			closesocket(session->socket);
			delete(session);
			continue;
		}
		if (session->sendflag)
			WriteEvent(session->socket);
		++iter;
	}
}
VOID Disconnect(SOCKET socket)
{
	auto iter = g_SessionMap->find(socket);
	if (iter != g_SessionMap->end())
	{
		Session *session = iter.second();
		SendMessage(g_hMainWnd, WM_MESSAGE, UM_DISCONNECT, (LPARAM)session);
		g_SessionMap->erase(iter);
		closesocket(session->socket);
		delete(session);
	}
}
VOID DestroyAll()
{
	for (auto iter = g_SessionMap->begin(); iter != g_SessionMap->end();)
	{
		Session *session = iter.second();
		iter = g_SessionMap->erase(iter);
		closesocket(session->socket);
		delete(session);
	}
}
VOID Network(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		ERROR(WSAGETSELECTERROR(lParam));
		Disconnect(wParam);
		return;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
		AcceptEvent();
		break;
	case FD_READ:
		RecvEvent(wParam);
		break;
	case FD_WRITE:
		WriteEvent(wParam);
		break;
	case FD_CLOSE:
		CloseEvent(wParam);
		break;
	default:
		break;
	}
}
VOID Message(WPARAM wParam, LPARAM lParam)
{
	static int iCnt;
	switch (wParam)
	{
	case UM_CONNECT:
		{
			Session* session = (Session*)lParam;
			HWND hListWnd = GetDlgItem(g_hMainWnd, IDC_LISTSESSION);
			wchar_t text[32];
			LVITEM item;
			item.mask = LVIF_TEXT;
			item.iItem = iCnt++;
			item.iSubItem = 0;
			_itow_s(item.iItem + 1, text, sizeof(text) / 2, 10);
			item.pszText = text;
			ListView_InsertItem(hListWnd, &item);
			ListView_SetItemText(hListWnd, item.iItem, 1, session->ip);
			_itow_s(session->port, text, sizeof(text) / 2, 10);
			ListView_SetItemText(hListWnd, item.iItem, 2, text);
			ListView_SetItemText(hListWnd, item.iItem, 3, L"Connect");
		}
		break;
	case UM_DISCONNECT:
		{
			Session* session = (Session*)lParam;
			HWND hListWnd = GetDlgItem(g_hMainWnd, IDC_LISTSESSION);
			wchar_t text[32];
			LVITEM item;
			item.mask = LVIF_TEXT;
			item.iItem = iCnt++;
			item.iSubItem = 0;
			_itow_s(item.iItem + 1, text, sizeof(text) / 2, 10);
			item.pszText = text;
			ListView_InsertItem(hListWnd, &item);
			ListView_SetItemText(hListWnd, item.iItem, 1, session->ip);
			_itow_s(session->port, text, sizeof(text) / 2, 10);
			ListView_SetItemText(hListWnd, item.iItem, 2, text);
			ListView_SetItemText(hListWnd, item.iItem, 3, L"Disconnect");
		}
		break;
	default:
		break;
	}
}
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		g_SessionMap = new Jay::HashMap<int, Session*>;
		Init(hWnd);
		if (!Listen(hWnd))
			DestroyWindow(hWnd);
		g_hMainWnd = hWnd;
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				EndDialog(hWnd, 0);
				break;
			default:
				break;
			}
		}
		break;
	case WM_NETWORK:
		Network(wParam, lParam);
		break;
	case WM_MESSAGE:
		Message(wParam, lParam);
		break;
	case WM_DESTROY:
		DestroyAll();
		delete g_SessionMap;
		break;
	default:
		break;
	}
	return FALSE;
}
