#pragma once

#include "GlobalConfig.hpp"
#include "utils.hpp"
#include "Parser.hpp"
#include "RequestParser.hpp"
#include "ResponseControl.hpp"
#include "ClientState.hpp"
#include "KqueueManager.hpp"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <map>
#include <string>
#include <signal.h>
#include <iostream>

#define PATH_MAX 1024
#define BUFFER_SIZE 2048
#define CGI_TIMEOUT 5000 // Timeout in milliseconds

class Server
{
private:
	std::vector<ServerConfig> serverConfigs;
	std::map<int, ServerConfig> serverSockets;
	RequestParser *_request;
	ResponseControl *_response;

	void setNonBlocking(int fd);
	void removeClient(int clientSocket);
	void handleAccept(int serverSocket);
	void handleRead(int clientSocket);
	void handleWrite(int clientSocket);
	void checkTimeouts();
	void processEvent(struct kevent &event);
	void createServerSocket(ServerConfig &config);
	ClientState *findClientByPipeFd(int fd);

	std::map<int, ClientState> clients;

public:
	Server();
	~Server();

	void configure(const std::string &configFilePath);
	void setup();
	void run();
	int getServerSocket() const;
};