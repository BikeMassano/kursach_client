#include "ping_utils.h"

using namespace winsock_utils;

int tcpPing(SOCKET& sock, int& packet_size, int& ttl)
{
    // Выделить буфер
    char* buffer = new char[packet_size+1];

    try
    {
        // Создать пакет данных
        std::string data(packet_size, 'A');

        // Запустить таймер
        auto start_time = std::chrono::high_resolution_clock::now();

        // Отправить данные
        int bytes_sent = sendTCP(sock, data.c_str());

        // Получить ответ
        int bytes_received = receiveTCP(sock, buffer, packet_size);

        // Остановить таймер
        auto end_time = std::chrono::high_resolution_clock::now();

        // Вычислить RTT (Round Trip Time)
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        int rtt = static_cast<int>(duration.count());

        // Получение TTL (Time To Live)
        int len = sizeof(ttl);
        if (getsockopt(sock, IPPROTO_IP, IP_TTL, (char*)&ttl, &len) == SOCKET_ERROR)
        {
            std::cerr << "Ошибка при получении TTL: " << WSAGetLastError() << std::endl;
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
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        delete[] buffer;
        stopTCP(sock);
        return -1;
    }
}
