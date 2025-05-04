#include "ping_utils.h"

int tcpPing(sockaddr_in& addr, float& timeout, int& packet_size, int& ttl)
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
            throw std::runtime_error("socket завершился с ошибкой: " + std::to_string(WSAGetLastError()));
        }

        // Установить время ожидания ответа
        int timeout_ms = static_cast<int>(timeout * 1000); // Преобразуем в миллисекунды
        // Установить время ожидания ответа
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        // Подключиться к серверу
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::runtime_error("connect завершился с ошибкой: " + std::to_string(WSAGetLastError()));
        }

        // Создать пакет данных
        std::string data(packet_size, 'A');

        // Запустить таймер
        auto start_time = std::chrono::high_resolution_clock::now();

        // Отправить данные
        int bytes_sent = sendTCP(sock, data.c_str());
        if (bytes_sent == SOCKET_ERROR)
        {
            throw std::runtime_error("Отправка данных завершилась с ошибкой: " + std::to_string(WSAGetLastError()));
        }

        // Получить ответ
        int bytes_received = receiveTCP(sock, buffer, packet_size);
        if (bytes_received == SOCKET_ERROR)
        {
            throw std::runtime_error("Получение данных завершилось с ошибкой: " + std::to_string(WSAGetLastError()));
        }

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
