#include "winsock_utils.h"

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
void stopTCP(SOCKET s)
{
    shutdown(s, SD_SEND); // Остановка передачи данных
    closesocket(s); // Закрытие сокета
    printf("Сокет %ld закрыт.\n", s);
}

// Передача данных
int sendTCP(SOCKET& s, const char* message)
{
    return send(s, message, strlen(message), 0); // Отправляем сообщение 'message'
}

// Получение данных
int receiveTCP(SOCKET& s, char* buffer, int bufferSize)
{
    return recv(s, buffer, bufferSize, 0);
}