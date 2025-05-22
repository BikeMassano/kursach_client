#include "ping_utils.h"

using namespace winsock_utils;

int tcpPing(int& packet_size, int& ttl, float& timeout, sockaddr_in& addr)
{
    // �������� �����
    char* buffer = new char[packet_size+1];
    
    SOCKET sock;

    try
    {
        // ������� �����
        sock = socketTCP();

        // ���������� ����� �������� ������
        int timeout_ms = static_cast<int>(timeout * 1000); // ����������� � ������������
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));

        // ������������ � �������
        connectTCP(sock, addr);

        // ������� ����� ������
        std::string data(packet_size, 'A');

        // ��������� ������
        auto start_time = std::chrono::high_resolution_clock::now();

        // ��������� ������
        int bytes_sent = sendTCP(sock, data.c_str());

        // �������� �����
        int bytes_received = receiveTCP(sock, buffer, packet_size);

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
