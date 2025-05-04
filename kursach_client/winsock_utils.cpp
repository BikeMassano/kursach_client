#include "winsock_utils.h"

namespace winsock_utils
{
    // Инициализация Windows Sockets DLL
    int WinSockInit()
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);

        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            std::cerr << "Ошибка: Не удалось найти работоспособную Winsock Dll" << std::endl;
            return 1;
        }

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
            std::cerr << "Ошибка: Не удалось найти работоспособную Winsock Dll" << std::endl;
            WSACleanup(); // Отключение Windows Sockets DLL
            return 1;
        }

        return 0;
    }

    // Отключение Windows Sockets DLL
    void WinSockClose()
    {
        WSACleanup();
        std::cout << "Очистка Winsock завершена." << std::endl;
    }

    // Остановка передачи данных
    void stopTCP(SOCKET& s)
    {
        shutdown(s, SD_SEND); // Остановка передачи данных
        closesocket(s); // Закрытие сокета
        printf("Сокет %ld закрыт.\n", s);
    }

    SOCKET socketTCP()
    {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            throw std::runtime_error("socket завершился с ошибкой: " + std::to_string(WSAGetLastError()));
        }
        return sock;
    }

    // Подключение к серверу
    void connectTCP(SOCKET& s, sockaddr_in& addr)
    {
        // Подключиться к серверу
        if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::runtime_error("connect завершился с ошибкой: " + std::to_string(WSAGetLastError()));
        }
    }

    // Передача данных
    int sendTCP(SOCKET& s, const char* message)
    {
        int bytes_sent = send(s, message, strlen(message), 0); // Отправляем сообщение 'message'
        if (bytes_sent == SOCKET_ERROR)
        {
            throw std::runtime_error("Отправка данных завершилась с ошибкой: " + std::to_string(WSAGetLastError()));
        }

        return bytes_sent;
    }

    // Получение данных
    int receiveTCP(SOCKET& s, char* buffer, int bufferSize)
    {
        int bytes_received = recv(s, buffer, bufferSize, 0);

        if (bytes_received == SOCKET_ERROR)
        {
            throw std::runtime_error("Получение данных завершилось с ошибкой: " + std::to_string(WSAGetLastError()));
        }

        buffer[bytes_received] = '\0';

        return bytes_received;
    }
}