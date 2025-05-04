#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "winsock_utils.h"
#include "ping_utils.h"

using namespace std;
using namespace winsock_utils;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    PingParameters config;

    config.packet_count = 8;
    config.interval = 0.1f;

    if (argc != 2)
    {
        cerr << "Использование: ping <имя_хоста>\n";
        return 1;
    }

    const char* hostname = argv[1];

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
        addr.sin_port = htons(config.talk_port);
        addr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];

        // Преобразовать IP-адрес в строку для вывода
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);

        // Создать сокет
        SOCKET sock = socketTCP();

        // Установить время ожидания ответа
        int timeout_ms = static_cast<int>(config.timeout * 1000); // Преобразуем в миллисекунды
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));

        // Подключиться к серверу
        connectTCP(sock, addr);

        // Отправить пакеты и получить ответы
        cout << "Обмен пакетами с " << hostname << " [" << ip_str << "] с " << config.data_size << " байтами данных:" << endl;
        
        for (size_t i = 0; i < config.packet_count; ++i)
        {
            int ttl = 0; // TTL value

            int rtt = tcpPing(sock, config.data_size, ttl);
            if (rtt != -1) 
            {
                cout << "Ответ от " << ip_str << ": число байт="<< config.data_size << " время=" << rtt << "мс TTL=" << ttl << endl;
            }
            this_thread::sleep_for(chrono::duration<float>(config.interval));
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