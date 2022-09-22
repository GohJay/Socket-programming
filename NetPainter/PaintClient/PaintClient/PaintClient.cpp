#include "stdafx.h"
#include "PaintClient.h"
#include "../../Common/Protocol.h"
#include "../../Network/include/RingBuffer.h"
#include <windowsx.h>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../../Network/lib/Release/Network.lib")

#define NAME_TITLE			L"PaintClient"
#define NAME_CLASS			L"Paint"
#define ERROR(err)			Error(__FILE__, __FUNCTION__, __LINE__, err)

#define WM_NETWORK			WM_USER + 1
#define WM_MESSAGE			WM_NETWORK + 1
#define UM_DISCONNECT		1

HWND	g_hMainWnd;
HPEN	g_hGridPen;
BOOL	g_bDrag;
INT		g_StartX;
INT		g_StartY;

SOCKET	g_Socket;
BOOL	g_bConnected;
BOOL	g_SendFlag;
Jay::RingBuffer	g_SendBuffer;
Jay::RingBuffer	g_RecvBuffer;

VOID Error(const char* file, const char* func, int line, int err);
BOOL Init(HWND hWnd);
BOOL Connect(PWCHAR ipaddress, INT port);
VOID Disconnect();
VOID RecvEvent();
VOID WriteEvent();
VOID Update(int EndX, int EndY);
VOID Render(HDC hDC);
VOID RenderLine(HDC hdc, int StartX, int StartY, int EndX, int EndY);
VOID Network(WPARAM wParam, LPARAM lParam);
VOID Message(WPARAM wParam, LPARAM lParam);

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

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
	
	// TODO: Place code here.
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINTCLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_PAINTCLIENT);
	wcex.lpszClassName = NAME_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	// Perform application initialization:
	g_hMainWnd = CreateWindowW(NAME_CLASS, NAME_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);
	if (!g_hMainWnd)
		return FALSE;
	
	ShowWindow(g_hMainWnd, nCmdShow);
	UpdateWindow(g_hMainWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDI_PAINTCLIENT));

	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WSACleanup();
	return (int)msg.wParam;
}
VOID Error(const char * file, const char * func, int line, int err)
{
	printf_s("%s::%s() line: %d, error: %d\n", file, func, line, err);
}
BOOL Init(HWND hWnd)
{
	g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_Socket == INVALID_SOCKET)
	{
		ERROR(WSAGetLastError());
		return FALSE;
	}
	int ret = WSAAsyncSelect(g_Socket, hWnd, WM_NETWORK, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
	if (ret == SOCKET_ERROR)
	{
		ERROR(WSAGetLastError());
		closesocket(g_Socket);
		return FALSE;
	}
	ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLGCONNECT), NULL, DlgProc);
	if (ret == 0)
	{
		ERROR(ret);
		closesocket(g_Socket);
		return FALSE;
	}
	return TRUE;
}
BOOL Connect(PWCHAR ipaddress, INT port)
{
	SOCKADDR_IN svrAddr = {};
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_port = htons(port);
	InetPton(AF_INET, ipaddress, &svrAddr.sin_addr);
	int ret = connect(g_Socket, (SOCKADDR*)&svrAddr, sizeof(svrAddr));
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			ERROR(err);
			return FALSE;
		}
	}
	return TRUE;
}
VOID Disconnect()
{
	closesocket(g_Socket);
	SendMessage(g_hMainWnd, WM_MESSAGE, UM_DISCONNECT, NULL);
}
VOID RecvEvent()
{
	int err;
	char buffer[512];
	int size = recv(g_Socket, buffer, sizeof(buffer), 0);
	switch (size)
	{
	case SOCKET_ERROR:
		err = WSAGetLastError();
		if (err != WSAECONNRESET && err != WSAECONNABORTED)
			ERROR(err);
	case 0:
		Disconnect();
		return;
	default:
		break;
	}

	int ret = g_RecvBuffer.Enqueue(buffer, size);
	if (ret != size)
	{
		ERROR(ret);
		Disconnect();
		return;
	}
	InvalidateRect(g_hMainWnd, NULL, FALSE);
}
VOID WriteEvent()
{
	g_SendFlag = TRUE;

	char buffer[1024];
	int size = g_SendBuffer.Peek(buffer, sizeof(buffer));
	if (size <= 0)
		return;

	int ret = send(g_Socket, buffer, size, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		switch (err)
		{
		case WSAEWOULDBLOCK:
			g_SendFlag = FALSE;
			return;
		case WSAECONNABORTED:
		case WSAECONNRESET:
			break;
		default:
			ERROR(err);
			break;
		}
		Disconnect();
		return;
	}
	g_SendBuffer.MoveFront(ret);
}
VOID Update(int EndX, int EndY)
{
	stHEADER header;
	st_DRAW_PACKET data;
	int headerSize = sizeof(header);
	int dataSize = sizeof(data);
	header.Len = dataSize;
	data.iStartX = g_StartX;
	data.iStartY = g_StartY;
	data.iEndX = EndX;
	data.iEndY = EndY;
	int size = g_SendBuffer.Enqueue((char*)&header, headerSize);
	if (size != headerSize)
	{
		ERROR(size);
		Disconnect();
		return;
	}
	size = g_SendBuffer.Enqueue((char*)&data, dataSize);
	if (size != dataSize)
	{
		ERROR(size);
		Disconnect();
		return;
	}
	if (g_SendFlag)
		WriteEvent();
}
VOID Render(HDC hDC)
{
	stHEADER header;
	st_DRAW_PACKET data;
	int headerSize = sizeof(stHEADER);
	HPEN hOldPen = (HPEN)SelectObject(hDC, g_hGridPen);
	for (;;)
	{
		if (g_RecvBuffer.GetUseSize() <= headerSize)
			break;

		int ret = g_RecvBuffer.Peek((char*)&header, headerSize);
		if (ret != headerSize)
		{
			ERROR(ret);
			CRASH;
		}

		if (g_RecvBuffer.GetUseSize() < headerSize + header.Len)
			break;

		g_RecvBuffer.MoveFront(headerSize);
		ret = g_RecvBuffer.Dequeue((char*)&data, header.Len);
		if (ret != header.Len)
		{
			ERROR(ret);
			CRASH;
		}

		RenderLine(hDC, data.iStartX, data.iStartY, data.iEndX, data.iEndY);
	}
	SelectObject(hDC, hOldPen);
}
VOID RenderLine(HDC hdc, int StartX, int StartY, int EndX, int EndY)
{
	MoveToEx(hdc, StartX, StartY, NULL);
	LineTo(hdc, EndX, EndY);
}
VOID Network(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		ERROR(WSAGETSELECTERROR(lParam));
		Disconnect();
		return;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		g_bConnected = TRUE;
		break;
	case FD_READ:
		RecvEvent();
		break;
	case FD_WRITE:
		WriteEvent();
		break;
	case FD_CLOSE:
		g_bConnected = FALSE;
		Disconnect();
		break;
	default:
		break;
	}
}
VOID Message(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case UM_DISCONNECT:
		MessageBox(NULL, L"서버와 연결이 종료되었습니다.\n프로그램을 종료합니다.", L"PaintClient", MB_OK);
		DestroyWindow(g_hMainWnd);
		break;
	default:
		break;
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		g_hGridPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		if (!Init(hWnd))
			DestroyWindow(hWnd);
		break;
	case WM_NETWORK:
		Network(wParam, lParam);
		break;
	case WM_MESSAGE:
		Message(wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
		g_StartX = GET_X_LPARAM(lParam);
		g_StartY = GET_Y_LPARAM(lParam);
		g_bDrag = TRUE;
		break;
	case WM_LBUTTONUP:
		g_bDrag = FALSE;
		break;
	case WM_MOUSEMOVE:
		if (g_bConnected && g_bDrag)
		{
			int EndX = GET_X_LPARAM(lParam);
			int EndY = GET_Y_LPARAM(lParam);
			Update(EndX, EndY);
			g_StartX = EndX;
			g_StartY = EndY;
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			Render(hdc);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		DeleteObject(g_hGridPen);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_EDITIP, L"127.0.0.1");
		SetDlgItemText(hWnd, IDC_EDITPORT, L"25000");
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCONNECT:
				WCHAR ip[16];
				WCHAR port[8];
				GetDlgItemText(hWnd, IDC_EDITIP, ip, sizeof(ip) / 2);
				GetDlgItemText(hWnd, IDC_EDITPORT, port, sizeof(port) / 2);
				if (Connect(ip, _wtoi(port)))
					EndDialog(hWnd, 1);
				break;
			case IDCANCEL:
				EndDialog(hWnd, 0);
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	return FALSE;
}
