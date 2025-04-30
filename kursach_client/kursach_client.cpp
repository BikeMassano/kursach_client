#include <stdio.h>
#include <conio.h>
#include <winsock.h>
#include <process.h>
#include <cstdlib>
#include <iostream>
#include <windows.h>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

#ifndef SD_SEND
#define SD_SEND 1
#endif

using namespace std;

const short TalkPort = 8080; // Порт сокета
const int DEFAULT_DATA_SIZE = 32;    // Размер данных по умолчанию в ICMP пакете
const int DEFAULT_PACKET_COUNT = 4; // Количество передаваемых пакетов по умолчанию
const float DEFAULT_INTERVAL = 1.0f; // Интервал между отправкой пакетов по умолчанию

int DATA_SIZE; // Размер данных в ICMP пакете
int PACKET_COUNT; // Количество передаваемых пакетов
float INTERVAL; // Интервал между отправкой пакетов (в секундах)

// Инициализация Windows Sockets DLL
int WinSockInit()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 0); /* Требуется WinSock ver 2.0*/
    printf("Запуск Winsock...\n");
    // Проинициализируем Dll
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        printf("\nОшибка: Не удалось найти работоспособную Winsock Dll\n");
        return 1;
    }
    // Проверка версии Dll
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0)
    {
        printf("\nОшибка: Не удалось найти работоспособную WinSock DLL\n");
        WSACleanup(); // Отключение Windows Sockets DLL
        return 1;
    }
    //printf(" Winsock запущен.\n");
    return 0;
}

// Отключение Windows Sockets DLL
void WinSockClose()
{
    WSACleanup();
    printf("Winsock закрыт...\n");
}

// Остановка передачи данных
void stopTCP(SOCKET s)
{
    shutdown(s, SD_SEND); // Остановка передачи данных
    closesocket(s); // Закрытие сокета
    printf("Сокет %ld закрыт.\n", s);
}

// Передача данных
int sendTCP(SOCKET s, const char* message)
{
    return send(s, message, strlen(message), 0); // Отправляем сообщение 'message'
}

// Получение данных
int receiveTCP(SOCKET s, char* buffer, int bufferSize)
{
    return recv(s, buffer, bufferSize, 0);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    DATA_SIZE = DEFAULT_DATA_SIZE; // Инициализация размера данных по умолчанию
    PACKET_COUNT = DEFAULT_PACKET_COUNT; // Инициализация количества передаваемых пакетов по умолчанию
    INTERVAL = DEFAULT_INTERVAL; // Инициализация интервала между отправкой пакетов по умолчанию

    if (argc != 2)
    {
        cerr << "Использование: ping <имя_хоста>\n";
        return 1;
    }

    if (WinSockInit() != 0)
    {
        return 1;
    }

    const char* hostname = argv[1];

    // Преобразование имени хоста в IP-адрес
    hostent* host = gethostbyname(hostname);
    if (host == NULL)
    {
        cerr << "Ошибка: Не удалось разрешить имя хоста.\n";
        WinSockClose();
        return 1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TalkPort);
    addr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];

    // Создание TCP сокета
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        cerr << "Ошибка: Не удалось создать сокет. Код ошибки: " << WSAGetLastError() << endl;
        WinSockClose();
        return 1;
    }

    // Установка времени ожидания ответа
    int timeout = 1000; // миллисекунды
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // Подключение к серверу
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cerr << "Ошибка: Не удалось подключиться к серверу. Код ошибки: " << WSAGetLastError() << endl;
        stopTCP(sock);
        WinSockClose();
        return 1;
    }

    char* data = new char[DATA_SIZE + 1]; // Выделите на 1 байт больше для '\0'
    memset(data, 'A', DATA_SIZE);
    data[DATA_SIZE] = '\0';  // Добавьте нулевой символ

    cout << "Обмен пакетами с " << hostname << " [" << inet_ntoa(*(in_addr*)host->h_addr_list[0]) << "] с " << DATA_SIZE << " байтами данных:" << endl;

    for (size_t i = 0; i < PACKET_COUNT; i++)
    {
        clock_t start_time;

        // Отправка данных
        start_time = clock(); // Запоминаем время отправки
        int bytes_sent = sendTCP(sock, data);
        if (bytes_sent == SOCKET_ERROR)
        {
            cerr << "Ошибка: Не удалось отправить данные. Код ошибки: " << WSAGetLastError() << endl;
            stopTCP(sock);
            WinSockClose();
            delete[] data;
            return 1;
        }

        // Получение данных
        char recv_buf[1024];
        int bytes_received = receiveTCP(sock, recv_buf, sizeof(recv_buf));
        if (bytes_received == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT)
            {
                cout << "Запрос превысил время ожидания." << endl;
            }
            else
            {
                cerr << "Ошибка: Не удалось получить ответ. Код ошибки: " << WSAGetLastError() << endl;
            }
        }
        else
        {
            clock_t end_time = clock(); // Запоминаем время получения
            double rtt = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000.0; // Время в миллисекундах

            cout << "Ответ от " << inet_ntoa(*(in_addr*)host->h_addr_list[0]) << ": число байт=" << bytes_received
                << " время=" << rtt << "мс" << endl;
        }

        // Ожидание по интервалу
        Sleep(INTERVAL * 1000);
    }

    stopTCP(sock);
    WinSockClose();
    delete[] data;

    return 0;
}