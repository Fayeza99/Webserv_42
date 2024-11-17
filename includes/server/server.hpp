#pragma once
#include "../../srcs/parsing/GlobalConfig.hpp"
#include "utils.hpp"
#include "../../srcs/parsing/Parser.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "server.hpp"
#include <cstring>
#include <arpa/inet.h>

class Server {
	private:
		GlobalConfig globalConfig;
		ServerConfig serverConfig;
		int serverSocket;
		struct sockaddr_in serverAddr;

	public:
		Server();
		void configure(const std::string& configFilePath);
		void setup();
		void run();
		ServerConfig getServerConfig() const;

};