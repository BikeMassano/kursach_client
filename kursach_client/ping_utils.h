#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <stdexcept>
#include "winsock_utils.h"

// Структура конфигурации
struct PingParameters
{
    short talk_port = 80;
    int data_size = 32;
    int packet_count = 4;
    float interval = 1.0f;
    float timeout = 5.0f;
    bool continuous = false;
};

int tcpPing(int& packet_size, int& ttl, float& timeout, sockaddr_in& addr);