#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream> // Нужен для cerr в WinSockInit и WinSockClose

#pragma comment(lib, "ws2_32.lib")

#ifndef SD_SEND
#define SD_SEND 1
#endif

// Инициализация Windows Sockets DLL
int WinSockInit();

// Отключение Windows Sockets DLL
void WinSockClose();

// Остановка передачи данных
void stopTCP(SOCKET s);

// Передача данных
int sendTCP(SOCKET& s, const char* message);

// Получение данных
int receiveTCP(SOCKET& s, char* buffer, int bufferSize);