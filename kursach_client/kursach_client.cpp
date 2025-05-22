#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "winsock_utils.h"
#include "ping_utils.h"
#include "CLI11.hpp"
#include <string>

using namespace std;
using namespace winsock_utils;

int main(int argc, char* argv[])
{
    // Русский язык
    setlocale(LC_ALL, "rus");

    // Конфигурация приложения
    PingParameters config;
    string hostname;

    // Добавляем позиционные параметры
    CLI::App app;

    app.add_option("hostname", hostname, "Адрес хоста")->required();
    app.add_flag("-t", config.continuous, "Проверяет связь с указанным узлом до прекращения.");
    app.add_option("-p", config.talk_port, "Порт для использования TCP ping");
    app.add_option("-l", config.data_size, "Размер буфера отправки.");
    app.add_option("-n", config.packet_count, "Число отправляемых запросов проверки связи.");
    app.add_option("-i", config.interval, "Интервал между отправкой пакетов (in seconds)");
    app.add_option("-w", config.timeout, "Задает время ожидания каждого ответа (в секундах)");


    CLI11_PARSE(app, argc, argv);

    try
    {
        // Инициализация Windows Sockets DLL
        if (WinSockInit() != 0)
        {
            return 1;
        }

        // Преобразование имени хоста в IP-адрес
        hostent* host = gethostbyname(hostname.c_str());
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
        
        // Для бесконечной отправки пакетов
        if (config.continuous)
        {
            // Бесконечный цикл пинга, пока не будет получен сигнал прерывания
            while (true)
            {
                int ttl = 0; // TTL value

                int rtt = tcpPing(config.data_size, ttl, config.timeout, addr);
                if (rtt != -1)
                {
                    cout << "Ответ от " << ip_str << ": число байт=" << config.data_size << " время=" << rtt << "мс TTL=" << ttl << endl;
                }
                this_thread::sleep_for(chrono::duration<float>(config.interval));
            }
        }
        // Для отправки ограниченного количества пакетов
        else
        {
            for (size_t i = 0; i < config.packet_count; ++i)
            {
                int ttl = 0; // TTL value

                int rtt = tcpPing(config.data_size, ttl, config.timeout, addr);
                if (rtt != -1) 
                {
                    cout << "Ответ от " << ip_str << ": число байт="<< config.data_size << " время=" << rtt << "мс TTL=" << ttl << endl;
                }
                this_thread::sleep_for(chrono::duration<float>(config.interval));
            }
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