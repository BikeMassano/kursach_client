#include "winsock_utils.h"

// ������������� Windows Sockets DLL
int WinSockInit() 
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        std::cerr << "������: �� ������� ����� ��������������� Winsock Dll" << std::endl;
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        std::cerr << "������: �� ������� ����� ��������������� Winsock Dll" << std::endl;
        WSACleanup(); // ���������� Windows Sockets DLL
        return 1;
    }

    return 0;
}

// ���������� Windows Sockets DLL
void WinSockClose()
{
    WSACleanup();
    std::cout << "������� Winsock ���������." << std::endl;
}

// ��������� �������� ������
void stopTCP(SOCKET s)
{
    shutdown(s, SD_SEND); // ��������� �������� ������
    closesocket(s); // �������� ������
    printf("����� %ld ������.\n", s);
}

// �������� ������
int sendTCP(SOCKET& s, const char* message)
{
    return send(s, message, strlen(message), 0); // ���������� ��������� 'message'
}

// ��������� ������
int receiveTCP(SOCKET& s, char* buffer, int bufferSize)
{
    return recv(s, buffer, bufferSize, 0);
}