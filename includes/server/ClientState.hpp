#pragma once
#include <string>
#include <ctime>
#include "GlobalConfig.hpp"

struct ClientState {
    std::string requestBuffer;
    std::string responseBuffer;
    std::string clientIPAddress;
    int clientPort;

    time_t lastActive;
    ServerConfig serverConfig;

    ClientState()
        : lastActive(std::time(NULL)), serverConfig() {}

    ClientState(const ServerConfig& config)
        : lastActive(std::time(NULL)), serverConfig(config) {}
};
