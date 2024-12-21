#pragma once
#include <string>
#include <ctime>
#include "GlobalConfig.hpp"
#include "RequestParser.hpp"
#include "KqueueManager.hpp"

struct ClientState {
	int fd;
	std::string requestBuffer;
	std::string responseBuffer;
	RequestParser* request;

	time_t lastActive;
	ServerConfig serverConfig;

	std::string clientIPAddress;
	int clientPort;

	bool isCgiRequest;
	int cgiInputFd;
	int cgiOutputFd;
	int cgiPid;

	ClientState() : fd(-1), request(nullptr), lastActive(std::time(NULL)), serverConfig(), isCgiRequest(false), cgiInputFd(-1), cgiOutputFd(-1), cgiPid(-1) {}

	ClientState(const ServerConfig& config, int _fd)
		: fd(_fd), request(nullptr), lastActive(std::time(NULL)), serverConfig(config), isCgiRequest(false), cgiInputFd(-1), cgiOutputFd(-1), cgiPid(-1) {}
};
