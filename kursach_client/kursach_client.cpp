#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "winsock_utils.h"
#include "ping_utils.h"

using namespace std;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    PingParameters config;

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

        // Отправить пакеты и получить ответы
        cout << "Обмен пакетами с " << hostname << " [" << ip_str << "] с " << config.data_size << " байтами данных:" << endl;
        for (size_t i = 0; i < config.packet_count; ++i)
        {
            int ttl = 0; // TTL value
            int rtt = tcpPing(addr, config.timeout, config.data_size, ttl);
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