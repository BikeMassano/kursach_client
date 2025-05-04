#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#ifndef SD_SEND
#define SD_SEND 1
#endif

namespace winsock_utils 
{
	// ������������� Windows Sockets DLL
	int WinSockInit();

	// ���������� Windows Sockets DLL
	void WinSockClose();

	// ��������� �������� ������
	void stopTCP(SOCKET& s);

	// ������� �����
	SOCKET socketTCP();

	// ������������ � �������
	void connectTCP(SOCKET& s, sockaddr_in& addr);

	// �������� ������
	int sendTCP(SOCKET& s, const char* message);

	// ��������� ������
	int receiveTCP(SOCKET& s, char* buffer, int bufferSize);
}
