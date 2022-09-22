#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ERROR(err) Error(__FILE__, __FUNCTION__, __LINE__, err)
#define PORT	25000
#define COUNT	100

#include "../../Common/Protocol.h"
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32")

struct Session
{
	bool enable;
	SOCKET socket;
};
Session g_Session[200];
int g_ClientSize;

void Error(const char* file, const char* func, int line, int err);
void GetIPAddress(char* ipaddress);
void GetClientSize();
bool Init();
void DeInit();
bool ConnectAll(const char* ipaddress, int port);
void DisconnectAll();
void Send(Session* session, const char* message, int size);

void Error(const char* file, const char* func, int line, int err)
{
	printf_s("%s::%s() line: %d, error: %d\n", file, func, line, err);
}
void GetIPAddress(char* ipaddress)
{
	std::cout << "접속할 IP 주소를 입력하세요 : ";
	std::cin >> ipaddress;
}
void GetClientSize()
{
	do
	{
		std::cout << "연결할 봇 클라이언트 수를 입력하세요(1 ~ 200) : ";
		std::cin >> g_ClientSize;
	} while (g_ClientSize <= 0 || g_ClientSize > 200);
}
bool Init()
{
	WSADATA wsadata;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (ret != 0)
	{
		ERROR(ret);
		return false;
	}
	for (int i = 0; i < g_ClientSize; i++)
	{
		g_Session[i].socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (g_Session[i].socket == INVALID_SOCKET)
		{
			ERROR(WSAGetLastError());
			return false;
		}
		g_Session[i].enable = true;
	}
	return true;
}
void DeInit()
{
	WSACleanup();
}
bool ConnectAll(const char* ipaddress, int port)
{
	SOCKADDR_IN svrAddr = {};
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_port = htons(port);
	svrAddr.sin_addr.s_addr = inet_addr(ipaddress);
	for (int i = 0; i < g_ClientSize; i++)
	{
		int ret = connect(g_Session[i].socket, (SOCKADDR*)&svrAddr, sizeof(svrAddr));
		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK)
			{
				ERROR(err);
				return false;
			}
		}
	}
	return true;
}
void DisconnectAll()
{
	for (int i = 0; i < g_ClientSize; i++)
	{
		if (g_Session[i].enable)
			closesocket(g_Session[i].socket);
	}
}
void Send(Session* session, const char* message, int size)
{
	if (!session->enable)
		return;

	int ret = send(session->socket, message, size, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		switch (err)
		{
		case WSAECONNABORTED:
		case WSAECONNRESET:
			break;
		default:
			ERROR(err);
			break;
		}
		closesocket(session->socket);
		session->enable = false;
	}
}
int main()
{
	char ip[16];
	GetIPAddress(ip);
	GetClientSize();

	if (!Init())
		return 0;

	if (!ConnectAll(ip, PORT))
		return 0;

	int offset = sizeof(stHEADER);
	int packetSize = offset + sizeof(st_DRAW_PACKET);
	char packet[sizeof(stHEADER) + sizeof(st_DRAW_PACKET)];
	stHEADER* header = (stHEADER*)packet;
	st_DRAW_PACKET* data = (st_DRAW_PACKET*)(packet + offset);

	printf_s("[PaintClient] DummyTest Start\n");
	for (int i = 1; i <= COUNT; i++)
	{
		for (int j = 0; j < g_ClientSize; j++)
		{
			header->Len = sizeof(st_DRAW_PACKET);
			data->iStartX = rand() % 400;
			data->iStartY = rand() % 400;
			data->iEndX = rand() % 400;
			data->iEndY = rand() % 400;
			Send(&g_Session[j], packet, packetSize);
		}
		printf_s("[PaintClient] DummyTest Count: %d/%d\n", i, COUNT);
		Sleep(500);
	}
	printf_s("[PaintClient] DummyTest End\n");

	DisconnectAll();
	DeInit();
}
