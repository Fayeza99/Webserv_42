#pragma once
#include <string>
#include <ctime>
#include "GlobalConfig.hpp"

struct ClientState {
	std::string requestBuffer;
	std::string responseBuffer;

	time_t lastActive;
	ServerConfig serverConfig;

	std::string clientIPAddress;
	int clientPort;

	bool isCgiRequest;
	int cgiInputFd;
	int cgiOutputFd;


	ClientState()
		: lastActive(std::time(NULL)), serverConfig(), isCgiRequest(false), cgiInputFd(-1), cgiOutputFd(-1) {}

	ClientState(const ServerConfig& config)
		: lastActive(std::time(NULL)), serverConfig(config), isCgiRequest(false), cgiInputFd(-1), cgiOutputFd(-1) {}
};
