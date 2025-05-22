#include "ping_utils.h"

using namespace winsock_utils;

int tcpPing(int& packet_size, int& ttl, float& timeout, sockaddr_in& addr)
{
    // Выделить буфер
    char* buffer = new char[packet_size+1];
    
    SOCKET sock;

    try
    {
        // Создать сокет
        sock = socketTCP();

        // Установить время ожидания ответа
        int timeout_ms = static_cast<int>(timeout * 1000); // Преобразуем в миллисекунды
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));

        // Подключиться к серверу
        connectTCP(sock, addr);

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
