#include "winsock_utils.h"

namespace winsock_utils
{
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
    void stopTCP(SOCKET& s)
    {
        shutdown(s, SD_SEND); // ��������� �������� ������
        closesocket(s); // �������� ������
        printf("����� %ld ������.\n", s);
    }

    SOCKET socketTCP()
    {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            throw std::runtime_error("socket ���������� � �������: " + std::to_string(WSAGetLastError()));
        }
        return sock;
    }

    // ����������� � �������
    void connectTCP(SOCKET& s, sockaddr_in& addr)
    {
        // ������������ � �������
        if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::runtime_error("connect ���������� � �������: " + std::to_string(WSAGetLastError()));
        }
    }

    // �������� ������
    int sendTCP(SOCKET& s, const char* message)
    {
        int bytes_sent = send(s, message, strlen(message), 0); // ���������� ��������� 'message'
        if (bytes_sent == SOCKET_ERROR)
        {
            throw std::runtime_error("�������� ������ ����������� � �������: " + std::to_string(WSAGetLastError()));
        }

        return bytes_sent;
    }

    // ��������� ������
    int receiveTCP(SOCKET& s, char* buffer, int bufferSize)
    {
        int bytes_received = recv(s, buffer, bufferSize, 0);

        if (bytes_received == SOCKET_ERROR)
        {
            throw std::runtime_error("��������� ������ ����������� � �������: " + std::to_string(WSAGetLastError()));
        }

        buffer[bytes_received] = '\0';

        return bytes_received;
    }
}