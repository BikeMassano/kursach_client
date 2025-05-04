#include "ping_utils.h"

int tcpPing(sockaddr_in& addr, float& timeout, int& packet_size, int& ttl)
{
    SOCKET sock = INVALID_SOCKET;
    char* buffer = nullptr;

    try
    {
        // �������� �����
        buffer = new char[packet_size];

        // ������� �����
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            throw std::runtime_error("socket ���������� � �������: " + std::to_string(WSAGetLastError()));
        }

        // ���������� ����� �������� ������
        int timeout_ms = static_cast<int>(timeout * 1000); // ����������� � ������������
        // ���������� ����� �������� ������
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        // ������������ � �������
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::runtime_error("connect ���������� � �������: " + std::to_string(WSAGetLastError()));
        }

        // ������� ����� ������
        std::string data(packet_size, 'A');

        // ��������� ������
        auto start_time = std::chrono::high_resolution_clock::now();

        // ��������� ������
        int bytes_sent = sendTCP(sock, data.c_str());
        if (bytes_sent == SOCKET_ERROR)
        {
            throw std::runtime_error("�������� ������ ����������� � �������: " + std::to_string(WSAGetLastError()));
        }

        // �������� �����
        int bytes_received = receiveTCP(sock, buffer, packet_size);
        if (bytes_received == SOCKET_ERROR)
        {
            throw std::runtime_error("��������� ������ ����������� � �������: " + std::to_string(WSAGetLastError()));
        }

        // ���������� ������
        auto end_time = std::chrono::high_resolution_clock::now();

        // ��������� RTT (Round Trip Time)
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        int rtt = static_cast<int>(duration.count());

        // ��������� TTL (Time To Live)
        int len = sizeof(ttl);
        if (getsockopt(sock, IPPROTO_IP, IP_TTL, (char*)&ttl, &len) == SOCKET_ERROR)
        {
            std::cerr << "������ ��� ��������� TTL: " << WSAGetLastError() << std::endl;
            ttl = -1; // Indicate error getting TTL
        }

        // ������� ����� �� ������� �� �������
        /*string received_string(buffer, bytes_received);

        cout << "�������� " << bytes_received << " ���� �� �������: " << received_string << endl;*/

        // ������� �� ������� ����� ������� �������� � ���������� ������

        // �������� �����
        delete[] buffer;

        return rtt;
    }
    catch (const std::exception& e)
    {
        std::cerr << "������: " << e.what() << std::endl;
        delete[] buffer;
        stopTCP(sock);
        return -1;
    }
}
