#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream> // ����� ��� cerr � WinSockInit � WinSockClose

#pragma comment(lib, "ws2_32.lib")

#ifndef SD_SEND
#define SD_SEND 1
#endif

// ������������� Windows Sockets DLL
int WinSockInit();

// ���������� Windows Sockets DLL
void WinSockClose();

// ��������� �������� ������
void stopTCP(SOCKET s);

// �������� ������
int sendTCP(SOCKET& s, const char* message);

// ��������� ������
int receiveTCP(SOCKET& s, char* buffer, int bufferSize);