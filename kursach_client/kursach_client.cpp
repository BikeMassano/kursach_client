#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#ifndef SD_SEND
#define SD_SEND 1
#endif

using namespace std;

const short TalkPort = 80; // Порт сокета (HTTP)

const int DEFAULT_DATA_SIZE = 32;    // Размер данных по умолчанию в ICMP пакете
const int DEFAULT_PACKET_COUNT = 4; // Количество передаваемых пакетов по умолчанию
const float DEFAULT_INTERVAL = 1.0f; // Интервал между отправкой пакетов по умолчанию

int DATA_SIZE = DEFAULT_DATA_SIZE; // Размер данных в ICMP пакете
int PACKET_COUNT = DEFAULT_PACKET_COUNT; // Количество передаваемых пакетов
float INTERVAL = DEFAULT_INTERVAL; // Интервал между отправкой пакетов (в секундах)

// Инициализация Windows Sockets DLL
int WinSockInit()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    // Проинициализируем Dll
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        cerr << "Ошибка: Не удалось найти работоспособную Winsock Dll" << endl;
        return 1;
    }
    // Проверка версии Dll
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cerr << "Ошибка: Не удалось найти работоспособную Winsock Dll" << endl;
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
    cout << "Очистка Winsock завершена." << endl;
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

int tcpPing(sockaddr_in addr, int timeout_ms, int packet_size, int& ttl)
{
    SOCKET sock = INVALID_SOCKET;
    char* buffer = nullptr;

    try
    {
        // Выделить буфер
        buffer = new char[packet_size];

        // Создать сокет
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            throw runtime_error("socket завершился с ошибкой: " + to_string(WSAGetLastError()));
        }

        // Установить время ожидания ответа
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));

        // Подключиться к серверу
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            throw runtime_error("connect завершился с ошибкой: " + to_string(WSAGetLastError()));
        }

        // Создать пакет данных
        string data(packet_size, 'A');

        // Запустить таймер
        auto start_time = chrono::high_resolution_clock::now();

        // Отправить данные
        int bytes_sent = sendTCP(sock, data.c_str());
        if (bytes_sent == SOCKET_ERROR)
        {
            throw runtime_error("Отправка данных завершилась с ошибкой: " + to_string(WSAGetLastError()));
        }

        // Получить ответ
        int bytes_received = receiveTCP(sock, buffer, packet_size);
        if (bytes_received == SOCKET_ERROR)
        {
            throw runtime_error("Получение данных завершилось с ошибкой: " + to_string(WSAGetLastError()));
        }

        // Остановить таймер
        auto end_time = chrono::high_resolution_clock::now();

        // Вычислить RTT (Round Trip Time)
        chrono::duration<double, milli> duration = end_time - start_time;
        int rtt = static_cast<int>(duration.count());

        // Получение TTL (Time To Live)
        int len = sizeof(ttl);
        if (getsockopt(sock, IPPROTO_IP, IP_TTL, (char*)&ttl, &len) == SOCKET_ERROR)
        {
            cerr << "Ошибка при получении TTL: " << WSAGetLastError() << endl;
            ttl = -1; // Indicate error getting TTL
        }

        // Вывести ответ от сервера на консоль
        /*string received_string(buffer, bytes_received);

        cout << "Получено " << bytes_received << " байт от клиента: " << received_string << endl;*/

        // Разница во времени между началом отправки и получением ответа

        // Очистить буфер
        delete[] buffer;

        return rtt;
    }
    catch (const exception& e) 
    {
        cerr << "Ошибка: " << e.what() << endl;
        delete[] buffer;
        stopTCP(sock);
        return -1;
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");

    char a[] = "localhost";
    argv[1] = a;
    const char* hostname = argv[1];

    /*if (argc != 2)
    {
        cerr << "Использование: ping <имя_хоста>\n";
        return 1;
    }*/

    try
    {
        if (WinSockInit() != 0)
        {
            return 1;
        }

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

        // Преобразовать IP-адрес в строку для вывода
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);

        // Отправить пакеты и получить ответы
        cout << "Обмен пакетами с " << hostname << " [" << ip_str << "] с " << DATA_SIZE << " байтами данных:" << endl;
        for (size_t i = 0; i < PACKET_COUNT; i++)
        {
            int ttl = 0; // TTL value
            int rtt = tcpPing(addr, 1000, DATA_SIZE, ttl);
            if (rtt != -1) 
            {
                cout << "Ответ от " << ip_str << ": число байт="<< DATA_SIZE << " время=" << rtt << "мс TTL=" << ttl << endl;
            }
            this_thread::sleep_for(chrono::duration<float>(INTERVAL));
        }

        WinSockClose();
        return 0;
    }
    catch (const exception& e) 
    {
        cerr << "Ошибка: " << e.what() << endl;
        WinSockClose();
        return 1;
    }

}